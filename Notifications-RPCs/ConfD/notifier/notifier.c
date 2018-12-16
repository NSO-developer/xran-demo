#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/poll.h>
#include <time.h>
#include <sys/time.h>
#include <pwd.h>

#include <confd_lib.h>
#include <confd_dp.h>

#include "xran-supervision.h"

#define OK(val) (assert((val) == CONFD_OK))
static int ctlsock, workersock;
static struct confd_daemon_ctx *dctx;
static struct confd_notification_ctx *live_ctx;



static int get_ctlsock(struct addrinfo *addr)
{
    int sock;

    if ((sock =
         socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) < 0)
        return -1;
    if (confd_connect(dctx, sock, CONTROL_SOCKET,
                      addr->ai_addr, addr->ai_addrlen) != CONFD_OK) {
        close(sock);
        return -1;
    }
    return sock;
}


static int get_workersock(struct addrinfo *addr)
{
    int sock;

    if ((sock =
         socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) < 0)
        return -1;
    if (confd_connect(dctx, sock, WORKER_SOCKET,
                      addr->ai_addr, addr->ai_addrlen) != CONFD_OK) {
        close(sock);
        return -1;
    }
    return sock;
}


static void getdatetime(struct confd_datetime *datetime)
{
    struct tm tm;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    gmtime_r(&tv.tv_sec, &tm);

    memset(datetime, 0, sizeof(*datetime));
    datetime->year = 1900 + tm.tm_year;
    datetime->month = tm.tm_mon + 1;
    datetime->day = tm.tm_mday;
    datetime->sec = tm.tm_sec;
    datetime->micro = tv.tv_usec;
    datetime->timezone = 0;
    datetime->timezone_minutes = 0;
    datetime->hour = tm.tm_hour;
    datetime->min = tm.tm_min;
}

static void getdatetime2(struct confd_datetime *datetime2, int seconds)
{
    struct tm tm2;
    struct timeval tv2;
    
    gettimeofday(&tv2, NULL);
    gmtime_r(&tv2.tv_sec, &tm2);
    
    tm2.tm_sec += seconds;
    mktime(&tm2);
    
    memset(datetime2, 0, sizeof(*datetime2));
    datetime2->year = 1900 + tm2.tm_year;
    datetime2->month = tm2.tm_mon + 1;
    datetime2->day = tm2.tm_mday;
    datetime2->sec = tm2.tm_sec;
    datetime2->micro = tv2.tv_usec;
    datetime2->timezone = 0;
    datetime2->timezone_minutes = 0;
    datetime2->hour = tm2.tm_hour;
    datetime2->min = tm2.tm_min;
}


static int timer()
{
    time_t rawtime;
    struct tm * timeinfo;
    double timer1 = 0;

    time( &rawtime );
    timeinfo = localtime ( &rawtime );
    timer1 = timeinfo->tm_sec + timeinfo->tm_min*60 + timeinfo->tm_hour*3600 + timeinfo->tm_yday*86400 + (timeinfo->tm_year-70)*31536000 + ((timeinfo->tm_year-69)/4)*86400 - ((timeinfo->tm_year-1)/100)*86400 + ((timeinfo->tm_year+299)/400)*86400;

    return timer1;
}


static void send_notification(confd_tag_value_t *vals, int nvals)
{
    struct confd_datetime now;

    getdatetime(&now);
    OK(confd_notification_send(live_ctx, &now, vals, nvals));
}


static void send_notify(int secs)
{
    struct confd_datetime nowtime;
    getdatetime2(&nowtime, secs);

    confd_tag_value_t vals[3];
    int i = 0;
    
    CONFD_SET_TAG_XMLBEGIN(&vals[i], xran_supervision_supervision_notification,     xran_supervision__ns);  i++;
    CONFD_SET_TAG_DATETIME(&vals[i],   xran_supervision_next_update_at,      nowtime);      i++;
    CONFD_SET_TAG_XMLEND(&vals[i],   xran_supervision_supervision_notification,     xran_supervision__ns);  i++;
    send_notification(vals, i);
}


static int read_variables2(const char *fname, char *variable)
{
    FILE *fptr;
    int interval1 = 0;
    int guard1 = 0;
    //Opens variables2 (output of supervision RPC) and reads interval and guard values
    if ((fptr = fopen(fname,"r")) == NULL){
        printf("Error! opening file");
        return(-1);
    }
    else {
        fscanf(fptr,"%d", &interval1);
        fscanf(fptr,"%d", &guard1);
        fclose(fptr);
        if (strcmp(variable,"interval") == 0) {
            return(interval1);
        }
        else if (strcmp(variable,"guard") == 0) {
            return(guard1);
        }
        else {
            return(60);
        }
    }
}


static void delete_variables2(const char *fname)
{
    double ct = 0;
    ct=timer();
    if (remove(fname) == 0) {
        printf("Variables2 file deleted successfully -- %.0lf\n", ct);
    }
    else {
        printf("Unable to delete Variables2 file\n");
    }
}


int file_exists(const char *fname)
{
    if( access( fname, F_OK ) != -1 ) {
        return(1);
    }
    else {
        return(0);
    }
}



int main(int argc, char **argv)
{
    char confd_port[16];
    struct addrinfo hints;
    struct addrinfo *addr = NULL;
    int debuglevel = CONFD_SILENT;
    int i;
    int c;
    char *p, *dname;
    struct confd_notification_stream_cbs ncb;
    struct pollfd set[3];
    int ret;
    int timeout = 0;
    
    double mastertime = 0;
    double intervaltime = 0;
    double currenttime = 0;
    int interval = 5;
    int guard = 3;

    char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    char variables2_file[100];
    strcpy(variables2_file,homedir);
    strcat(variables2_file,"/rpc/variables2");
    printf("Variables Filename is:   %s\n\n", variables2_file);
    
    mastertime = timer();
    intervaltime = mastertime;
    
    if (file_exists(variables2_file)) {
        interval=read_variables2(variables2_file,"interval");
        guard=read_variables2(variables2_file,"guard");
        delete_variables2(variables2_file);
    }
    printf("Inteval read from file is: %d\n", interval);
    printf("Guard read from file is: %d\n", guard);
    
    snprintf(confd_port, sizeof(confd_port), "%d", CONFD_PORT);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    while ((c = getopt(argc, argv, "dtpc:")) != -1) {
        switch (c) {
        case 'd':
            debuglevel = CONFD_DEBUG;
            break;
        case 't':
            debuglevel = CONFD_TRACE;
            break;
        case 'p':
            debuglevel = CONFD_PROTO_TRACE;
            break;
        case 'c':
            if ((p = strchr(optarg, '/')) != NULL)
                *p++ = '\0';
            else
                p = confd_port;
            if (getaddrinfo(optarg, p, &hints, &addr) != 0) {
                if (p != confd_port) {
                    *--p = '/';
                    p = "/port";
                } else {
                    p = "";
                }
                fprintf(stderr, "%s: Invalid address%s: %s\n",
                        argv[0], p, optarg);
                exit(1);
            }
            break;
        default:
            fprintf(stderr,
                    "Usage: %s [-dtpr] [-c address[/port]]\n",
                    argv[0]);
            exit(1);
        }
    }

    if (addr == NULL &&
        ((i = getaddrinfo("127.0.0.1", confd_port, &hints, &addr)) != 0))
        /* "Can't happen" */
        confd_fatal("%s: Failed to get address for ConfD: %s\n",
                    argv[0], gai_strerror(i));
    if ((dname = strrchr(argv[0], '/')) != NULL)
        dname++;
    else
        dname = argv[0];
    /* Init library */
    confd_init(dname, stderr, debuglevel);

    if ((dctx = confd_init_daemon(dname)) == NULL)
        confd_fatal("Failed to initialize ConfD\n");
    if ((ctlsock = get_ctlsock(addr)) < 0)
        confd_fatal("Failed to connect to ConfD\n");
    if ((workersock = get_workersock(addr)) < 0)
        confd_fatal("Failed to connect to ConfD\n");

    memset(&ncb, 0, sizeof(ncb));
    ncb.fd = workersock;
    ncb.get_log_times = NULL;
    ncb.replay = NULL;
    strcpy(ncb.streamname, "supervision");
    ncb.cb_opaque = NULL;
    if (confd_register_notification_stream(dctx, &ncb, &live_ctx) != CONFD_OK) {
        confd_fatal("Couldn't register stream %s\n", ncb.streamname);
    }
    if (confd_register_done(dctx) != CONFD_OK) {
        confd_fatal("Failed to complete registration\n");
    }

    printf("\nNetconf Notification Agent (notifier) Started!\n\n");
    fflush(stdout);
    
    while (1) {
        set[0].fd = ctlsock;
        set[0].events = POLLIN;
        set[0].revents = 0;

        set[1].fd = 0;
        set[1].events = POLLIN;
        set[1].revents = 0;

        timeout = 0;
        
        switch (poll(set, 2, timeout)) {
        case -1:
            break;

        default:
            /*Check for I/O */

            if (set[0].revents & POLLIN) { /* ctlsock */
                if ((ret = confd_fd_ready(dctx, ctlsock)) == CONFD_EOF) {
                    confd_fatal("Control socket closed\n");
                } else if (ret == CONFD_ERR && confd_errno != CONFD_ERR_EXTERNAL) {
                    confd_fatal("Error on control socket request: %s (%d): %s\n", confd_strerror(confd_errno), confd_errno, confd_lasterr());
                }
            }

            currenttime=timer();
            if (currenttime >= intervaltime+interval) {
                send_notify(interval);
                printf("Notification was sent -- %.0lf\n", currenttime);
                intervaltime=currenttime;
            }
            if (currenttime >= mastertime+interval+guard) {
                printf("RESET action due to Supervision Watchdog Timer Expiry!!! -- %.0lf\n", currenttime);
                mastertime=currenttime;
            }
            if (file_exists(variables2_file)) {
                interval=read_variables2(variables2_file,"interval");
                guard=read_variables2(variables2_file,"guard");
                printf("RPC Supervision Watchdog Timer Reset Received with inteval=%d and guard=%d -- %.0lf\n", interval, guard, currenttime);
                delete_variables2(variables2_file);
                mastertime=currenttime;
            }
        }
    }
}

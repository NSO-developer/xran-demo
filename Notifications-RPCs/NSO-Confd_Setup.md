# NSO - ConfD Setup

This document describes the procedure to configure NSO and ConfD on separate machines (servers / VMs) to demenostrate use of Notifications and RPCs in an xRAN-RU Netconf/Yang implementation. Particularly, we will limit the scope of this document to notifications and RPCs used by the xran-supervision model.

## xRAN-RU Supervision Description

The NSO machine serves as the Netconf Client and the ConfD machine serves as the Netconf Server (RU). Below is the flow diagram for this use case. 
**Reference:** [20180713-XRAN-FHMP0-v0100.pdf](http://www.xran.org/s/20180713-XRAN-FHMP0-v0100.pdf)

![xran-supervision-flow.png](https://github.com/NSO-developer/xran-demo/raw/master/Notifications-RPCs/xran-supervision-flow.png "xRAN-RU Supervision Flow Diagram")

The implemention of the above flow diagram is described below.

A **notifier** (c-based) process sends the **xran-supervision** notifications from the RU (ConfD) to the RU Controller (NSO), as per the configured interval. NSO will be configured to subscribe to this notification. When NSO receives the notification, a **Kicker** event is triggered and a configured **xran-supervision-action** is executed. The **xran-supervision-action** calls a python script that sends an **xran-supervision** RPC to the ConfD. When ConfD receives the RPC, it executes the associated **supervision-watchdog-reset.sh** script, which writes a **variables2** text (flat) file with the interval and guard timer values. The **notifier** process will detect that this text file exists, and will clear/reset the supervision watchdog timer, and cycle through this loop continously. If an RPC is not received within the interval + guard time frame, the **notifier** process triggers an RU RESET action.

## Prerequisites

For both the NSO and ConfD machines we will assume that the following items are met prior to the execution of the following steps:

* OS: Ubuntu **16.04**
* Username: **ubuntu**
* Install packages:
```Bash
sudo apt update
sudo apt upgrade
sudo apt-get install vim -y
sudo apt-get install ssh -y
sudo apt-get install python -y
sudo apt-get install ant -y
sudo apt-get install python-pip -y
sudo apt-get install default-jdk -y
sudo apt-get install xsltproc -y
sudo apt-get install curl -y
sudo pip install --upgrade pip
sudo pip install requests
sudo pip install pyangbind
```    
* NSO Installer: **nso-4.7.2.1.linux.x86_64.installer.bin**


## NSO Installation

NSO is available from https://developer.cisco.com/site/nso/.

To install NSO, ssh to the NSO machine and follow the steps below.

```Bash
cd $HOME
sh nso-4.7.2.1.linux.x86_64.installer.bin $HOME/nso-4.7.2.1 --local-install

source $HOME/nso-4.7.2.1/ncsrc >> ~/.profile
source $HOME/nso-4.7.2.1/netsim/confd/confdrc >> ~/.profile
```

Exit the ssh session and relogin. Create and prepare the NSO running directory **ncs-run** on your home directory.

```Bash
mkdir ncs-run
ncs-setup --dest $HOME/ncs-run
```

Replace the **~/ncs-run/ncs.conf** configuration file with the one provided here:
[ncs.conf](https://github.com/NSO-developer/xran-demo/blob/master/Notifications-RPCs/NSO/ncs-run/ncs.conf)

Run NSO.
```Bash
cd ~/ncs-run
ncs
```

### xRAN-RU NED Compiling and Loading
Transfer all the 2.0 yang files to the **~/yang** directory.

Replace the **~/yang/xran-supervision.yang** file with the one provide here:
[xran-supervision.yang](https://github.com/NSO-developer/xran-demo/blob/master/Notifications-RPCs/NSO/yang-override/xran-supervision.yang)

Generate and compile the **xran20** NED package based on the yang files located at the **~/yang** directory. Place the package in the **~/ncs-run/packages** directory.
```Bash
cd $HOME
ncs-make-package --netconf-ned ~/yang xran20 --dest ~/ncs-run/packages/xran20 --vendor xran --build --no-java
```

Restart NSO and reload the packages.
```Bash
cd ~/ncs-run
ncs --stop
ncs --with-package-reload
```

## Action Package Compiling and Loading
 
Transfer the **[xran-supervison-action](https://github.com/NSO-developer/xran-demo/tree/master/Notifications-RPCs/NSO/packages/xran-supervision-action)** package and place it in the **~/ncs-run/packages** directory.
```Bash
cd ~/ncs-run/packages/xran-supervision-action/src
make clean && make
```

Restart NSO and reload the packages.
```Bash
cd ~/ncs-run
ncs --stop
ncs --with-package-reload
```

## Kicker Configuration
Login to the NSO CLI and issue the following commands to configure the Kicker:
```
config
unhide debug
kickers notification-kicker kicker1 kick-node /xran-supervision-action:action action-name watchdogreset selector-expr "$SUBSCRIPTION_NAME = 'mysub'"
commit
end
```

Verify that the Kicker was configured correctly.
```
show running-config kickers
```
		kickers notification-kicker kicker1
		 selector-expr "$SUBSCRIPTION_NAME = 'mysub'"
		 kick-node     /action
		 action-name   watchdogreset
		!

## ConfD Installation
### In NSO machine
cd ~/nso-4.7.2.1/netsim
tar cvf confd.tar confd

# Transfer NSO-confd to the ConfD machine
e.g.,
scp confd.tar ubuntu@192.168.3.241:/home/ubuntu

# In ConfD machine
cd $HOME
tar xvf confd.tar
rm confd.tar
cd confd
sed -i 's|nso-4.7.2.1/netsim/||g' confdrc
sed -i 's|nso-4.7.2.1/netsim/||g' confdrc.tcsh
sed -i 's|nso-4.7.2.1/netsim/||g' etc/confd/confd.conf

cd $HOME
vi .profile
source /home/ubuntu/confd/confdrc


## Customize confd.conf
vi ~/confd/etc/confd/confd.conf
  <cdb>
    <operational>
      <enabled>true</enabled>
    </operational>
  </cdb>
  <notifications>
    <eventStreams>
      <stream>
        <name>supervision</name>
        <description>xran-supervision-notification</description>
        <replaySupport>false</replaySupport>
      </stream>
    </eventStreams>
  </notifications>
  <cli>
    <style>c</style>
  </cli> 


# Transfer 2-0, XML_2-0, notifier, and rpc directories to $HOME
cd ~/2-0
./compiler.sh
mv *.fxs ~/confd/etc/confd/

cd ~/notifier
cp ~/2-0/xran-supervision* .
make clean
make

### For Troubleshooting ONLY
#confdc -c -a xran-supervision-ann.yang xran-supervision.yang; confdc --emit-h xran-supervision.h xran-supervision.fxs; cc -c -o notifier.o notifier.c -Wall -g -I/home/ubuntu/confd/include; cc notifier.o /home/ubuntu/confd/lib/libconfd.a -lpthread -lm -Wall -g -I/home/ubuntu/confd/include -o notifier; ./notifier

#confdc -c -a xran-supervision-ann.yang xran-supervision.yang
#confdc --emit-h xran-supervision.h xran-supervision.fxs
#cc -c -o notifier.o notifier.c -Wall -g -I ~/confd/include
#cc notifier.o ~/confd/lib/libconfd.a -lpthread -lm -Wall -g -I ~/confd/include -o notifier


## Start ConfD
cd ~/confd
confd




# Add device on NSO CLI
config
devices authgroups group default
umap system remote-name admin remote-password admin
devices device ru0 authgroup default address 192.168.3.241 port 2022 device-type netconf
state admin-state unlocked 
commit
ssh fetch-host-keys 
connect
sync-from
end

# Verify
show running-config devices device ru0
show devices device ru0 live-status


# Load nso-config.xml
- From NSO machine
ncs_load -l -m ~/XML_2-0/nso-config.xml

- From NSO CLI
devices device ru0 sync-to


# Load confd-*.xml
- From confd machine
confd_load -lCO ~/XML_2-0/confd-all.xml

- From NSO CLI
devices device ru0 sync-from


# Subscription to supervision notification on NSO CLI
config
devices device ru0 netconf-notifications subscription mysub local-user admin stream supervision
commit
end

# Verify
show devices device ru0 netconf-notifications subscription



# From ConfD machine start notifier
cd ~/notifier
./notifier

# Check logs on ConfD machine
tail -f ~/rpc/supervision-watchdog-reset.result 

# Check logs on NSO machine
tail -f ~/ncs-run/logs/python-supervision-action.log 





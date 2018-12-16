# NSO - ConfD Setup

This document describes the procedure to configure NSO and ConfD on separate machines (servers / VMs) to demenostrate the use of Notifications and RPCs in an xRAN-RU Netconf/Yang implementation. Particularly, we will limit the scope of this document to notifications and RPCs used by the xran-supervision module.

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

echo "source $HOME/nso-4.7.2.1/ncsrc" >> ~/.profile
echo "source $HOME/nso-4.7.2.1/netsim/confd/confdrc" >> ~/.profile
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

## ConfD Installation
### In NSO Machine
Tar the **~/nso-4.7.2.1/netsim/confd** directory.
```Bash
cd ~/nso-4.7.2.1/netsim
tar cvf confd.tar confd
```

Transfer the **confd.tar** file to the ConfD machine.
```Bash
scp confd.tar ubuntu@<IP Address>:<HOME Directory>
```
e.g.,
```Bash
scp confd.tar ubuntu@192.168.3.241:/home/ubuntu
```

### In ConfD Machine
Extract the **confd.tar** file and the configuration files.
```Bash
cd $HOME
tar xvf confd.tar
rm confd.tar
cd confd
sed -i 's|nso-4.7.2.1/netsim/||g' confdrc
sed -i 's|nso-4.7.2.1/netsim/||g' confdrc.tcsh
sed -i 's|nso-4.7.2.1/netsim/||g' etc/confd/confd.conf

cd $HOME
echo "source /home/ubuntu/confd/confdrc" >> ~/.profile
```

Replace the **~/confd/etc/confd/confd.conf** file with the one provide here:
[confd.conf](https://github.com/NSO-developer/xran-demo/blob/master/Notifications-RPCs/ConfD/confd/etc/confd/confd.conf)

Transfer all the 2.0 yang files to the **~/yang** directory. Add/replace the following files provided here to this directory:
[xran-supervision.yang](https://github.com/NSO-developer/xran-demo/blob/master/Notifications-RPCs/ConfD/yang-override/xran-supervision.yang)
[xran-supervision-ann.yang](https://github.com/NSO-developer/xran-demo/blob/master/Notifications-RPCs/ConfD/yang-override/xran-supervision-ann.yang)
[compiler.sh](https://github.com/NSO-developer/xran-demo/blob/master/Notifications-RPCs/ConfD/yang-override/compiler.sh)

Copy the following directory structures to the ~/ (HOME) directory:
[notifier](https://github.com/NSO-developer/xran-demo/tree/master/Notifications-RPCs/ConfD/notifier)
[rpc](https://github.com/NSO-developer/xran-demo/tree/master/Notifications-RPCs/ConfD/rpc)

Compile all the yang files and place their compiled versions on the **~/confd/etc/confd** directory.
```Bash
cd ~/yang
./compiler.sh
mv *.fxs ~/confd/etc/confd/
```

Compile the **notifier** process.
```Bash
cd ~/notifier
cp ~/yang/xran-supervision* .
make clean
make
```

Start ConfD.
```Bash
cd ~/confd
confd
```

## Device Commissioning on NSO CLI
Login to the NSO CLI and issue the following commands to add the ConfD device **ru** to the NSO DB:

```
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
```
Verify that the **ru0** Device was added and configured correctly, and that NSO is able to fetch live-status from it.
```
show running-config devices device ru0
show devices device ru0 live-status
```

## Subscription to Supervision Notification on NSO CLI
Login to the NSO CLI and issue the following commands to subscribe the **ru0** device to the supervision notifications:
```
config
devices device ru0 netconf-notifications subscription mysub local-user admin stream supervision
commit
end
```
Verify that the **ru0** Device was subscribed successfuly to the supervision notifications.
```
show devices device ru0 netconf-notifications subscription
````

## Supervision Flow Validation

From the ConfD machine start the **notifier** process to validate the Supervision Flow.
```Bash
cd ~/notifier
./notifier
```
Sample output:
		Variables Filename is:   /home/ubuntu/rpc/variables2

		Inteval read from file is: 5
		Guard read from file is: 3

		Netconf Notification Agent (notifier) Started!

		Notification was sent -- 1544907843
		RPC Supervision Watchdog Timer Reset Received with inteval=10 and guard=6 -- 1544907843
		Variables2 file deleted successfully -- 1544907843
		Notification was sent -- 1544907853
		RPC Supervision Watchdog Timer Reset Received with inteval=10 and guard=6 -- 1544907853
		Variables2 file deleted successfully -- 1544907853




You may review the following logs on the ConfD machine:
```Bash
tail -f ~/rpc/supervision-watchdog-reset.result 
```
You may review the following logs on the NSO machine:
```Bash
tail -f ~/ncs-run/logs/python-supervision-action.log 
```




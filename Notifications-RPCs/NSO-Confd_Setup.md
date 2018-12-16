# NSO - ConfD Setup

This document describes the procedure to configure NSO and ConfD on separate machines (servers / VMs) to demenostrate use of Notifications and RPCs in an xRAN-RU Netconf/Yang implementation. Particularly, we will limit the scope of this document to notifications and RPCs used by the xran-supervision model.

## High-level Description

The NSO machine serves as the Netconf Client and the ConfD machine serves as the Netconf Server (RU).

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
The Action Package serves as a way to trigger
# Copy xran-supervison-action package to $HOME/ncs-run/packages
cd $HOME/ncs-run/packages/xran-supervision-action/src
make clean && make

cd $HOME/ncs-run
ncs --stop
ncs --with-package-reload


## Kicker Config
config
unhide debug
kickers notification-kicker kicker1 kick-node /xran-supervision-action:action action-name watchdogreset selector-expr "$SUBSCRIPTION_NAME = 'mysub'"
commit
end
show running-config kickers




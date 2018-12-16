# NSO - ConfD Setup

This document describes the procedure to configure NSO and ConfD on separate machines (servers / VMs).


## Prerequisites

For both the NSO and ConfD machines we will assume that the following items are met prior to the execution of the following steps:
* OS: Ubuntu 16.04
* Username: ubuntu
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

Hello World


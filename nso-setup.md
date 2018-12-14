# NSO Setup

NSO is available for free download for non-production use. This introduction is how to setup a small NETCONF/YANG environment with NSO to exercise xRAN's YANG based management plane interface.

## Download NSO

NSO is available from <https://developer.cisco.com/site/nso/>

**Unpack the installer**

This aassumes a macOS installation using NSO4.7...

    $ sh nso-4.7.darwin.x86_64.signed.bin

**Copy installer, e.g., to home directory and install nso and install nso**


    $ sh nso-4.7.darwin.x86_64.installer.bin /Users/admin/nso-4.7

**set environmental variables**


    $ source ~/nso-4.7/ncsrc
    $ source ~/nso-4.7/netsim/confd/confdrc


## Setup NSO and verify


    $ ncs-setup --dest /Users/admin/ncs-run
    $ cd ~/ncs-run
    $ ncs
    $ ncs --status | grep status
    $ ncs --version

## Download xRAN YANG models,

And store in directory, e.g., ~/yang/V1_1_0

If you are downloading the "common models" from the xRAN private repository, then **DO NOT** include the ietf-yang-library.yang in the above directory.

As per the warning in the xran-usermgmt.yang model, we need to address the of the constraint that one user account needs to be always enabled on the RU, as this will not be the case when we initially start the simulated RU.

``` yang
container xran-users {
  must "user/enabled='true'" {
    error-message "At least one account needs to be enabled.";
  }
  //TAKE NOTE - any configuration with zero enabled users is invalid.
  //This will typically be the case when using a simulated NETCONF Server
  //and so this constraint should be removed when operating in those scenarios

```
so these need to be commented out when working with simulated RUs

``` yang
container xran-users {
//  must "user/enabled='true'" {
//    error-message "At least one account needs to be enabled.";
//  }
  //TAKE NOTE - any configuration with zero enabled users is invalid.
  //This will typically be the case when using a simulated NETCONF Server
  //and so this constraint should be removed when operating in those scenarios

```

Also take note, there is an NSO bug (RT#35651) which currently requires a manual editing of the xran-uplane-conf.yang model. The leafref with require-instance to the imported mcap: module needs to be edited and changed to a uint16. The original YANG leaf is shown below:

``` yang
uses common-array-carrier-elements;

leaf band-number {
  if-feature mcap:LAA;
  type leafref {
    path "/mcap:module-capability/mcap:band-capabilities/mcap:band-number";
    require-instance false;
  }
  description
    "This parameter informing which frequency band particular antenna
     array is serving for.
     Intended use is to deal with multiband solutions.";
}

```
and this needs to be changed to a type uint16 as shown

``` yang
uses common-array-carrier-elements;

leaf band-number {
  if-feature mcap:LAA;
  type uint16; //avoiding error

  description
    "This parameter informing which frequency band particular antenna
     array is serving for.
     Intended use is to deal with multiband solutions.";
}

```


## Build Network Element Driver

Now the simulator constraints have been addressed, we can build the NED

    $ ~/ncs4.7/bin/ncs-make-package --netconf-ned ~/yang/V1_1_0 xran110 --dest ~/ncs-run/packages/xran110 --vendor xran --build --no-java


## Create simulated xRAN Radio Unit

Obvioulsy if you have a real RU, that is great, but for this demo we will be using "simulated RUs" that implement the xRAN M-Plane. These are created using the following commands:

    $ cd ~/ncs-run/packages
    $ ncs-netsim create-network ./xran110 1 rusim --dir ../netsim

and start

    $ cd ~/ncs-run
    $ ncs-netsim start


## Load simulated xRAN Radio Units into NSO

    $ cd ~/ncs-run/
    $ ncs-setup --netsim-dir ./netsim --dest ./ncs-run
    $ ncs-netsim ncs-xml-init > devices.xml
    $ ncs_load -l -m devices.xml

## Load xRAN package

Enter NSO CLI


    $ ncs_cli -C -u admin

Now use the NSO CLI to perform a package reload


    admin@ncs#packages reload
    reload-result {
     package xran110
     result true
    }

## Load the Radio Unit's configuration from xml

Add in system to the default group

    admin@ncs# config
    Entering configuration mode terminal
    admin@ncs(config)# devices authgroups group default
    admin@ncs(config-group-default)# umap system remote-name admin remote-password admin
    admin@ncs(config-umap-system)# commit
    Commit complete.
    admin@ncs(config-umap-system)# end
    admin@ncs# exit

Now load the RU's configuration data from the xml file

    $ ncs_load -l -m nso-config.xml

## Sync the configuration to the Radio Unit


    $ ncs_cli -C -u admin
    admin@ncs# devices device rusim0 sync-to
    result true
    admin@ncs# exit

## Load the operational state into RU

Get the IPC port for the NETSIM RU


    $ ncs-netsim list
    ncs-netsim list for ~/ncs-run/netsim
    name=rusim0 netconf=12022 snmp=11022 ipc=5010 cli=10022 dir=~/ncs-run/netsim/rusim/rusim

Load the RU's operational data from the xml files using the identified IPC port, in this case 5010 (where xxx is used to identify a particular model in the list of xml files)


    $ env CONFD_IPC_PORT=5010 confd_load -lC confd-xxx.xml

**Success!!** You know have a NETCONF client, supporting the xRAN YANG models, and a simulated Radio Unit.

You can check the live status of your RU from the NCS CLI


    $ ncs_cli -C -u admin
    admin@ncs# show devices device rusim0 live-status
    live-status hardware component netsim-ru-module
    class  XRAN-RADIO
    xran-name netsim-ru-module
    live-status hardware component netsim-ru-port0
    class  port
    xran-name netsim-ru-port0
    live-status hardware component netsim-ru-port1
    class  port
    xran-name netsim-ru-port1

(an example of the complete live status is included in [livestatus.txt](https://github.com/mark-grayson/xrandemo/blob/master/livestatus.txt) )

# xRAN Background

The [xRAN Forum](www.xran.org) have specified a so-called "lower-layer split" for supporting a multi-vendor LTE and 5G Radio Acees Networks (RANs). This splits the RAN into a Radio Unit (RU) and a lower-layer split Central Unit (lls-CU). Importantly, xRAN have defined the use of NETCONF/YANG for managing their Radio Unit.


# xRAN demo overview

These demos show how NSO and Confd can be used to exercise the set of [xRAN](http://http://www.xran.org/) YANG models used to support its split RAN architecture.

The models need to be downloaded from the xRAN website public
  * http://www.xran.org/resources/
  * http://www.xran.org/specs-download

Note - currently registration is required to download the zipped YANG model files

The xRAN M-Plane specification is also available by following the above links.

----

1. [nso-setup.md](https://github.com/NSO-developer/xran-demo/blob/master/nso-setup.md) describes how to install, and setup NSO, build the Network Element Driver for xRAN's RU, start a simulated RU and load example configuration and operational data.

2. [restconfexample.md](https://github.com/NSO-developer/xran-demo/blob/master/restconfexample.md) describes how to use NSO's RESTCONF API to recover the live status from the simulated RU, to use PyangBind to define and update the configuration of the RU and to load the live status of the simulated RU into a Python class hierarchy.

3. [Notifactions and RPCs](https://github.com/NSO-developer/xran-demo/tree/master/Notifications-RPCs) describes how to use NSO and ConfD to build structured xRAN procedures using RPCs and Notifications. In particular, it uses xran-supervision.yang as an example of how to develop on top of NSO the necessary capability to continually check that the RU has NETCONF connectivity using a subscription to the timer triggered supervision-notification and corresponding NSO functionality to repeatedly send the supervision-watchdog-reset RPC to avoid the RU going into supervision-failure mode.

# xRAN Background

The [xRAN Forum](www.xran.org) have specified a so-called "lower-layer split" for supporting a multi-vendor LTE and 5G Radio Acees Networks (RANs). This splits the RAN into a Radio Unit (RU) and a lower-layer split Central Unit (lls-CU). Importantly, xRAN have defined the use of NETCONF/YANG for managing their Radio Unit.

@startuml

package "Hybrid M-Plane Model" <<Rectangle>> {
"NMS" <--> "lls-CU"
hide "NMS" circle
hide "lls-CU" circle
hide "RU" circle
Class "NMS" {
lls-CU mgmt
..
NETCONF \nClient
}
Class "lls-CU" {
lls-CU mgmt
..
NETCONF \nClient
}
"lls-CU" "1..N" <--> "1" "RU"
"RU" : NETCONF \nServer
"NMS" "1..N" <--> "1" "RU"
note right of "NMS" : lls-CU OAM \nis not specified \nbyxRAN forum
}
@enduml

# xRAN demo overview

These demos show how NSO and Confd can be used to exercise the set of [xRAN](http://http://www.xran.org/) YANG models used to support its split RAN architecture.

The models need to be downloaded from the xRAN website public
  * http://www.xran.org/resources/
  * http://www.xran.org/specs-download

Note - currently registration is required to download the zipped YANG model files

The xRAN M-Plane specification is also available by following the above links.

----

1. nso-setup.md describes how to install, and setup NSO, build the Network Element Driver for xRAN's RU, start a simulated RU and load its configuration and operational data.

2. restconfexample.md describes how to use NSO's RESTCONF API to recover the live status from the simulated RU, to use PyangBind to define and update the configuration of the RU and to load the live status of the simulated RU into a Python class hierarchy.

3. [to be completed] describes how to use NSO and ConfD to build structured procedures using RPCs and Notifications

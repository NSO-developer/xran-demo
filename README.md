# xran demo overview

These demos show how NSO and Confd can be used to exercise the set of [xRAN](http://http://www.xran.org/) YANG models used to support its split RAN architecture.

The models need to be downloaded from the xRAN website public
  * http://www.xran.org/resources/
  * http://www.xran.org/specs-download

Note - currently registration is required to download the zipped YANG model files

The xRAN M-Plane specification is also available by following the above links.

----

1. nso-setup.md describes how to install, and setup NSO, build the Network Element Driver for xRAN's RU, start a simulated RU and load its configuration and operational data.

2. restconfexample.md describes how to use NSO's RESTCONF API to recover the live status from the simulated RU and to update the configuration of the RU

3. <more to add>

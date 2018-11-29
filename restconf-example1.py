#!/usr/bin/env python
import requests
import pyangbind.lib.pybindJSON as pybindJSON
from binding import xran_usermgmt

# NOTE - this uses the most recent xran-usermgmt.yang model

# hardcoded url for the demo
# see ncsx.x/examples.ncs/getting-started/developing-with-ncs/13-restconf for more information
url = 'http://admin:admin@localhost:8080/restconf/data/devices/device=rusim0/live-status/xran-usermgmt:xran-users'
headers={'Accept':'application/yang-data+json'}

# example getting live-status account information from the RU

resp = requests.get(url, headers=headers)

print "***Current user management configuration***"
print resp.content

# now define a new user account

users = xran_usermgmt()

user = users.xran_users.user.add("pythonuser")
user.password = "password123"
user.enabled = 1

print "***New account to be added***"
print(pybindJSON.dumps(users, mode="ietf"))

# now use restconf to update NSO - PATCH is used to add to the exiting configuration

url = 'http://admin:admin@localhost:8080/restconf/data/devices/device=rusim0/config/xran-usermgmt:xran-users'
headers={'content-type': 'application/yang-data+json', 'Accept':'application/yang-data+json'}
resp = requests.patch(url, data=pybindJSON.dumps(users, mode="ietf"), headers=headers)

print resp

url = 'http://admin:admin@localhost:8080/restconf/data/devices/device=rusim0/live-status/xran-usermgmt:xran-users'
headers={'Accept':'application/yang-data+json'}
resp = requests.get(url, headers=headers)

print "***NEW user management configuration***"
print resp.content

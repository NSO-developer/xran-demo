#!/usr/bin/env python
import requests
import binding
import pyangbind.lib.pybindJSON as pybindJSON
import json

headers={'Accept':'application/yang-data+json'}
url = 'http://admin:admin@localhost:8080/restconf/data/devices/device=rusim0/live-status/xran-usermgmt:xran-users'

# example getting live-status of RU

resp = requests.get(url, headers=headers)

ietf_json = resp.content

string_to_load = ietf_json.replace('\n','')
live_users = pybindJSON.loads_ietf(string_to_load, binding, "xran_usermgmt")

json_users = json.loads(pybindJSON.dumps(live_users))
json_users = json_users['xran-users']['user']

users=[]
i=0
for k in json_users.keys():
    users.append(str(k))

for user in users:
    print "user", i ,live_users.xran_users.user[user].get()
    i+=1

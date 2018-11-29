# Simple demo of how to develop on top of NSO

NSO supports a number of alternative development approaches, including Java and Python, but in this demo we will be using NSO's RESTCONF interface.

You can find more information on NSO's support of RESTCONF in the nso directory:
~/ncs4.7/examples.ncs/getting-started/developing-with-ncs/13-restconf

We will also be demonstrating how [PyangBind](http://pynms.io/pyangbind/) can be used to automatically generating Python class hierarchies.

## Creating  Python Bindings

Follow the steps described in the PyangBind [getting started guide](http://pynms.io/pyangbind/getting_started/):


    $ pip install pyangbind

Next set up a pointer to the PyangBind location

    $  export PYBINDPLUGIN=`/usr/bin/env python -c \
    'import pyangbind; import os; print "%s/plugin" % os.path.dirname(pyangbind.__file__)'`

Finally, create the necessary bindings. In this example, we will use the latest xran-usermgmt.yang model.

    $ pyang --plugindir $PYBINDPLUGIN -f pybind -o binding.py ~/yang/V1_1_0/xran-usermgmt.yang

Now move the resulting binding.py to your python directory

## Getting Live-Status using RESTCONF

We can recover the current live-status of the xran-usermgmt.model from NSO and display its JSON representation.

``` python
#!/usr/bin/env python  
import requests  
import pyangbind.lib.pybindJSON as pybindJSON  
from binding import xran_usermgmt  

# hardcoded url for the demo  
# see ncsx.x/examples.ncs/getting-started/developing-with-ncs/13-restconf for more information  
url = 'http://admin:admin@localhost:8080/restconf/data/devices/device=rusim0/live-status/xran-usermgmt:xran-users'  
headers={'Accept':'application/yang-data+json'}  

# example getting live-status account information from the RU  

resp = requests.get(url, headers=headers)  

print "Current user management configuration"  
print resp.content
```

and this should output the JSON encoded current configuration that was loaded from the nso-config.xml file.

    Current user management configuration
    {
        "xran-usermgmt:xran-users": {
        "user": [
          {
            "name": "fmpmuser",
            "password": "hashedpassword",
            "enabled": true
          },
          {
            "name": "nmsuser",
            "password": "hashedpassword",
            "enabled": true
          },
          {
            "name": "swmmuser",
            "password": "hashedpassword",
            "enabled": true
          },
          {
            "name": "xranuser",
            "password": "hashedpassword",
            "enabled": true
          }
        ]
      }
    }

## Use PyangBind to create a new xRAN user account on the RU

We are using the xran-usermgmt.yang model as an example, and it has the following tree structure.

    module: xran-usermgmt
      +--rw xran-users
         +--rw user* [name]
            +--rw name        username-type
            +--rw password?   password-type
            +--rw enabled?    boolean

We need to create an xran-users object and then define its name, password and whether it is enabled. (Note: NSO uses the ietf JSON encodings and not PyangBind's default open-configuration encoding.)


``` python
# now define a new user account  

users = xran_usermgmt()  

user = users.xran_users.user.add("pythonuser")  
user.password = "password123"  
user.enabled = 1  

print "New account to be added"  
print(pybindJSON.dumps(users, mode="ietf"))
```

which should output the following

    New account to be added
    {
        "xran-usermgmt:xran-users": {
            "user": [
                {
                    "password": "password123",
                    "enabled": true,
                    "name": "pythonuser"
                }
            ]
        }
    }

## Use RESTCONF to add the new user to the xran-usermgmt.yang list

We can now use the RESCONF PATCH method to add the newly defined user to the configuration and we can confirm the addition by getting the latest configuration back from NSO

``` python
# now use restconf to update NSO  

url = 'http://admin:admin@localhost:8080/restconf/data/devices/device=rusim0/config/xran-usermgmt:xran-users'  
headers={'content-type': 'application/yang-data+json', 'Accept':'application/yang-data+json'}  
resp = requests.patch(url, data=pybindJSON.dumps(users, mode="ietf"), headers=headers)  

# and check the configuration by re-getting the xran-users container from NSO

url = 'http://admin:admin@localhost:8080/restconf/data/devices/device=rusim0/live-status/xran-usermgmt:xran-users'  
headers={'Accept':'application/yang-data+json'}  
resp = requests.get(url, headers=headers)  

print "NEW user management configuration"  
print resp.content
```

which should confirm the new user has been added to the configuration

    NEW user management configuration
    {
      "xran-usermgmt:xran-users": {
        "user": [
          {
            "name": "fmpmuser",
            "password": "hashedpassword",
            "enabled": true
          },
          {
            "name": "nmsuser",
            "password": "hashedpassword",
            "enabled": true
          },
          {
            "name": "pythonuser",
            "password": "password123",
            "enabled": true
          },
          {
            "name": "swmmuser",
            "password": "hashedpassword",
            "enabled": true
          },
          {
            "name": "xranuser",
            "password": "hashedpassword",
            "enabled": true
          }
        ]
      }
    }

## Use RESTCONF and PyangBind to import NSO data into python

Finally, we can use the same PyangBind library to take JSON received from NSO and load it back into the python class hierarchy.

``` python
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
```
And from the display, we can see details of the five user accounts received from NSO over the RESTCONF interface.

    user 0 {'password': u'hashedpassword', 'enabled': True, 'name': u'nmsuser'}
    user 1 {'password': u'hashedpassword', 'enabled': True, 'name': u'swmmuser'}
    user 2 {'password': u'hashedpassword', 'enabled': True, 'name': u'xranuser'}
    user 3 {'password': u'hashedpassword', 'enabled': True, 'name': u'fmpmuser'}
    user 4 {'password': u'password123', 'enabled': True, 'name': u'pythonuser'}

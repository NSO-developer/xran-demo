# -*- mode: python; python-indent: 4 -*-
import ncs
from ncs.application import Service
from ncs.dp import Action
import subprocess
import requests
import json

import os

# ---------------
# ACTIONS EXAMPLE
# ---------------
class WatchdogresetAction(Action):
    @Action.action
    def cb_action(self, uinfo, name, kp, input, output):
        #self.log.info('action name: ', name)
        self.log.info('action name: ', name, ' -- kp: ', kp)
        self.log.info('action input.path: ', input.path)

	device = input.path.split("{")[1].split("}")[0]
	logfile = os.environ['HOME']+"/ncs-run/logs/python-supervision-action.log"

        # Updating the output data structure will result in a response
        # being returned to the caller.
	#f = open("/home/ubuntu/ncs-run/logs/python-supervision-action.log", "a")
        f = open(logfile, "a")
	f.write(input.path+"\n")
	f.write("Supervision Kicker Executed. Triggering Supervision RPC.\n")
	#f.write(logfile+"\n")
	f.close
	
	interval = '60'
	guard = '10'

	data = '{ "xran-supervision:supervision-watchdog-reset": { "supervision-notification-interval":"'+interval+'", "guard-timer-overhead":"'+guard+'"}}'
	json_data = json.loads(data)

	url = 'http://admin:admin@localhost:8080/api/running/devices/device/'+device+'/rpc/xran-supervision:rpc-supervision-watchdog-reset/_operations/supervision-watchdog-reset' 
	headers={'content-type': 'application/vnd.yang.operation+json', 'Accept':'application/vnd.yang.operation+json'}  

	resp = requests.post(url, data=json.dumps(json_data), headers=headers)


	#subprocess.call(['/home/ubuntu/kicker/supervision-rpc.sh',device])
        
	#output.result = input.number * 2
	#output.result = 1


# ------------------------
# SERVICE CALLBACK EXAMPLE
# ------------------------
class ServiceCallbacks(Service):

    # The create() callback is invoked inside NCS FASTMAP and
    # must always exist.
    @Service.create
    def cb_create(self, tctx, root, service, proplist):
        self.log.info('Service create(service=', service._path, ')')


    # The pre_modification() and post_modification() callbacks are optional,
    # and are invoked outside FASTMAP. pre_modification() is invoked before
    # create, update, or delete of the service, as indicated by the enum
    # ncs_service_operation op parameter. Conversely
    # post_modification() is invoked after create, update, or delete
    # of the service. These functions can be useful e.g. for
    # allocations that should be stored and existing also when the
    # service instance is removed.

    # @Service.pre_lock_create
    # def cb_pre_lock_create(self, tctx, root, service, proplist):
    #     self.log.info('Service plcreate(service=', service._path, ')')

    # @Service.pre_modification
    # def cb_pre_modification(self, tctx, op, kp, root, proplist):
    #     self.log.info('Service premod(service=', kp, ')')

    # @Service.post_modification
    # def cb_post_modification(self, tctx, op, kp, root, proplist):
    #     self.log.info('Service premod(service=', kp, ')')


# ---------------------------------------------
# COMPONENT THREAD THAT WILL BE STARTED BY NCS.
# ---------------------------------------------
class Main(ncs.application.Application):
    def setup(self):
        # The application class sets up logging for us. It is accessible
        # through 'self.log' and is a ncs.log.Log instance.
        self.log.info('Main RUNNING')

        # Service callbacks require a registration for a 'service point',
        # as specified in the corresponding data model.
        #
        #self.register_service('xran-supervision-action-servicepoint', ServiceCallbacks)

        # When using actions, this is how we register them:
        #
        self.register_action('xran-supervision-action-action', WatchdogresetAction)

        # If we registered any callback(s) above, the Application class
        # took care of creating a daemon (related to the service/action point).

        # When this setup method is finished, all registrations are
        # considered done and the application is 'started'.

    def teardown(self):
        # When the application is finished (which would happen if NCS went
        # down, packages were reloaded or some error occurred) this teardown
        # method will be called.

        self.log.info('Main FINISHED')

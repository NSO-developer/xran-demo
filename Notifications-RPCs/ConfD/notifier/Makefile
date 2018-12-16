######################################################################
# NETCONF Notification example
# (C) 2006-2009 Tail-f Systems
#
# See the README file for more information
######################################################################

default: usage notifier
	@echo "Build successful"

usage:
	@echo "#######################################################"
	@echo "# Make Options:                                       #"
	@echo "#######################################################"
	@echo ""
	@echo "make          Build notifier"
	@echo "make clean    Remove all built and intermediary files"
	@echo ""

######################################################################
# Where is ConfD installed? Make sure CONFD_DIR points it out
#CONFD_DIR ?= ../../..
CONFD_DIR = ~/confd

# Include standard ConfD build definitions and rules
include $(CONFD_DIR)/src/confd/build/include.mk

# In case CONFD_DIR is not set (correctly), this rule will trigger
$(CONFD_DIR)/src/confd/build/include.mk:
	@echo 'Where is ConfD installed? Set $$CONFD_DIR to point it out!'
	@echo ''

######################################################################
# Example specific definitions and rules
CONFD_FLAGS = --addloadpath $(CONFD_DIR)/etc/confd
START_FLAGS ?=


CFLAGS	+= $(EXPAT_INC)
LIBS	+= $(EXPAT_LIB)

notifier: notifier.o
	$(CC) notifier.o ~/confd/lib/libconfd.a -lpthread -lm -Wall -g -I ~/confd/include -o notifier

notifier.o: notifier.c xran-supervision.h
	$(CC) -c -o notifier.o notifier.c -Wall -g -I ~/confd/include

xran-supervision.h: xran-supervision.fxs
	$(CONFDC) --emit-h xran-supervision.h xran-supervision.fxs

xran-supervision.fxs: xran-supervision.yang xran-supervision-ann.yang
	$(CONFDC) -c -a xran-supervision-ann.yang xran-supervision.yang


######################################################################
clean:	iclean
	-rm -rf notifier *.h *.fxs 2> /dev/null || true


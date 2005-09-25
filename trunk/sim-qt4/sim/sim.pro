######################################################################
# created by Christian Ehrlicher
######################################################################

TEMPLATE = subdirs
CONFIG += ordered
include(sim.pri)

# the order is important
SUBDIRS += sim
# now all other plugin directories (currently none)


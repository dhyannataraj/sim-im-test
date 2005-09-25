######################################################################
# created by Christian Ehrlicher
######################################################################

isEmpty(TARGET):error(You must set SIM_TARGET before include()'ing $${_FILE_})

# to get clean source directories
# vc uses 'debug' and 'release' as temporary directory, so no need to change this here
!equals(TEMPLATE_PREFIX, "vc") {
    CONFIG      -= debug_and_release

    MOC_DIR      = tmp
    OBJECTS_DIR  = tmp
    RCD_DIR      = tmp
    UI_DIR       = tmp
}
CONFIG      -= debug_and_release_target
CONFIG      += debug                        # change dthis to release when you need it...
DEFINES     += VERSION=\"0.9.4\" PACKAGE=\"SIM\"
INCLUDEPATH *= $$TARGET
QT          += qt3support network
QMAKE_LIBS  *= user32.lib ws2_32.lib
 
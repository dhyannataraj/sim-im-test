###################
# jisp additional #
###################

# this needs to get generic

project(additional)

set (additional_JISP
        0.png
        1.png
        icondef.xml
)

ADD_CUSTOM_COMMAND(OUTPUT additional.jisp
 COMMAND ${ZIP_EXECUTABLE}
 ARGS
 -j
 ${additional_JISP}
 DEPENDS ${additional_JISP}
)

install( FILES additional.jisp DESTINATION  ${ICONS_OUTPUT_PATH} )

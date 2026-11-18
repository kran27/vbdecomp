add_library(granny2 STATIC IMPORTED GLOBAL)

set_target_properties(granny2 PROPERTIES
    IMPORTED_LOCATION
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/granny2_x64.lib"
)

target_include_directories(granny2 INTERFACE
    "include/"
)
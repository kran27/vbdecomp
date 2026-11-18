add_library(bink STATIC IMPORTED GLOBAL)

set_target_properties(bink PROPERTIES
    IMPORTED_LOCATION
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/binkw64.lib"
)
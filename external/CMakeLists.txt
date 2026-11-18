add_subdirectory("adecode")
add_subdirectory("bink")
add_subdirectory("granny2")
add_subdirectory("small")
add_subdirectory("zlib")

# DirectSound 8 and DirectShow come from the Windows SDK (x86 and x64).
# The DX8.1 audio include/lib directory must not enter the search paths.
add_library(windows-audio INTERFACE)
target_link_libraries(windows-audio INTERFACE dsound dxguid strmiids ole32 oleaut32)

# Preserve the optional legacy DirectMusic Performance Layer only for x86.
# Only seven DirectMusic headers are exposed, never the bundled dsound.h.
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_include_directories(windows-audio INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/legacy-directmusic/include"
    )
endif()

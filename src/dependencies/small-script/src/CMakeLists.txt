add_executable(scpack "scpack.c")
add_executable(sc "sc1.c" "sc2.c" "sc3.c" "sc4.c" "sc5.c" "sc6.c" "sc7.c" "scvars.c" "sclist.c" "scexpand.c")

if (UNIX)
target_link_libraries(sc PRIVATE m)
# The compiler source gates `<unistd.h>` (and `unlink`, `access`, etc.)
# on `LINUX` being defined, but nothing actually defines it on its own.
# Set it for both `sc` and `scpack` so a Linux-native build links.
target_compile_definitions(sc PRIVATE LINUX)
target_compile_definitions(scpack PRIVATE LINUX)
endif (UNIX)

# Windows-only DLL: in-memory compile entry for host applications.
# See scdll.h for the public API.
if (WIN32)
    add_library(sc_dll SHARED
        "scdll.c"
        "sc1.c" "sc2.c" "sc3.c" "sc4.c" "sc5.c" "sc6.c" "sc7.c"
        "scvars.c" "sclist.c" "scexpand.c")
    target_compile_definitions(sc_dll PRIVATE NO_MAIN SC_DLL_EXPORTS)
    set_target_properties(sc_dll PROPERTIES
        OUTPUT_NAME "sc"
        PREFIX ""                # produce sc.dll, not libsc.dll on MinGW
        IMPORT_PREFIX "")        # and sc.lib, not libsc.lib
endif (WIN32)
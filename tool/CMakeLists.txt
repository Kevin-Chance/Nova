
if (NOVA_BUILD_CLI)
    add_subdirectory(cli)
endif ()

if (NOVA_WITH_PROXYFMU)
    add_subdirectory(proxyfmu)
endif ()

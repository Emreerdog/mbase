include_guard(GLOBAL)

include(MBASECommonConfig)
include(MBASEStdSys)

set(MBASE_FRAMEWORK_SYS_STRING "MBASE Framework")
set(MBASE_FRAMEWORK_LIB_NAME "framework")

mbase_build_version(1 0 0 ${MBASE_VERSION_STABLE} MBASE_FRAMEWORK_VERSION)
mbase_build_include_install_path(${MBASE_FRAMEWORK_LIB_NAME} MBASE_FRAMEWORK_INCLUDE_INSTALL_PATH)

list(APPEND MBASE_FRAMEWORK_INCLUDE_DEPENDS ${MBASE_FRAMEWORK_INCLUDES})

list(APPEND MBASE_FRAMEWORK_INCLUDE_STABLE_FILES
    event_handler.h
    event_manager.h
    handler_base.h
    logical_processing.h
    object_watcher.h
    thread_pool.h
    timer_loop.h
    timers.h
)

foreach(MB_INCLUDE_FILE IN LISTS MBASE_FRAMEWORK_INCLUDE_STABLE_FILES)
    list(APPEND MBASE_FRAMEWORK_INCLUDE_INSTALL_FILES ${MBASE_FRAMEWORK_INCLUDE_INSTALL_PATH}/${MB_INCLUDE_FILE})
endforeach()

set(MBASE_FRAMEWORK_BIN_INSTALL_DIR ${CMAKE_INSTALL_BINDIR})
set(MBASE_FRAMEWORK_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(MBASE_FRAMEWORK_LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})

install(FILES ${MBASE_FRAMEWORK_INCLUDE_INSTALL_FILES}
        PERMISSIONS
            OWNER_READ
            OWNER_WRITE
            GROUP_READ
            WORLD_READ
        DESTINATION ${MBASE_FRAMEWORK_INCLUDE_INSTALL_DIR}/mbase/framework)
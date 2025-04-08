include_guard(GLOBAL)

include(CMakePackageConfigHelpers)

set(MBASE_GLOBAL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(MBASE_GLOBAL_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/mbase)
set(MBASE_GLOBAL_CONFIG_IN_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake_config_in)
set(MBASE_CURRENT_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(MBASE_COMMON_COMPILE_DEFINITIONS MBASE_BUILD MBASE_INTERNAL_API _CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64 -mtune=generic")
string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}" MBASE_SYSTEM_NAME)

if (WIN32)
    set(MBASE_COMMON_COMPILE_OPTIONS -W4) # Enable W4 Compiler warning
endif (WIN32)

if(UNIX)
    set(MBASE_COMMON_COMPILE_OPTIONS 
    -pedantic 
    -Wall 
    -Wextra 
    -Wcast-align 
    -Wctor-dtor-privacy 
    -Wdisabled-optimization 
    -Wformat=2 
    -Winit-self 
    -Wmissing-declarations
    -Woverloaded-virtual 
    -Wredundant-decls 
    -Wshadow 
    -Wsign-promo 
    -Wstrict-overflow=5 
    -Wswitch-default 
    -Wundef 
    -Wno-unused
)
endif(UNIX)

list(APPEND MBASE_STABILITY_CATEGORIES
    alpha    
    beta
    stable
)

list(GET MBASE_STABILITY_CATEGORIES 0 MBASE_VERSION_ALPHA)
list(GET MBASE_STABILITY_CATEGORIES 1 MBASE_VERSION_BETA)
list(GET MBASE_STABILITY_CATEGORIES 2 MBASE_VERSION_STABLE)

function(mbase_build_version in_major in_minor in_patch in_stability out_version_string)
    # do input checks later
    set(${out_version_string} ${in_major}.${in_minor}.${in_patch}${in_stability} PARENT_SCOPE)
endfunction()

function(mbase_build_include_install_path in_path_string out_install_path)
    # do input checks later
    if(in_path_string STREQUAL " ")
        # this is the only exception for mbase std library
        set(${out_install_path} ${MBASE_GLOBAL_INCLUDE} PARENT_SCOPE)
    else()
        set(${out_install_path} ${MBASE_GLOBAL_INCLUDE}/mbase/${in_path_string}/ PARENT_SCOPE)
    endif()
endfunction()

function(mbase_build_lib_path in_lib_name out_lib_path)
    list(APPEND MBASE_CREATED_LIBRARY_LIST ${in_lib_name})
    set(MBASE_TMP_CREATED_LIBRARY_LIST ${MBASE_CREATED_LIBRARY_LIST})
    set(MBASE_CREATED_LIBRARY_LIST ${MBASE_TMP_CREATED_LIBRARY_LIST} PARENT_SCOPE)
    set(${out_lib_path} ${MBASE_GLOBAL_SOURCE_DIRECTORY}/${in_lib_name} PARENT_SCOPE)
endfunction()

function(mbase_install_libraries)
    foreach(MBASE_TMP_LIB IN LISTS MBASE_CREATED_LIBRARY_LIST)
        string(TOUPPER ${MBASE_TMP_LIB} MBASE_LIB_NAME_UPPERCASE)
        
        set(MBASE_${MBASE_LIB_NAME_UPPERCASE}_BIN_INSTALL_DIR ${CMAKE_INSTALL_BINDIR})
        set(MBASE_${MBASE_LIB_NAME_UPPERCASE}_LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
        set(MBASE_${MBASE_LIB_NAME_UPPERCASE}_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})

        set(MBASE_GENERATED_BIN_INSTALL_DIR ${MBASE_${MBASE_LIB_NAME_UPPERCASE}_BIN_INSTALL_DIR})
        set(MBASE_GENERATED_LIB_INSTALL_DIR ${MBASE_${MBASE_LIB_NAME_UPPERCASE}_LIB_INSTALL_DIR})
        set(MBASE_GENERATED_INCLUDE_INSTALL_DIR ${MBASE_${MBASE_LIB_NAME_UPPERCASE}_INCLUDE_INSTALL_DIR})
        configure_package_config_file(
            ${MBASE_GLOBAL_CONFIG_IN_DIRECTORY}/mbase-${MBASE_TMP_LIB}-config.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/mbase-${MBASE_TMP_LIB}-config.cmake
            INSTALL_DESTINATION ${MBASE_GENERATED_LIB_INSTALL_DIR}/cmake/mbase-${MBASE_TMP_LIB}
            PATH_VARS
                MBASE_${MBASE_LIB_NAME_UPPERCASE}_INCLUDE_INSTALL_DIR
                MBASE_${MBASE_LIB_NAME_UPPERCASE}_LIB_INSTALL_DIR
                MBASE_${MBASE_LIB_NAME_UPPERCASE}_BIN_INSTALL_DIR
        )

        install(
            TARGETS mb_${MBASE_TMP_LIB}
            RUNTIME DESTINATION ${MBASE_GENERATED_BIN_INSTALL_DIR}
            LIBRARY DESTINATION ${MBASE_GENERATED_LIB_INSTALL_DIR}
            ARCHIVE DESTINATION ${MBASE_GENERATED_LIB_INSTALL_DIR}
        )

        install(
            FILES ${MBASE_${MBASE_LIB_NAME_UPPERCASE}_INCLUDE_INSTALL_FILES}
            PERMISSIONS
                OWNER_READ
                OWNER_WRITE
                GROUP_READ
                WORLD_READ
            DESTINATION ${MBASE_GENERATED_INCLUDE_INSTALL_DIR}/mbase/${MBASE_TMP_LIB}
        )

        install(
            FILES ${CMAKE_CURRENT_BINARY_DIR}/mbase-${MBASE_TMP_LIB}-config.cmake
            PERMISSIONS
                OWNER_READ
                OWNER_WRITE
                GROUP_READ
                WORLD_READ
            DESTINATION ${MBASE_GENERATED_LIB_INSTALL_DIR}/cmake/mbase.libs
        )
    endforeach()
endfunction()
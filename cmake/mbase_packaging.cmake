# COMMON CPACK OPTIONS

set(CPACK_PACKAGE_VENDOR "MBASE Yazilim")
set(CPACK_PACKAGE_CONTACT "erdog@mbasesoftware.com")
set(CPACK_PACKAGE_VERSION_MAJOR ${MBASE_LIBS_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${MBASE_LIBS_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${MBASE_LIBS_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/description.txt)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MBASE SDK")
set(CPACK_RESOURCE_FILE_README ${MBASE_LIBS_README})
set(CPACK_RESOURCE_FILE_LICENSE ${MBASE_LIBS_LICENSE})
set(CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS 
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_STRIP_FILES YES)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS YES)

if(MBASE_BUNDLED_INSTALL)
    set(CPACK_PACKAGE_NAME "mbase.libs-bundled")
else()
    set(CPACK_PACKAGE_NAME "mbase.libs")
endif(MBASE_BUNDLED_INSTALL)

if(GGML_CUDA)
    if (WIN32)
        find_package(CUDAToolkit)
        install(
            FILES ${CUDAToolkit_BIN_DIR}/cublas64_12.dll ${CUDAToolkit_BIN_DIR}/cublasLt64_12.dll ${CUDAToolkit_BIN_DIR}/cudart64_12.dll
            DESTINATION ${MBASE_LIBS_BIN_INSTALL_DIR}
        )
        message("BIN INSTALL DIR: ${CUDAToolkit_BIN_DIR}")
    elseif(UNIX)

    endif()
endif()

set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_NSIS_DISPLAY_NAME "MBASE SDK")
set(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")
set(CPACK_NSIS_UNINSTALL_NAME "MBASE SDK Uninstall")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "mbase.libs")
set(CPACK_NSIS_BRANDING_TEXT "${CPACK_PACKAGE_VENDOR}")

include(CPack)

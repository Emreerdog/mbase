if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "
FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.
")
endif()

cmake_minimum_required(VERSION 3.5...3.31)
   
project("mbase.libs" LANGUAGES CXX)

include(GNUInstallDirs)

option(BUILD_SHARED_LIBS "MBASE Shared library build default." ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(CMAKE_CXX_STANDARD 17)

set(MBASE_LIBS_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(MBASE_LIBS_LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
set(MBASE_LIBS_BIN_INSTALL_DIR ${CMAKE_INSTALL_BINDIR})
set(MBASE_LIBS_LIB_NAME "mbase.libs")
set(MBASE_LIBS_MAJOR 0)
set(MBASE_LIBS_MINOR 2)
set(MBASE_LIBS_PATCH 4)
set(MBASE_LIBS_VERSION "${MBASE_LIBS_MAJOR}.${MBASE_LIBS_MINOR}.${MBASE_LIBS_PATCH}")
set(MBASE_LIBS_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
set(MBASE_LIBS_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)

if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES "native")
    if(MBASE_CUDA_TARGET STREQUAL "datacenter")
        set(CMAKE_CUDA_ARCHITECTURES "100;90;89;86;80")
    elseif(MBASE_CUDA_TARGET STREQUAL "titan")
        set(CMAKE_CUDA_ARCHITECTURES "120;89;86;75")
    elseif(MBASE_CUDA_TARGET STREQUAL "jetson")
        set(CMAKE_CUDA_ARCHITECTURES "87;72")
    endif()
else()
    set(MBASE_CUDA_TARGET ${CMAKE_CUDA_ARCHITECTURES})
endif()

include(MBASEStdSys)
include(MBASEJsonSys)
include(MBASEFrameworkSys)
include(MBASECoreSys)
include(MBASEInferenceSys)

if(MBASE_BUNDLED_INSTALL STREQUAL "ON")
  install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/llama.cpp/LICENSE
    PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        GROUP_READ
        WORLD_READ
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/mbase/license
    RENAME LICENSE-llama.cpp.txt
  )
endif()

include(InstallRequiredSystemLibraries)
mbase_install_libraries()

configure_package_config_file(
  ${MBASE_GLOBAL_CONFIG_IN_DIRECTORY}/mbase.libs-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/mbase.libs-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mbase.libs-config.cmake
  PATH_VARS
    MBASE_LIBS_INCLUDE_INSTALL_DIR
    MBASE_LIBS_LIB_INSTALL_DIR
    MBASE_LIBS_BIN_INSTALL_DIR
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/mbase.libs-config.cmake
  PERMISSIONS
      OWNER_READ
      OWNER_WRITE
      GROUP_READ
      WORLD_READ
  DESTINATION ${MBASE_LIBS_LIB_INSTALL_DIR}/cmake/mbase.libs
)

install(
  FILES ${MBASE_LIBS_LICENSE}
  PERMISSIONS
      OWNER_READ
      OWNER_WRITE
      GROUP_READ
      WORLD_READ
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/mbase/license
)

add_subdirectory(examples/openai-server)
add_subdirectory(examples/simple-conversation)
add_subdirectory(examples/typo-fixer)
add_subdirectory(examples/benchmark)
add_subdirectory(examples/embedding)
add_subdirectory(examples/retrieval)

if(MBASE_PACKAGING)
  include(mbase_packaging)
endif()
if(NOT WIN32 OR NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8 OR NOT MSVC_TOOLSET_VERSION EQUAL 143)
  message(FATAL_ERROR "SENTRIUM requires Windows x64 / MSVC v143")
endif()
if(NOT CMAKE_CXX_COMPILER_VERSION STREQUAL "19.44.35229.0" OR NOT CMAKE_VERSION VERSION_EQUAL "4.4.4")
  message(FATAL_ERROR "SENTRIUM compiler/CMake lock mismatch")
endif()
if(NOT "$ENV{VCToolsVersion}" STREQUAL "14.44.35207" OR NOT "$ENV{VSCMD_ARG_HOST_ARCH}" STREQUAL "x64" OR NOT "$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x64")
  message(FATAL_ERROR "Run scripts/windows/sentrium-build.ps1 in the approved environment")
endif()
execute_process(COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/scripts/windows/sentrium-preflight.ps1" -VerifyOnly RESULT_VARIABLE check)
if(NOT check EQUAL 0)
  message(FATAL_ERROR "SENTRIUM exact prerequisite check failed")
endif()
file(READ "${CMAKE_SOURCE_DIR}/vcpkg.json" manifest)
string(JSON baseline GET "${manifest}" builtin-baseline)
get_filename_component(SENTRIUM_WORKSPACE_PARENT "${CMAKE_SOURCE_DIR}" DIRECTORY)
set(SENTRIUM_VCPKG_ROOT "${SENTRIUM_WORKSPACE_PARENT}/_toolchain/vcpkg")
if(NOT EXISTS "${SENTRIUM_VCPKG_ROOT}/.git")
  message(FATAL_ERROR "SENTRIUM approved vcpkg checkout not found")
endif()
execute_process(
  COMMAND git -C "${SENTRIUM_VCPKG_ROOT}" rev-parse HEAD
  OUTPUT_VARIABLE head
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0 OR NOT head STREQUAL baseline)
  message(FATAL_ERROR "SENTRIUM vcpkg checkout differs from the single manifest baseline")
endif()

cmake_minimum_required(VERSION 4.4.4)
project(sentrium VERSION 0.0.0 LANGUAGES CXX)
include(cmake/sentrium-toolchain-check.cmake)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(SENTRIUM_ASAN "Enable MSVC AddressSanitizer" OFF)
set(SENTRIUM_BUILD_ID "" CACHE STRING "Auditable source identity")
if(NOT SENTRIUM_BUILD_ID)
  execute_process(
    COMMAND git rev-parse --short=16 HEAD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE SENTRIUM_GIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE SENTRIUM_GIT_RESULT
  )
  if(SENTRIUM_GIT_RESULT EQUAL 0 AND SENTRIUM_GIT_SHA)
    execute_process(
      COMMAND git status --porcelain
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE SENTRIUM_GIT_STATUS
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(SENTRIUM_BUILD_ID "source-${SENTRIUM_GIT_SHA}")
    if(SENTRIUM_GIT_STATUS)
      string(APPEND SENTRIUM_BUILD_ID "-dirty")
    endif()
  else()
    set(SENTRIUM_BUILD_ID "source-unavailable")
  endif()
endif()
configure_file(cmake/sentrium-build.hpp.in generated/sentrium/build.hpp @ONLY)

if(DEFINED ENV{SENTRIUM_QT_ROOT} AND NOT "$ENV{SENTRIUM_QT_ROOT}" STREQUAL "")
  list(PREPEND CMAKE_PREFIX_PATH "$ENV{SENTRIUM_QT_ROOT}")
endif()
find_package(Qt6 6.12.0 EXACT REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.12)

set(SENTRIUM_INTELBRAS_SDK_ROOT "$ENV{SENTRIUM_INTELBRAS_SDK_ROOT}" CACHE PATH "External Intelbras/Dahua NetSDK root")
if(NOT SENTRIUM_INTELBRAS_SDK_ROOT)
  message(FATAL_ERROR "SENTRIUM_INTELBRAS_SDK_ROOT is required for Foundation-004")
endif()
file(TO_CMAKE_PATH "${SENTRIUM_INTELBRAS_SDK_ROOT}" SENTRIUM_INTELBRAS_SDK_ROOT)
foreach(relative
    "include/dhnetsdk.h"
    "bin/dhnetsdk.lib"
    "bin/dhconfigsdk.lib"
    "bin/dhnetsdk.dll"
    "bin/dhconfigsdk.dll")
  if(NOT EXISTS "${SENTRIUM_INTELBRAS_SDK_ROOT}/${relative}")
    message(FATAL_ERROR "Foundation-004 NetSDK component missing: ${relative}")
  endif()
endforeach()

set(SENTRIUM_PLAYSDK_ROOT "$ENV{SENTRIUM_PLAYSDK_ROOT}" CACHE PATH "External Intelbras/Dahua PlaySDK root")
if(NOT SENTRIUM_PLAYSDK_ROOT)
  message(FATAL_ERROR "SENTRIUM_PLAYSDK_ROOT is required for Foundation-004")
endif()
file(TO_CMAKE_PATH "${SENTRIUM_PLAYSDK_ROOT}" SENTRIUM_PLAYSDK_ROOT)
foreach(relative
    "DLL/dhplay.h"
    "DLL/dhplay.lib"
    "DLL/dhplay.dll")
  if(NOT EXISTS "${SENTRIUM_PLAYSDK_ROOT}/${relative}")
    message(FATAL_ERROR "Foundation-004 PlaySDK component missing: ${relative}")
  endif()
endforeach()

add_library(sentrium-core libs/sentrium-core/core.cpp)
target_include_directories(sentrium-core PUBLIC libs/sentrium-core/include "${CMAKE_CURRENT_BINARY_DIR}/generated")
add_library(sentrium-platform-win libs/sentrium-platform-win/platform.cpp)
target_include_directories(sentrium-platform-win PUBLIC libs/sentrium-platform-win/include)
target_compile_definitions(sentrium-platform-win PUBLIC UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(sentrium-platform-win PUBLIC sentrium-core PRIVATE shell32 ole32 advapi32)

add_library(sentrium-device
  libs/sentrium-device/device.cpp
  libs/sentrium-device/cloud.cpp
  libs/sentrium-device/cloud_wire.cpp
  libs/sentrium-device/cloud_udp.cpp
  libs/sentrium-device/cloud_udp_socket.cpp
  libs/sentrium-device/cloud_signaling.cpp
  libs/sentrium-device/cloud_ptcp.cpp
  libs/sentrium-device/cloud_ptcp_session.cpp
  libs/sentrium-device/cloud_ptcp_receive_window.cpp
  libs/sentrium-device/cloud_ptcp_channel.cpp
  libs/sentrium-device/cloud_ptcp_stream.cpp
  libs/sentrium-device/cloud_loopback_bridge.cpp
  libs/sentrium-device/cloud_stream_tunnel.cpp
  libs/sentrium-device/cloud_auth.cpp)
target_link_libraries(sentrium-device PRIVATE ws2_32 bcrypt)
target_include_directories(sentrium-device PUBLIC libs/sentrium-device/include)

add_library(sentrium-intelbras libs/sentrium-intelbras/intelbras.cpp)
target_include_directories(sentrium-intelbras PUBLIC libs/sentrium-intelbras/include)
target_include_directories(sentrium-intelbras SYSTEM PRIVATE "${SENTRIUM_INTELBRAS_SDK_ROOT}/include")
target_compile_definitions(sentrium-intelbras PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(sentrium-intelbras PUBLIC sentrium-device PRIVATE kernel32)

add_library(sentrium-media libs/sentrium-media/media.cpp)
target_include_directories(sentrium-media PUBLIC libs/sentrium-media/include)

add_executable(sentrium-server apps/sentrium-server/main.cpp)
target_link_libraries(sentrium-server PRIVATE sentrium-platform-win)
add_subdirectory(apps/sentrium-client)
enable_testing()
add_executable(sentrium-unit-tests tests/unit/main.cpp)
target_link_libraries(sentrium-unit-tests PRIVATE sentrium-core)
add_executable(sentrium-integration-tests tests/integration/main.cpp)
target_link_libraries(sentrium-integration-tests PRIVATE sentrium-platform-win)

add_executable(sentrium-device-tests tests/device/main.cpp)
target_link_libraries(sentrium-device-tests PRIVATE sentrium-device ws2_32)

add_executable(sentrium-cloud-network-tests tests/device/cloud_network_lab.cpp)
target_link_libraries(sentrium-cloud-network-tests PRIVATE sentrium-device ws2_32)

add_executable(sentrium-media-tests tests/media/main.cpp)
target_link_libraries(sentrium-media-tests PRIVATE sentrium-media)

add_executable(sentrium-media-lab labs/sentrium-media-lab/main.cpp)
target_include_directories(sentrium-media-lab SYSTEM PRIVATE
    "${SENTRIUM_INTELBRAS_SDK_ROOT}/include"
    "${SENTRIUM_PLAYSDK_ROOT}/DLL")
target_compile_definitions(sentrium-media-lab PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(sentrium-media-lab PRIVATE sentrium-core sentrium-device sentrium-intelbras sentrium-media kernel32)

foreach(target sentrium-core sentrium-platform-win sentrium-device sentrium-intelbras sentrium-media sentrium-server sentrium-client sentrium-media-lab sentrium-unit-tests sentrium-integration-tests sentrium-device-tests sentrium-cloud-network-tests sentrium-media-tests)
  target_compile_options(${target} PRIVATE /W4 /WX /permissive- /utf-8)
  if(SENTRIUM_ASAN)
    target_compile_options(${target} PRIVATE /fsanitize=address /Zi)
    target_link_options(${target} PRIVATE /INCREMENTAL:NO)
  endif()
endforeach()
foreach(id RANGE 1 8)
  add_test(NAME U-${id} COMMAND sentrium-unit-tests ${id})
endforeach()
foreach(id RANGE 1 11)
  add_test(NAME I-${id} COMMAND sentrium-integration-tests ${id} $<TARGET_FILE:sentrium-server>)
endforeach()
foreach(id RANGE 1 40)
  add_test(NAME D-${id} COMMAND sentrium-device-tests ${id})
endforeach()
foreach(id RANGE 41 46)
  add_test(NAME D-${id} COMMAND sentrium-cloud-network-tests ${id})
endforeach()
foreach(id RANGE 1 6)
  add_test(NAME M-${id} COMMAND sentrium-media-tests ${id})
endforeach()
add_test(
  NAME F002-CLIENT-SMOKE
  COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass
          -File "${CMAKE_SOURCE_DIR}/scripts/windows/sentrium-client-smoke.ps1"
          -Executable "$<TARGET_FILE:sentrium-client>"
          -Cycles 10
)
set_tests_properties(I-1 I-2 I-3 I-4 I-5 I-6 I-7 I-8 I-9 I-10 I-11 PROPERTIES TIMEOUT 30)
set_tests_properties(F002-CLIENT-SMOKE PROPERTIES TIMEOUT 120)

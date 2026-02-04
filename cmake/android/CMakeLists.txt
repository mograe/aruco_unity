cmake_minimum_required(VERSION 3.22)
project(aruco_unity_android LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Repo root (aruco_unity.cpp/.h + debug dump + etc.)
set(SRC_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")

# ---------- outputs ----------
# Android builds are typically single-config (Ninja/Make): CMAKE_BUILD_TYPE decides Debug/Release
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# ---------- OpenCV ----------
# Must be provided via -DOpenCV_DIR=... (folder containing OpenCVConfig.cmake)
find_package(OpenCV REQUIRED COMPONENTS
  core
  imgproc
  calib3d
  aruco
  objdetect
  features2d
  flann
)

# ---------- Debug dump sources only for Debug-ish builds ----------
set(ARUCO_DUMP_SOURCES "")
set(ARUCO_DUMP_DEFS "")

set(IS_DEBUG_BUILD OFF)
if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set(IS_DEBUG_BUILD ON)
endif()

if (ANDROID AND IS_DEBUG_BUILD)
  list(APPEND ARUCO_DUMP_SOURCES
    "${SRC_ROOT}/aruco_unity_debug_dump.cpp"
    "${SRC_ROOT}/aruco_unity_debug_dump.h"
  )
  list(APPEND ARUCO_DUMP_DEFS ARUCO_UNITY_ENABLE_NATIVE_DUMP=1)
endif()

# ---------------------------
# 1) Unity plugin .so
# ---------------------------
add_library(aruco_unity SHARED
  "${SRC_ROOT}/aruco_unity.cpp"
  "${SRC_ROOT}/aruco_unity.h"
  ${ARUCO_DUMP_SOURCES}
)

set_target_properties(aruco_unity PROPERTIES
  OUTPUT_NAME "aruco_unity"  # => libaruco_unity.so
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_compile_definitions(aruco_unity PRIVATE
  ARUCO_UNITY_EXPORTS
  ${ARUCO_DUMP_DEFS}
)

target_include_directories(aruco_unity PRIVATE
  "${SRC_ROOT}"
  ${OpenCV_INCLUDE_DIRS}
)

# Android system libs (часто нужно log; android иногда полезен)
target_link_libraries(aruco_unity PRIVATE
  ${OpenCV_LIBS}
  log
  android
)

# ---------------------------
# 2) (Optional) native test executable
# На Android это обычно не нужно (проще тестить через Unity),
# но оставлю как опцию: можно включить -DARUCO_BUILD_NATIVE_TEST=ON
# ---------------------------
option(ARUCO_BUILD_NATIVE_TEST "Build native test executable (Android)" OFF)

if (ARUCO_BUILD_NATIVE_TEST)
  add_executable(aruco_unity_test
    "${SRC_ROOT}/test_main.cpp"
  )
  set_target_properties(aruco_unity_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  target_include_directories(aruco_unity_test PRIVATE "${SRC_ROOT}")
  target_link_libraries(aruco_unity_test PRIVATE aruco_unity log android)
endif()

# ---------------------------
# 3) Packaging for Unity:
# build-android/bin/unity/Plugins/Android/<abi>/*.so
# ---------------------------
if (ANDROID)
  if (NOT DEFINED ANDROID_ABI)
    message(FATAL_ERROR "ANDROID_ABI is not defined. Pass -DANDROID_ABI=arm64-v8a")
  endif()

  set(UNITY_PLUGIN_DIR "${CMAKE_BINARY_DIR}/bin/unity/Plugins/Android/${ANDROID_ABI}")
  add_custom_command(TARGET aruco_unity POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${UNITY_PLUGIN_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:aruco_unity>"
      "${UNITY_PLUGIN_DIR}/$<TARGET_FILE_NAME:aruco_unity>"
    VERBATIM
  )

  # ---- Copy OpenCV *.so рядом с плагином ----
  # У тебя OpenCV собран как shared libs и лежит примерно в:
  #   ~/build-opencv-arm64/lib/arm64-v8a/libopencv_*.so
  # Поэтому лучше всего явно передавать путь:
  #   -DOpenCV_ANDROID_SO_DIR=/home/mograne/build-opencv-arm64/lib/arm64-v8a
  #
  # Если не передали — попробуем угадать через OpenCV_DIR.
  if (DEFINED OpenCV_ANDROID_SO_DIR)
    set(OPENCV_SO_DIR "${OpenCV_ANDROID_SO_DIR}")
  else()
    # best-effort guess:
    # OpenCV_DIR = .../unix-install/abi-arm64-v8a
    # build root = two levels up => .../build-opencv-arm64
    get_filename_component(_p1 "${OpenCV_DIR}" DIRECTORY)  # .../unix-install
    get_filename_component(_p2 "${_p1}" DIRECTORY)         # .../build-opencv-arm64
    set(OPENCV_SO_DIR "${_p2}/lib/${ANDROID_ABI}")
  endif()

  message(STATUS "OpenCV .so dir: ${OPENCV_SO_DIR}")

  add_custom_command(TARGET aruco_unity POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${OPENCV_SO_DIR}"
      "${UNITY_PLUGIN_DIR}"
    VERBATIM
  )

  # ---- Copy libc++_shared.so if using c++_shared ----
  # Unity/Quest почти всегда требует положить libc++_shared.so рядом.
  if (DEFINED ANDROID_NDK)
    if (ANDROID_ABI STREQUAL "arm64-v8a")
      set(_TRIPLE "aarch64-linux-android")
    elseif (ANDROID_ABI STREQUAL "armeabi-v7a")
      set(_TRIPLE "arm-linux-androideabi")
    elseif (ANDROID_ABI STREQUAL "x86_64")
      set(_TRIPLE "x86_64-linux-android")
    else()
      set(_TRIPLE "")
    endif()

    if (NOT _TRIPLE STREQUAL "")
      # ANDROID_PLATFORM like android-29
      if (DEFINED ANDROID_PLATFORM)
        string(REPLACE "android-" "" _API "${ANDROID_PLATFORM}")
      elseif (DEFINED ANDROID_NATIVE_API_LEVEL)
        set(_API "${ANDROID_NATIVE_API_LEVEL}")
      else()
        set(_API "29")
      endif()

      # Detect host tag from compiler path
      get_filename_component(_CC_PATH "${CMAKE_C_COMPILER}" ABSOLUTE)
      string(REPLACE "\\" "/" _CC_PATH "${_CC_PATH}")
      string(REGEX REPLACE ".*/toolchains/llvm/prebuilt/([^/]+)/bin/clang.*" "\\1" _HOST_TAG "${_CC_PATH}")

      set(LIBCXX_SHARED
        "${ANDROID_NDK}/toolchains/llvm/prebuilt/${_HOST_TAG}/sysroot/usr/lib/${_TRIPLE}/${_API}/libc++_shared.so"
      )

      message(STATUS "libc++_shared guess: ${LIBCXX_SHARED}")

      if (EXISTS "${LIBCXX_SHARED}")
        add_custom_command(TARGET aruco_unity POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LIBCXX_SHARED}"
            "${UNITY_PLUGIN_DIR}/libc++_shared.so"
          VERBATIM
        )
      else()
        message(WARNING "libc++_shared.so not found at: ${LIBCXX_SHARED}")
      endif()
    endif()
  endif()
endif()

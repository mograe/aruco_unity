cmake_minimum_required(VERSION 3.22)
project(aruco_unity_win LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Repo root (aruco_unity.cpp/.h + test_main.cpp + test1.jpg)
set(SRC_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")

# Все exe/dll будут складываться в build-win/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Для DLL на Windows тоже используем runtime output dir
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# OpenCV (через vcpkg toolchain)
find_package(OpenCV REQUIRED COMPONENTS
  core imgproc imgcodecs calib3d aruco objdetect features2d flann
)

# ---------------------------
# 1) Unity plugin DLL
# ---------------------------
add_library(aruco_unity SHARED
  "${SRC_ROOT}/aruco_unity.cpp"
  "${SRC_ROOT}/aruco_unity.h"
  "${SRC_ROOT}/aruco_unity_debug_dump.cpp"
  "${SRC_ROOT}/aruco_unity_debug_dump.h"
)

set_target_properties(aruco_unity PROPERTIES
  OUTPUT_NAME "aruco_unity"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Enable dllexport via aruco_unity.h macros
target_compile_definitions(aruco_unity PRIVATE ARUCO_UNITY_EXPORTS)

target_include_directories(aruco_unity PRIVATE
  "${SRC_ROOT}"
  ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(aruco_unity PRIVATE
  ${OpenCV_LIBS}
)

# ---------------------------
# 2) Console test executable (NO OpenCV usage in source)
# ---------------------------
add_executable(aruco_unity_test
  "${SRC_ROOT}/test_main.cpp"
)

set_target_properties(aruco_unity_test PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(aruco_unity_test PRIVATE
  "${SRC_ROOT}"
)

# Link only with our DLL (import library)
target_link_libraries(aruco_unity_test PRIVATE
  aruco_unity
)

# ---------------------------
# 3) Pick correct vcpkg runtime folder (Debug vs Release)
# Ninja is single-config -> use CMAKE_BUILD_TYPE.
# ---------------------------
if (WIN32)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(VCPKG_RUNTIME_BIN "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin")
  else()
    set(VCPKG_RUNTIME_BIN "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
  endif()
endif()

# ---------------------------
# 4) Post-build: populate bin/ and bin/unity/
# ---------------------------
# ---- Runtime output to bin/ ----
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# ... targets aruco_unity / aruco_unity_test ...

if (WIN32)
  # Ninja is single-config -> decide by CMAKE_BUILD_TYPE
  set(IS_DEBUG_BUILD FALSE)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(IS_DEBUG_BUILD TRUE)
  endif()

  if (IS_DEBUG_BUILD)
    set(VCPKG_RUNTIME_BIN "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin")
    set(UNITY_PACK_DIR "$<TARGET_FILE_DIR:aruco_unity_test>/unity_debug")
  else()
    set(VCPKG_RUNTIME_BIN "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
    set(UNITY_PACK_DIR "$<TARGET_FILE_DIR:aruco_unity_test>/unity")
  endif()

  add_custom_command(TARGET aruco_unity_test POST_BUILD
    # Ensure folder
    COMMAND ${CMAKE_COMMAND} -E make_directory "${UNITY_PACK_DIR}"

    # Copy plugin DLL into package folder
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:aruco_unity>"
      "${UNITY_PACK_DIR}/aruco_unity.dll"

    # Copy PDB (only exists on MSVC debug-ish builds)
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_PDB_FILE:aruco_unity>"
      "${UNITY_PACK_DIR}/aruco_unity.pdb"

    # Copy vcpkg runtime deps into package folder (only correct bin for this build)
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${VCPKG_RUNTIME_BIN}"
      "${UNITY_PACK_DIR}"

    # Also copy plugin + deps next to exe so aruco_unity_test runs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:aruco_unity>"
      "$<TARGET_FILE_DIR:aruco_unity_test>/aruco_unity.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${VCPKG_RUNTIME_BIN}"
      "$<TARGET_FILE_DIR:aruco_unity_test>"

    # Copy test image next to exe
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${SRC_ROOT}/test1.jpg"
      "$<TARGET_FILE_DIR:aruco_unity_test>/test1.jpg"

    VERBATIM
  )
endif()


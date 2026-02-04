#pragma once
#include <stdint.h>

#if defined(_DEBUG) || !defined(NDEBUG)
    #define ARUCO_UNITY_ENABLE_DUMP 1
#else
    #define ARUCO_UNITY_ENABLE_DUMP 0
#endif

#include "aruco_unity.h"

#ifdef __cplusplus
extern "C" {
#endif


ARUCO_UNITY_API int ARUCO_UNITY_CALL DumpGray8Png(
    const uint8_t* gray, int width, int height,
    const char* outPathUtf8
);

ARUCO_UNITY_API int ARUCO_UNITY_CALL DumpRGBA8Png(
    const uint8_t* rgba, int width, int height,
    const char* outPathUtf8
);

ARUCO_UNITY_API int ARUCO_UNITY_CALL DumpEncodedPng(
    const uint8_t* encoded, int encodedLen,
    const char* outPathUtf8
);

#ifdef __cplusplus
} // extern "C"
#endif
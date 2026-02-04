#pragma once
#include <stdint.h>


#if defined(_WIN32) || defined(_WIN64)
  #if defined(ARUCO_UNITY_EXPORTS)
    #define ARUCO_UNITY_API __declspec(dllexport)
  #else
    #define ARUCO_UNITY_API __declspec(dllimport)
  #endif
  #define ARUCO_UNITY_CALL __cdecl
#else
  #define ARUCO_UNITY_API
  #define ARUCO_UNITY_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

// dictId:
// 0 = DICT_4X4_50
// 1 = DICT_5X5_100
// 2 = DICT_6X6_250
// 3 = DICT_ARUCO_ORIGINAL
//
// Returns the number of detected markers (<= maxMarkers).
// gray: 8-bit grayscale image buffer (width * height bytes)
// K9: 3x3 camera intrinsics matrix (9 floats, row-major)
// dist: distortion coefficients (can be null, set distCount = 0)
// markerLength: marker side length in meters
// outIds: int[maxMarkers] output marker IDs
// outRvecs/outTvecs: float[3 * maxMarkers] output rvec/tvec (3 values per marker)
ARUCO_UNITY_API int ARUCO_UNITY_CALL DetectArucoGray8(
    const uint8_t* gray, int width, int height,
    const float* K9,
    const float* dist, int distCount,
    float markerLength,
    int dictId,
    int* outIds,
    float* outRvecs,
    float* outTvecs,
    int maxMarkers
);

// Same as DetectArucoGray8, but takes an encoded image (JPEG/PNG) and decodes it
// inside OpenCV using cv::imdecode() to a grayscale image.
//
// encoded: pointer to JPEG/PNG bytes
// encodedLen: number of bytes
ARUCO_UNITY_API int ARUCO_UNITY_CALL DetectArucoEncodedImage(
    const uint8_t* encoded, int encodedLen,
    const float* K9,
    const float* dist, int distCount,
    float markerLength,
    int dictId,
    int* outIds,
    float* outRvecs,
    float* outTvecs,
    int maxMarkers
);

// Detect ArUco markers from raw RGBA8 frame (width*height*4 bytes).
// rgba: RGBA pixel buffer (row-major, tightly packed)
// Returns number of detected markers (<= maxMarkers).
ARUCO_UNITY_API int ARUCO_UNITY_CALL DetectArucoRGBA8(
    const uint8_t* rgba, int width, int height,
    const float* K9,
    const float* dist, int distCount,
    float markerLength,
    int dictId,
    int* outIds,
    float* outRvecs,
    float* outTvecs,
    int maxMarkers
);

#ifdef __cplusplus
} // extern "C"
#endif

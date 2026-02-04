#include "aruco_unity_debug_dump.h"

#if ARUCO_UNITY_ENABLE_DUMP

    #include <opencv2/core.hpp>
    #include <opencv2/imgcodecs.hpp>
    #include <opencv2/imgproc.hpp>

    #include <string>

    static inline bool PathOk(const char* p)
    {
        return p && p[0];
    }

    extern "C" int ARUCO_UNITY_CALL DumpGray8Png(
        const uint8_t* gray, int width, int height,
        const char* outPathUtf8)
    {
        if (!gray || width <= 0 || height <= 0) return 0;
        if (!PathOk(outPathUtf8)) return 0;

        cv::Mat img(height, width, CV_8UC1, (void*)gray);
        bool ok = cv::imwrite(outPathUtf8, img);
        return ok ? 1 : 0;
    } 

    extern "C" int ARUCO_UNITY_CALL DumpRGBA8Png(
        const uint8_t* rgba, int width, int height,
        const char* outPathUtf8)
    {
        if (!rgba || width <= 0 || height <= 0) return 0;
        if (!PathOk(outPathUtf8)) return 0;

        cv::Mat rgbaMat(height, width, CV_8UC4, (void*)rgba);
        cv::Mat gray;
        cv::cvtColor(rgbaMat, gray, cv::COLOR_RGBA2GRAY);

        bool ok = cv::imwrite(outPathUtf8, gray);
        return ok ? 1 : 0;
    }

    extern "C" int ARUCO_UNITY_CALL DumpEncodedPng(
        const uint8_t* encoded, int encodedLen,
        const char* outPathUtf8)
    {
        if (!encoded || encodedLen <= 0) return 0;
        if (!PathOk(outPathUtf8)) return 0;

        cv::Mat buf(1, encodedLen, CV_8U, (void*)encoded);
        cv::Mat img = cv::imdecode(buf, cv::IMREAD_COLOR);
        bool ok = cv::imwrite(outPathUtf8, img);
        return ok ? 1 : 0;
    }
#else

    extern "C" int ARUCO_UNITY_CALL DumpGray8Png(
        const uint8_t* gray, int width, int height,
        const char* outPathUtf8) { return 0; }
    extern "C" int ARUCO_UNITY_CALL DumpRGBA8Png(
        const uint8_t* rgba, int width, int height,
        const char* outPathUtf8) { return 0; }
    extern "C" int ARUCO_UNITY_CALL DumpEncodedPng(
        const uint8_t* encoded, int encodedLen,
        const char* outPathUtf8) { return 0; }

#endif
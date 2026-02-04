#include "aruco_unity.h"

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/imgcodecs.hpp>

#include <vector>
#include <cstring>
#include <type_traits>

//Helpers

static cv::Mat MakeDistCoeffs(const float* dist, int distCount)
{
    if (!dist || distCount <= 0) return cv::Mat();
    return cv::Mat(1, distCount, CV_32F, (void*)dist);
}

template<typename T>
static cv::Ptr<cv::aruco::Dictionary> ToDictPtr(T&& d)
{
    using DT = std::decay_t<T>;
    if constexpr (std::is_same_v<DT, cv::Ptr<cv::aruco::Dictionary>>)
    {
        return d;
    }
    else 
    {
        return cv::makePtr<cv::aruco::Dictionary>(d);
    }
}

static cv::Ptr<cv::aruco::Dictionary> GetDictPtr(int dictId)
{
    switch (dictId)
    {
        case 1: return ToDictPtr(cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_100));
        case 2: return ToDictPtr(cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250));
        case 3: return ToDictPtr(cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL));
        case 0:
        default: return ToDictPtr(cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50));
    }
}

//API


extern "C" int ARUCO_UNITY_CALL DetectArucoGray8(
    const uint8_t* gray, int width, int height,
    const float* K9,
    const float* dist, int distCount,
    float markerLength,
    int dictId,
    int* outIds,
    float* outRvecs,
    float* outTvecs,
    int maxMarkers)
{
    if (!gray || width <= 0 || height <= 0) return 0;
    if (!K9 || !outIds || !outRvecs || !outTvecs) return 0;
    if (maxMarkers <= 0) return 0;

    cv::Mat img(height, width, CV_8UC1, (void*)gray);

    //bool isSaved = cv::imwrite("C:/Users/Eugene/AppData/LocalLow/DefaultCompany/ArUcoTest/dump_cpp.png", img);

    cv::Mat K(3, 3, CV_32F, (void*)K9);
    cv::Mat distCoeffs = MakeDistCoeffs(dist, distCount);

    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners;
    std::vector<std::vector<cv::Point2f>> rejected;

    auto dict = GetDictPtr(dictId);

    auto params = cv::makePtr<cv::aruco::DetectorParameters>();
    cv::aruco::detectMarkers(img, dict, corners, ids, params, rejected);

    int n = (int)ids.size();
    if (n <= 0) return 0;
    if (n > maxMarkers) n = maxMarkers;

    std::memset(outIds, 0, sizeof(int) * (size_t)maxMarkers);
    std::memset(outRvecs, 0, sizeof(float) * (size_t)maxMarkers * 3);
    std::memset(outTvecs, 0, sizeof(float) * (size_t)maxMarkers * 3);

    if (markerLength > 0.0f)
    {
        std::vector<cv::Vec3d> rvecs, tvecs;
        cv::aruco::estimatePoseSingleMarkers(corners, (double)markerLength, K, distCoeffs, rvecs, tvecs);

        for (int i = 0; i < n; i++)
        {
            outIds[i] = ids[i];

            outRvecs[i * 3 + 0] = (float)rvecs[i][0];
            outRvecs[i * 3 + 1] = (float)rvecs[i][1];
            outRvecs[i * 3 + 2] = (float)rvecs[i][2];

            outTvecs[i * 3 + 0] = (float)tvecs[i][0];
            outTvecs[i * 3 + 1] = (float)tvecs[i][1];
            outTvecs[i * 3 + 2] = (float)tvecs[i][2];
        }
    }
    else
    {
        for (int i = 0; i < n; i++)
            outIds[i] = ids[i];
    }

    return n;
}

extern "C" int ARUCO_UNITY_CALL DetectArucoEncodedImage(
    const uint8_t* encoded, int encodedLen,
    const float* K9,
    const float* dist, int distCount,
    float markerLength,
    int dictId,
    int* outIds,
    float* outRvecs,
    float* outTvecs,
    int maxMarkers)
{
    if (!encoded || encodedLen <= 0) return 0;
    if (!K9 || !outIds || !outRvecs || !outTvecs) return 0;
    
    cv::Mat buf(1, encodedLen, CV_8U,  (void*)encoded);
    cv::Mat gray = cv::imdecode(buf, cv::IMREAD_GRAYSCALE);
    if (gray.empty()) return 0;

    return DetectArucoGray8(
        gray.data, gray.cols, gray.rows,
        K9,
        dist, distCount,
        markerLength,
        dictId,
        outIds,
        outRvecs,
        outTvecs,
        maxMarkers
    );
}


extern "C" int ARUCO_UNITY_CALL DetectArucoRGBA8(
    const uint8_t* rgba, int width, int height,
    const float* K9,
    const float* dist, int distCount,
    float markerLength,
    int dictId,
    int* outIds,
    float* outRvecs,
    float* outTvecs,
    int maxMarkers
) 
{
    if(!rgba || width <= 0 || height <= 0) return 0;
    
    cv::Mat rgbaMat(height, width, CV_8UC4, (void*)rgba);
    
    cv::Mat gray;
    cv::cvtColor(rgbaMat, gray, cv::COLOR_RGBA2GRAY);

    return DetectArucoGray8(
        gray.data, gray.cols, gray.rows,
        K9,
        dist, distCount,
        markerLength,
        dictId,
        outIds,
        outRvecs,
        outTvecs,
        maxMarkers
    );
}


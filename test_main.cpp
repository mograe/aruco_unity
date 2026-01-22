#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "aruco_unity.h"

static bool ReadFileBytes(const std::string& path, std::vector<uint8_t>& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;

    f.seekg(0, std::ios::end);
    std::streamoff size = f.tellg();
    if (size <= 0) return false;
    f.seekg(0, std::ios::beg);

    out.resize(static_cast<size_t>(size));

    f.read(reinterpret_cast<char*>(out.data()), size);
    return f.good();
}


int main(int argc, char** argv)
{
    const std::string path = (argc > 1) ? argv[1] : "test1.jpg";

    std::vector<uint8_t> encoded;
    if (!ReadFileBytes(path, encoded))
    {
        std::cerr << "Failed to read file " << path << std::endl;
        return 1;
    }

    float fx = 800.0f;
    float fy = 800.0f;
    float cx = 640.0f;
    float cy = 360.0f;

    float K9[9] = {
        fx, 0.0f, cx,
        0.0f, fy, cy,
        0.0f, 0.0f, 1.0f
    };

    const int Max = 64;
    std::vector<int> ids(Max);
    std::vector<float> rvecs(Max * 3);
    std::vector<float> tvecs(Max * 3);

    int dictId = 2;
    float markerLength = 0.05f;

    int n = DetectArucoEncodedImage(
        encoded.data(), static_cast<int>(encoded.size()),
        K9,
        nullptr, 0,
        markerLength,
        dictId,
        ids.data(),
        rvecs.data(),
        tvecs.data(),
        Max
    );

    std::cout << "Detected " << n << " markers" << std::endl;
    for (int i = 0; i < n; i++)
    {
        std::cout << "ID=" << ids[i];

        const float tx = tvecs[i * 3 + 0];
        const float ty = tvecs[i * 3 + 1];
        const float tz = tvecs[i * 3 + 2];
        std::cout << " tx=" << tx << " ty=" << ty << " tz=" << tz;

        std::cout << std::endl;
    }

    return 0;
}

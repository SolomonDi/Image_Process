#pragma once

#include"im_read.hpp"
#include<cmath>
#include<cstdint>


struct ImageStats {

    uint16_t min = 0;
    uint16_t max = 0;
    double mean = 0.0;
    double stdDev = 0.0;
    double effectBits = 0.0;
    
};


inline ImageStats compute_stats(const RawImage &raw) {

    ImageStats stats;
    if (raw.data.empty()) return stats;

    stats.min = raw.data[0];
    stats.max = raw.data[0];
    double sum = 0;
    double sumSq = 0;

    for (const auto &value : raw.data) {
        sum += value;

        if (value < stats.min) stats.min = value;
        if (value > stats.max) stats.max = value;

    }

    stats.mean = sum / raw.data.size();

    for (const auto &value : raw.data) {

        double diff = value - stats.mean;
        sumSq += diff * diff;

    }

    stats.stdDev = std::sqrt(sumSq / raw.data.size());

    double range = static_cast<double>(stats.max - stats.min);
    stats.effectBits = (range > 0.0) ? std::log2(range) : 0.0;

    return stats;

}


inline ImageStats compute_region(const RawImage &raw, int x0, int y0, 
    int width, int height) {

    ImageStats stats;
    if (raw.data.empty()) return stats;

    std::vector<uint16_t> regionData;
    regionData.reserve(static_cast<std::size_t>(width) * height);

    for (int y = y0; y < y0 + height; ++y) {

        for (int x = x0; x < x0 + width; ++x) {

            regionData.push_back(raw.data[static_cast<std::size_t>(y) 
                * raw.width + x]);
        }
    }

    RawImage raw_image;

    raw_image.data = std::move(regionData);
    raw_image.width = width;
    raw_image.height = height;

    return compute_stats(raw_image);

}
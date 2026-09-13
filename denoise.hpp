#pragma once

#include"im_read.hpp"

#include<algorithm>
#include<array>
#include<atomic>
#include<chrono>
#include<cmath>
#include<functional>
#include<future>
#include<thread>
#include<vector>


enum class DenoiseMethod { Gaussian, Median, Bilateral };


inline const char* denoise_method_name(DenoiseMethod method) {

    if (method == DenoiseMethod::Median) return "median";
    if (method == DenoiseMethod::Bilateral) return "bilateral";
    return "gaussian";
}


struct DenoiseParams {

    DenoiseMethod method = DenoiseMethod::Gaussian;
    double sigma = 1.f;
    int medianSize = 3;
    double rangeSigma = 40.f;
};


struct CfaPlane {

    int width = 0;
    int height = 0;
    std::vector<float> data;
};


struct PlaneIndex {

    std::vector<int> column;
    std::vector<std::size_t> row;
    int radius = 0;

    PlaneIndex(const CfaPlane &plane, int reach) : radius(reach) {

        column.resize(static_cast<std::size_t>(plane.width) + 2 * reach);
        row.resize(static_cast<std::size_t>(plane.height) + 2 * reach);

        for (int i = 0; i < static_cast<int>(column.size()); ++i)
            column[i] = std::min(std::max(i - reach, 0), plane.width - 1);

        for (int j = 0; j < static_cast<int>(row.size()); ++j)
            row[j] = static_cast<std::size_t>(std::min(std::max(j - reach, 0), plane.height - 1))
                     * plane.width;
    }

    std::size_t at(int x, int y) const { return row[y + radius] + column[x + radius]; }
};


inline int denoise_radius(const DenoiseParams &params) {

    if (params.method == DenoiseMethod::Median)
        return std::max(1, params.medianSize / 2);

    if (params.method == DenoiseMethod::Bilateral)
        return std::max(1, static_cast<int>(std::ceil(2.0 * params.sigma)));

    return std::max(1, static_cast<int>(std::ceil(3.0 * params.sigma)));
}


inline int denoise_passes(const DenoiseParams &params) {

    return params.method == DenoiseMethod::Gaussian ? 2 : 1;
}


inline bool denoise_run_rows(int rows, const std::function<void(int)> &body,
                             std::atomic<int> &progress, int total,
                             std::atomic<bool> &stopped,
                             const std::function<bool(int, int)> &report) {

    if (rows <= 0) return !stopped.load();

    int workers = parallel_workers(rows, 16);
    int rowsPerWorker = (rows + workers - 1) / workers;
    int target = progress.load() + rows;

    std::vector<std::future<void>> tasks;

    for (int i = 0; i < workers; ++i) {

        int fromRow = i * rowsPerWorker;
        int toRow = std::min(rows, fromRow + rowsPerWorker);
        if (fromRow >= toRow) break;

        tasks.push_back(std::async(std::launch::async, [&, fromRow, toRow]() {

            for (int y = fromRow; y < toRow; ++y) {

                if (stopped.load(std::memory_order_relaxed)) return;

                body(y);
                progress.fetch_add(1, std::memory_order_relaxed);
            }
        }));
    }

    while (true) {

        int done = progress.load(std::memory_order_relaxed);

        if (report && !report(done, total)) stopped.store(true);
        if (stopped.load() || done >= target) break;

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    for (auto &task : tasks) task.wait();

    return !stopped.load();
}


inline bool denoise_gaussian_plane(const CfaPlane &source, CfaPlane &result, double sigma,
                                   std::atomic<int> &progress, int total,
                                   std::atomic<bool> &stopped,
                                   const std::function<bool(int, int)> &report) {

    sigma = std::max(0.1, sigma);
    int radius = std::max(1, static_cast<int>(std::ceil(3.0 * sigma)));

    std::vector<float> kernel(2 * radius + 1);
    double weightSum = 0.0;

    for (int i = -radius; i <= radius; ++i) {
        double weight = std::exp(-static_cast<double>(i) * i / (2.0 * sigma * sigma));
        kernel[i + radius] = static_cast<float>(weight);
        weightSum += weight;
    }

    for (float &weight : kernel) weight = static_cast<float>(weight / weightSum);

    PlaneIndex index(source, radius);

    CfaPlane horizontal;
    horizontal.width = source.width;
    horizontal.height = source.height;
    horizontal.data.assign(source.data.size(), 0.f);

    bool finished = denoise_run_rows(source.height, [&](int y) {

        float *out = horizontal.data.data() + static_cast<std::size_t>(y) * source.width;

        for (int x = 0; x < source.width; ++x) {

            float accumulated = 0.f;

            for (int i = -radius; i <= radius; ++i)
                accumulated += kernel[i + radius] * source.data[index.at(x + i, y)];

            out[x] = accumulated;
        }

    }, progress, total, stopped, report);

    if (!finished) return false;

    result.width = source.width;
    result.height = source.height;
    result.data.assign(source.data.size(), 0.f);

    return denoise_run_rows(source.height, [&](int y) {

        float *out = result.data.data() + static_cast<std::size_t>(y) * source.width;

        for (int x = 0; x < source.width; ++x) {

            float accumulated = 0.f;

            for (int i = -radius; i <= radius; ++i)
                accumulated += kernel[i + radius] * horizontal.data[index.at(x, y + i)];

            out[x] = accumulated;
        }

    }, progress, total, stopped, report);
}


inline bool denoise_median_plane(const CfaPlane &source, CfaPlane &result, int size,
                                 std::atomic<int> &progress, int total,
                                 std::atomic<bool> &stopped,
                                 const std::function<bool(int, int)> &report) {

    int radius = std::max(1, std::min(3, size / 2));
    int count = (2 * radius + 1) * (2 * radius + 1);

    PlaneIndex index(source, radius);

    result.width = source.width;
    result.height = source.height;
    result.data.assign(source.data.size(), 0.f);

    return denoise_run_rows(source.height, [&](int y) {

        float window[49];
        float *out = result.data.data() + static_cast<std::size_t>(y) * source.width;

        for (int x = 0; x < source.width; ++x) {

            int n = 0;

            for (int dy = -radius; dy <= radius; ++dy)
                for (int dx = -radius; dx <= radius; ++dx)
                    window[n++] = source.data[index.at(x + dx, y + dy)];

            std::nth_element(window, window + count / 2, window + count);
            out[x] = window[count / 2];
        }

    }, progress, total, stopped, report);
}


inline bool denoise_bilateral_plane(const CfaPlane &source, CfaPlane &result,
                                    double sigma, double rangeSigma,
                                    std::atomic<int> &progress, int total,
                                    std::atomic<bool> &stopped,
                                    const std::function<bool(int, int)> &report) {

    sigma = std::max(0.1, sigma);
    rangeSigma = std::max(0.5, rangeSigma);

    int radius = std::max(1, static_cast<int>(std::ceil(2.0 * sigma)));
    int side = 2 * radius + 1;

    std::vector<float> spatial(static_cast<std::size_t>(side) * side);

    for (int dy = -radius; dy <= radius; ++dy)
        for (int dx = -radius; dx <= radius; ++dx)
            spatial[(dy + radius) * side + (dx + radius)] = static_cast<float>(
                std::exp(-static_cast<double>(dx * dx + dy * dy) / (2.0 * sigma * sigma)));

    int lutSize = static_cast<int>(std::ceil(6.0 * rangeSigma)) + 1;
    std::vector<float> similarity(lutSize);

    for (int d = 0; d < lutSize; ++d)
        similarity[d] = static_cast<float>(
            std::exp(-static_cast<double>(d) * d / (2.0 * rangeSigma * rangeSigma)));

    PlaneIndex index(source, radius);

    result.width = source.width;
    result.height = source.height;
    result.data.assign(source.data.size(), 0.f);

    return denoise_run_rows(source.height, [&](int y) {

        float *out = result.data.data() + static_cast<std::size_t>(y) * source.width;

        for (int x = 0; x < source.width; ++x) {

            float center = source.data[index.at(x, y)];
            float weightSum = 0.f;
            float valueSum = 0.f;

            for (int dy = -radius; dy <= radius; ++dy) {

                for (int dx = -radius; dx <= radius; ++dx) {

                    float value = source.data[index.at(x + dx, y + dy)];
                    int difference = static_cast<int>(std::fabs(value - center) + 0.5f);

                    if (difference >= lutSize) continue;

                    float weight = spatial[(dy + radius) * side + (dx + radius)] * similarity[difference];

                    weightSum += weight;
                    valueSum += weight * value;
                }
            }

            out[x] = weightSum > 0.f ? valueSum / weightSum : center;
        }

    }, progress, total, stopped, report);
}


inline bool denoise_frame(RawImage &raw, int x0, int y0, int width, int height,
                          const DenoiseParams &params,
                          const std::function<bool(int, int)> &report) {

    if (raw.data.empty() || width <= 0 || height <= 0) return true;

    x0 = std::max(0, std::min(x0, raw.width - 1));
    y0 = std::max(0, std::min(y0, raw.height - 1));
    width = std::max(1, std::min(width, raw.width - x0));
    height = std::max(1, std::min(height, raw.height - y0));

    int margin = 2 * (denoise_radius(params) + 1);

    int areaX0 = std::max(0, x0 - margin);
    int areaY0 = std::max(0, y0 - margin);
    areaX0 -= areaX0 % 2;
    areaY0 -= areaY0 % 2;

    int areaX1 = std::min(raw.width, x0 + width + margin);
    int areaY1 = std::min(raw.height, y0 + height + margin);

    std::array<CfaPlane, 4> planes;

    for (int k = 0; k < 4; ++k) {

        int offsetX = k & 1;
        int offsetY = (k >> 1) & 1;

        CfaPlane &plane = planes[k];
        plane.width = std::max(0, (areaX1 - areaX0 - offsetX + 1) / 2);
        plane.height = std::max(0, (areaY1 - areaY0 - offsetY + 1) / 2);
        plane.data.resize(static_cast<std::size_t>(plane.width) * plane.height);

        for (int j = 0; j < plane.height; ++j) {

            const uint16_t *row = raw.data.data()
                + static_cast<std::size_t>(areaY0 + offsetY + 2 * j) * raw.width;

            for (int i = 0; i < plane.width; ++i)
                plane.data[static_cast<std::size_t>(j) * plane.width + i] = row[areaX0 + offsetX + 2 * i];
        }
    }

    int total = 0;
    for (const CfaPlane &plane : planes) total += plane.height * denoise_passes(params);

    std::atomic<int> progress{0};
    std::atomic<bool> stopped{false};

    std::array<CfaPlane, 4> filtered;

    for (int k = 0; k < 4; ++k) {

        if (planes[k].width == 0 || planes[k].height == 0) continue;

        bool finished = false;

        if (params.method == DenoiseMethod::Median)
            finished = denoise_median_plane(planes[k], filtered[k], params.medianSize,
                                            progress, total, stopped, report);
        else if (params.method == DenoiseMethod::Bilateral)
            finished = denoise_bilateral_plane(planes[k], filtered[k], params.sigma, params.rangeSigma,
                                               progress, total, stopped, report);
        else
            finished = denoise_gaussian_plane(planes[k], filtered[k], params.sigma,
                                              progress, total, stopped, report);

        if (!finished) return false;
    }

    for (int y = y0; y < y0 + height; ++y) {

        int offsetY = y & 1;
        int j = (y - areaY0 - offsetY) / 2;

        for (int x = x0; x < x0 + width; ++x) {

            int offsetX = x & 1;
            const CfaPlane &plane = filtered[(offsetY << 1) | offsetX];

            int i = (x - areaX0 - offsetX) / 2;
            float value = plane.data[static_cast<std::size_t>(j) * plane.width + i];

            raw.data[static_cast<std::size_t>(y) * raw.width + x] =
                static_cast<uint16_t>(std::min(65535.f, std::max(0.f, value + 0.5f)));
        }
    }

    if (report) report(total, total);
    return true;
}

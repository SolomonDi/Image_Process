#pragma once

#include"im_read.hpp"
#include<cmath>
#include<cstdint>
#include<array>
#include<string>
#include<thread>
#include<future>
#include<vector>


struct ImageStats {

    uint16_t min = 0;
    uint16_t max = 0;
    double mean = 0.0;
    double stdDev = 0.0;
    double rangeBits = 0.0;
    double usefulBits = 0.0;
    double snrDb = 0.0;
    
};


inline void finish_bits(ImageStats &stats, const RawImage &raw) {

    double range = static_cast<double>(stats.max) - static_cast<double>(stats.min);
    stats.rangeBits = range > 0.0 ? std::log2(range + 1.0) : 0.0;

    double fullScale = static_cast<double>(std::max(1, raw.white_level - raw.black_level));
    double signal = stats.mean - static_cast<double>(raw.black_level);

    stats.usefulBits = stats.stdDev > 0.0 ? std::max(0.0, std::log2(fullScale / stats.stdDev)) : 0.0;
    stats.snrDb = (stats.stdDev > 0.0 && signal > 0.0) ? 20.0 * std::log10(signal / stats.stdDev) : 0.0;
}


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

    finish_bits(stats, raw);

    return stats;

}


inline ImageStats compute_region(const RawImage &raw, int x0, int y0,
    int width, int height) {

    ImageStats stats;
    if (raw.data.empty()) return stats;

    struct Partial {
        double sum = 0.0;
        double sumSq = 0.0;
        std::size_t count = 0;
        uint16_t min = 65535;
        uint16_t max = 0;
    };

    int workers = parallel_workers(height);
    int rowsPerWorker = (height + workers - 1) / workers;

    std::vector<Partial> partials(workers);
    std::vector<std::future<void>> tasks;

    for (int i = 0; i < workers; ++i) {

        int fromRow = y0 + i * rowsPerWorker;
        int toRow = std::min(y0 + height, fromRow + rowsPerWorker);
        if (fromRow >= toRow) break;

        tasks.push_back(std::async(std::launch::async, [&, fromRow, toRow, i]() {

            double localSum = 0.0;
            std::size_t localCount = 0;
            uint16_t localMin = 65535;
            uint16_t localMax = 0;

            for (int y = fromRow; y < toRow; ++y) {

                const uint16_t *row = raw.data.data() + static_cast<std::size_t>(y) * raw.width;

                for (int x = x0; x < x0 + width; ++x) {

                    uint16_t value = row[x];
                    localSum += value;
                    localCount++;
                    if (value < localMin) localMin = value;
                    if (value > localMax) localMax = value;
                }
            }

            Partial &part = partials[i];
            part.sum = localSum;
            part.count = localCount;
            part.min = localMin;
            part.max = localMax;
        }));
    }

    for (auto &task : tasks) task.wait();
    tasks.clear();

    std::size_t total = 0;
    double sum = 0.0;
    uint16_t low = 65535, high = 0;

    for (const Partial &part : partials) {

        if (part.count == 0) continue;

        total += part.count;
        sum += part.sum;
        low = std::min(low, part.min);
        high = std::max(high, part.max);
    }

    if (total == 0) return stats;

    stats.min = low;
    stats.max = high;
    stats.mean = sum / total;

    for (int i = 0; i < workers; ++i) {

        int fromRow = y0 + i * rowsPerWorker;
        int toRow = std::min(y0 + height, fromRow + rowsPerWorker);
        if (fromRow >= toRow) break;

        tasks.push_back(std::async(std::launch::async, [&, fromRow, toRow, i]() {

            double local = 0.0;

            for (int y = fromRow; y < toRow; ++y) {

                const uint16_t *row = raw.data.data() + static_cast<std::size_t>(y) * raw.width;

                for (int x = x0; x < x0 + width; ++x) {
                    double diff = row[x] - stats.mean;
                    local += diff * diff;
                }
            }

            partials[i].sumSq = local;
        }));
    }

    for (auto &task : tasks) task.wait();

    double sumSq = 0.0;
    for (const Partial &part : partials) sumSq += part.sumSq;

    stats.stdDev = std::sqrt(sumSq / total);

    finish_bits(stats, raw);

    return stats;
}


inline RawImage region_crop(const RawImage &raw, int x0, int y0, int width, int height) {

    x0 -= (x0 % 2);
    y0 -= (y0 % 2);
    width += (width % 2);
    height += (height % 2);

    x0 = std::max(0, std::min(x0, raw.width - width));
    y0 = std::max(0, std::min(y0, raw.height - height));


    RawImage cropped;

    cropped.width = width;
    cropped.height = height;
    cropped.black_level = raw.black_level;
    cropped.white_level = raw.white_level;
    cropped.cfaPattern = raw.cfaPattern;
    cropped.wbR = raw.wbR;
    cropped.wbG = raw.wbG;
    cropped.wbB = raw.wbB;
    cropped.camera = raw.camera;
    cropped.lens = raw.lens;
    cropped.iso = raw.iso;
    cropped.shutter = raw.shutter;
    cropped.aperture = raw.aperture;
    cropped.focal = raw.focal;
    
    cropped.data.reserve(static_cast<std::size_t>(width) * height);
    for (int y = y0; y < y0 + height; ++y) {

        for (int x = x0; x < x0 + width; ++x) {

            cropped.data.push_back(raw.data[static_cast<std::size_t>(y) * raw.width + x]);

        }

    }

    return cropped;

}


inline int cfa_index(int x, int y) { return ((y & 1) << 1) | (x & 1); }


inline std::array<std::string, 4> cfa_labels(const RawImage &raw) {

    std::array<std::string, 4> labels;
    int greens = 0;

    for (int y = 0; y < 2; ++y) {

        for (int x = 0; x < 2; ++x) {

            int idx = cfa_index(x, y);
            int code = raw.color_at(x, y);

            if (code == 0)      labels[idx] = "R";
            else if (code == 2) labels[idx] = "B";
            else                labels[idx] = "G" + std::to_string(++greens);
        }
    }

    return labels;
}


inline std::string cfa_pattern_name(const RawImage &raw) {

    std::array<std::string, 4> labels = cfa_labels(raw);
    std::string name;

    for (int i = 0; i < 4; ++i) name += labels[i].substr(0, 1);
    return name;
}


inline void clamp_region(const RawImage &raw, int &x0, int &y0, int &width, int &height) {

    x0 = std::max(0, std::min(x0, raw.width - 1));
    y0 = std::max(0, std::min(y0, raw.height - 1));
    width = std::max(1, std::min(width, raw.width - x0));
    height = std::max(1, std::min(height, raw.height - y0));
}


struct ChannelStats {

    ImageStats stats[4];
    std::size_t counts[4] = {0, 0, 0, 0};
    std::array<std::string, 4> labels;
};


inline ChannelStats compute_region_channels(const RawImage &raw, int x0, int y0,
                                            int width, int height) {

    ChannelStats result;
    if (raw.data.empty()) return result;

    result.labels = cfa_labels(raw);
    clamp_region(raw, x0, y0, width, height);

    struct Partial {
        double sum[4] = {0.0, 0.0, 0.0, 0.0};
        double sumSq[4] = {0.0, 0.0, 0.0, 0.0};
        std::size_t count[4] = {0, 0, 0, 0};
        uint16_t min[4] = {65535, 65535, 65535, 65535};
        uint16_t max[4] = {0, 0, 0, 0};
    };

    int workers = parallel_workers(height);
    int rowsPerWorker = (height + workers - 1) / workers;

    std::vector<Partial> partials(workers);
    std::vector<std::future<void>> tasks;

    for (int i = 0; i < workers; ++i) {

        int fromRow = y0 + i * rowsPerWorker;
        int toRow = std::min(y0 + height, fromRow + rowsPerWorker);
        if (fromRow >= toRow) break;

        tasks.push_back(std::async(std::launch::async, [&, fromRow, toRow, i]() {

            double localSum[4] = {0.0, 0.0, 0.0, 0.0};
            std::size_t localCount[4] = {0, 0, 0, 0};
            uint16_t localMin[4] = {65535, 65535, 65535, 65535};
            uint16_t localMax[4] = {0, 0, 0, 0};

            for (int y = fromRow; y < toRow; ++y) {

                const uint16_t *row = raw.data.data() + static_cast<std::size_t>(y) * raw.width;

                for (int x = x0; x < x0 + width; ++x) {

                    int k = cfa_index(x, y);
                    uint16_t value = row[x];

                    localSum[k] += value;
                    localCount[k]++;
                    if (value < localMin[k]) localMin[k] = value;
                    if (value > localMax[k]) localMax[k] = value;
                }
            }

            Partial &part = partials[i];

            for (int k = 0; k < 4; ++k) {
                part.sum[k] = localSum[k];
                part.count[k] = localCount[k];
                part.min[k] = localMin[k];
                part.max[k] = localMax[k];
            }
        }));
    }

    for (auto &task : tasks) task.wait();
    tasks.clear();

    double mean[4] = {0.0, 0.0, 0.0, 0.0};

    for (int k = 0; k < 4; ++k) {

        std::size_t total = 0;
        double sum = 0.0;
        uint16_t low = 65535, high = 0;

        for (const Partial &part : partials) {

            if (part.count[k] == 0) continue;

            total += part.count[k];
            sum += part.sum[k];
            low = std::min(low, part.min[k]);
            high = std::max(high, part.max[k]);
        }

        result.counts[k] = total;
        if (total == 0) continue;

        mean[k] = sum / total;

        result.stats[k].min = low;
        result.stats[k].max = high;
        result.stats[k].mean = mean[k];
    }

    for (int i = 0; i < workers; ++i) {

        int fromRow = y0 + i * rowsPerWorker;
        int toRow = std::min(y0 + height, fromRow + rowsPerWorker);
        if (fromRow >= toRow) break;

        tasks.push_back(std::async(std::launch::async, [&, fromRow, toRow, i]() {

            double local[4] = {0.0, 0.0, 0.0, 0.0};

            for (int y = fromRow; y < toRow; ++y) {

                const uint16_t *row = raw.data.data() + static_cast<std::size_t>(y) * raw.width;

                for (int x = x0; x < x0 + width; ++x) {

                    int k = cfa_index(x, y);
                    double diff = row[x] - mean[k];
                    local[k] += diff * diff;
                }
            }

            for (int k = 0; k < 4; ++k) partials[i].sumSq[k] = local[k];
        }));
    }

    for (auto &task : tasks) task.wait();

    for (int k = 0; k < 4; ++k) {

        if (result.counts[k] == 0) continue;

        double sumSq = 0.0;
        for (const Partial &part : partials) sumSq += part.sumSq[k];

        result.stats[k].stdDev = std::sqrt(sumSq / result.counts[k]);
        finish_bits(result.stats[k], raw);
    }

    return result;
}


inline std::array<std::vector<uint32_t>, 4> compute_histogram(const RawImage &raw,
        int x0, int y0, int width, int height, int bins) {

    std::array<std::vector<uint32_t>, 4> hist;
    for (auto &channel : hist) channel.assign(std::max(1, bins), 0u);

    if (raw.data.empty() || bins <= 0) return hist;

    clamp_region(raw, x0, y0, width, height);

    double top = static_cast<double>(std::max(1, raw.white_level));

    int workers = parallel_workers(height);
    int rowsPerWorker = (height + workers - 1) / workers;

    std::vector<std::array<std::vector<uint32_t>, 4>> partials(workers);
    std::vector<std::future<void>> tasks;

    for (int i = 0; i < workers; ++i) {

        int fromRow = y0 + i * rowsPerWorker;
        int toRow = std::min(y0 + height, fromRow + rowsPerWorker);
        if (fromRow >= toRow) break;

        for (auto &channel : partials[i]) channel.assign(bins, 0u);

        tasks.push_back(std::async(std::launch::async, [&, fromRow, toRow, i]() {

            for (int y = fromRow; y < toRow; ++y) {

                const uint16_t *row = raw.data.data() + static_cast<std::size_t>(y) * raw.width;

                for (int x = x0; x < x0 + width; ++x) {

                    int bin = static_cast<int>(row[x] / top * (bins - 1));
                    bin = std::max(0, std::min(bin, bins - 1));

                    partials[i][cfa_index(x, y)][bin]++;
                }
            }
        }));
    }

    for (auto &task : tasks) task.wait();

    for (const auto &partial : partials)
        for (int k = 0; k < 4; ++k)
            if (!partial[k].empty())
                for (int bin = 0; bin < bins; ++bin) hist[k][bin] += partial[k][bin];

    return hist;
}


inline std::array<std::vector<uint32_t>, 4> compute_noise_histogram(const RawImage &raw,
        int x0, int y0, int width, int height, int bins, double span,
        const std::array<double, 4> &mean) {

    std::array<std::vector<uint32_t>, 4> hist;
    for (auto &channel : hist) channel.assign(std::max(1, bins), 0u);

    if (raw.data.empty() || bins <= 0 || span <= 0.0) return hist;

    clamp_region(raw, x0, y0, width, height);

    int workers = parallel_workers(height);
    int rowsPerWorker = (height + workers - 1) / workers;

    std::vector<std::array<std::vector<uint32_t>, 4>> partials(workers);
    std::vector<std::future<void>> tasks;

    for (int i = 0; i < workers; ++i) {

        int fromRow = y0 + i * rowsPerWorker;
        int toRow = std::min(y0 + height, fromRow + rowsPerWorker);
        if (fromRow >= toRow) break;

        for (auto &channel : partials[i]) channel.assign(bins, 0u);

        tasks.push_back(std::async(std::launch::async, [&, fromRow, toRow, i]() {

            for (int y = fromRow; y < toRow; ++y) {

                const uint16_t *row = raw.data.data() + static_cast<std::size_t>(y) * raw.width;

                for (int x = x0; x < x0 + width; ++x) {

                    int k = cfa_index(x, y);
                    double deviation = row[x] - mean[k];

                    int bin = static_cast<int>((deviation + span) / (2.0 * span) * (bins - 1));
                    if (bin < 0 || bin >= bins) continue;

                    partials[i][k][bin]++;
                }
            }
        }));
    }

    for (auto &task : tasks) task.wait();

    for (const auto &partial : partials)
        for (int k = 0; k < 4; ++k)
            if (!partial[k].empty())
                for (int bin = 0; bin < bins; ++bin) hist[k][bin] += partial[k][bin];

    return hist;
}


inline std::vector<double> compute_profile(const RawImage &raw, int x0, int y0,
                                           int width, int height, bool alongX) {

    std::vector<double> profile;
    if (raw.data.empty()) return profile;

    clamp_region(raw, x0, y0, width, height);

    if (alongX) {

        profile.assign(width, 0.0);

        for (int x = 0; x < width; ++x) {

            double sum = 0.0;
            for (int y = y0; y < y0 + height; ++y)
                sum += raw.data[static_cast<std::size_t>(y) * raw.width + x0 + x];

            profile[x] = sum / height;
        }

    } else {

        profile.assign(height, 0.0);

        for (int y = 0; y < height; ++y) {

            double sum = 0.0;
            for (int x = x0; x < x0 + width; ++x)
                sum += raw.data[static_cast<std::size_t>(y0 + y) * raw.width + x];

            profile[y] = sum / width;
        }
    }

    return profile;
}


struct NoiseReport {

    double diffMean[4] = {0.0, 0.0, 0.0, 0.0};
    double temporal[4] = {0.0, 0.0, 0.0, 0.0};
    double total[4] = {0.0, 0.0, 0.0, 0.0};
    double fpn[4] = {0.0, 0.0, 0.0, 0.0};
    std::array<std::string, 4> labels;
    bool valid = false;
};


inline NoiseReport compute_two_frame(const RawImage &first, const RawImage &second,
                                     int x0, int y0, int width, int height) {

    NoiseReport report;
    if (first.data.empty() || second.data.empty()) return report;

    if (first.width != second.width || first.height != second.height) return report;

    report.labels = cfa_labels(first);
    clamp_region(first, x0, y0, width, height);

    double sumD[4] = {0, 0, 0, 0}, sumD2[4] = {0, 0, 0, 0};
    double sumA[4] = {0, 0, 0, 0}, sumA2[4] = {0, 0, 0, 0};
    std::size_t count[4] = {0, 0, 0, 0};

    for (int y = y0; y < y0 + height; ++y) {

        for (int x = x0; x < x0 + width; ++x) {

            std::size_t i = static_cast<std::size_t>(y) * first.width + x;

            double a = first.data[i];
            double b = second.data[i];
            double d = a - b;

            int k = cfa_index(x, y);

            sumD[k] += d;
            sumD2[k] += d * d;
            sumA[k] += a;
            sumA2[k] += a * a;
            count[k]++;
        }
    }

    for (int k = 0; k < 4; ++k) {

        if (count[k] == 0) continue;

        double n = static_cast<double>(count[k]);

        double meanD = sumD[k] / n;
        double varD = std::max(0.0, sumD2[k] / n - meanD * meanD);

        double meanA = sumA[k] / n;
        double varA = std::max(0.0, sumA2[k] / n - meanA * meanA);

        report.diffMean[k] = meanD;
        report.temporal[k] = std::sqrt(varD / 2.0);
        report.total[k] = std::sqrt(varA);
        report.fpn[k] = std::sqrt(std::max(0.0, varA - varD / 2.0));
    }

    report.valid = true;
    return report;
}


inline RawImage subtract_dark(const RawImage &light, const RawImage &dark) {

    RawImage out = light;

    if (light.width != dark.width || light.height != dark.height) return out;

    int top = light.white_level > 0 ? light.white_level : 65535;

    for (std::size_t i = 0; i < out.data.size(); ++i) {

        int value = static_cast<int>(light.data[i]) - static_cast<int>(dark.data[i])
                    + light.black_level;

        out.data[i] = static_cast<uint16_t>(std::max(0, std::min(value, top)));
    }

    return out;
}
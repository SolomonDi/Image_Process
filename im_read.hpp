#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include"stb_image_write.h"

#include<libraw/libraw.h>
#include<tiffio.h>
#include<fstream>
#include<vector>
#include<string>
#include<stdexcept>
#include<cstdint>
#include<optional>
#include<functional>
#include<thread>
#include<future>
#include<atomic>
#include<chrono> 
#include<algorithm> 
#include<cmath>     
#include<iostream>

//constexpr float Vgamma = 1.0f / 2.2f;

//class for reading of image
class Image {
public:

    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<uint8_t> data;

public:
    std::size_t byte_size(void) const { return data.size();}
    bool data_is_empty(void) const {return data.empty();}

    //access to a pixel(return a reference to modify it)
    uint8_t& at(int x, int y, int z) {
    return (data.at((size_t)(y * width + x) * channels + z));
    }

    //check a pixel using coordinate
    uint8_t at(int x, int y, int z) const {
    return (data.at((size_t)(y * width + x) * channels + z));
    }

};

class Image16 {
public:

    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<uint16_t> data;
};


class RawImage{
public:
    int width = 0;
    int height = 0;
    int black_level = 0;
    int white_level = 0;
    unsigned cfaPattern = 0;
    float wbR = 1.f, wbG = 1.f, wbB = 1.f;
    std::vector<uint16_t> data;

    int flip = 0;
    int activeX = 0;
    int activeY = 0;
    int fullWidth = 0;
    int fullHeight = 0;

    std::vector<uint16_t> obData;
    int obWidth = 0;
    int obHeight = 0;

    std::string camera;
    std::string lens;
    float iso = 0.f;
    float shutter = 0.f;
    float aperture = 0.f;
    float focal = 0.f;
    
public:
    // what color at x,y pos
    int color_at(int x, int y) const {

        return (cfaPattern >> ((((y << 1) & 14) | (x & 1)) << 1)) & 3;
    }

    std::size_t size () const {return data.size() * sizeof(uint16_t);}
};

//func for image loading
inline std::optional<Image> load_image(const std::string& path, int desiredChannels = 0) {

    int w = 0;
    int h = 0;
    int ch = 0;

    uint8_t* raw_im = stbi_load(path.c_str(), &w, &h, &ch, desiredChannels);
    if (!raw_im) return std::nullopt;

    Image image;
    image.width = w;
    image.height = h;
    image.channels = desiredChannels ? desiredChannels : ch;

    image.data.assign(raw_im, raw_im + (size_t)w * h * image.channels);

    stbi_image_free(raw_im);

    return image;
}

inline std::optional<RawImage> load_raw(const std::string &path) {

    LibRaw processor;

    int rc = processor.open_file(path.c_str());
    if (rc != LIBRAW_SUCCESS) {
        std::cerr << "load_raw: open_file(\"" << path << "\") failed: "
                  << libraw_strerror(rc) << "\n";
        return std::nullopt;
    }

    rc = processor.unpack();
    if (rc != LIBRAW_SUCCESS) {
        std::cerr << "load_raw: unpack() failed: " << libraw_strerror(rc) << "\n";
        return std::nullopt;
    }

    auto& sizes = processor.imgdata.sizes;
    auto& color = processor.imgdata.color;
    auto& idata = processor.imgdata.idata;
    auto& raw = processor.imgdata.rawdata;

    if (idata.filters == 9) {
        std::cerr << "load_raw: X-Trans sensor (filters == 9), Bayer demosaic "
                     "is not applicable\n";
        return std::nullopt;
    }

    if (idata.filters == 0) {
        std::cerr << "load_raw: no CFA pattern (filters == 0). The file is not a "
                     "Bayer mosaic: it is a linear/demosaiced RAW ("
                  << idata.colors << " colors per pixel). "
                     "Feed a real sensor RAW (CR2/NEF/ARW/RAF or a non-linear DNG).\n";
        return std::nullopt;
    }

    if (!raw.raw_image) {
        std::cerr << "load_raw: rawdata.raw_image is null (the file stores "
                     "multi-channel data, not a single-channel mosaic)\n";
        return std::nullopt;
    }

    RawImage image;

    image.black_level = color.black;
    image.white_level = color.maximum;
    image.cfaPattern = idata.filters;
    image.flip = sizes.flip;

    
    const std::size_t pitchPx = sizes.raw_pitch
        ? sizes.raw_pitch / sizeof(uint16_t)
        : static_cast<std::size_t>(sizes.raw_width);

    const int fullW = sizes.raw_width;
    const int fullH = sizes.raw_height;

    std::vector<uint16_t> full(static_cast<std::size_t>(fullW) * fullH);
    for (int y = 0; y < fullH; ++y) {
        const uint16_t* src = raw.raw_image + static_cast<std::size_t>(y) * pitchPx;
        std::copy(src, src + fullW, full.begin() + static_cast<std::size_t>(y) * fullW);
    }

    image.fullWidth = fullW;
    image.fullHeight = fullH;

    int marginX = sizes.left_margin - (sizes.left_margin % 2);
    int marginY = sizes.top_margin - (sizes.top_margin % 2);

    int activeW = sizes.width > 0 ? sizes.width : fullW;
    int activeH = sizes.height > 0 ? sizes.height : fullH;

    if (marginX < 0 || marginY < 0 || marginX >= fullW || marginY >= fullH) {
        marginX = 0;
        marginY = 0;
    }

    activeW = std::min(activeW, fullW - marginX);
    activeH = std::min(activeH, fullH - marginY);

    if (activeW <= 0 || activeH <= 0) {
        marginX = 0;
        marginY = 0;
        activeW = fullW;
        activeH = fullH;
    }

    image.width = activeW;
    image.height = activeH;
    image.activeX = marginX;
    image.activeY = marginY;

    image.data.resize(static_cast<std::size_t>(activeW) * activeH);
    for (int y = 0; y < activeH; ++y) {
        const uint16_t* src = full.data() + static_cast<std::size_t>(y + marginY) * fullW + marginX;
        std::copy(src, src + activeW,
                  image.data.begin() + static_cast<std::size_t>(y) * activeW);
    }

    int obX = 2;
    int obW = marginX - 4;
    obW -= obW % 2;

    if (obW >= 4) {

        int obH = activeH - (activeH % 2);

        image.obWidth = obW;
        image.obHeight = obH;
        image.obData.resize(static_cast<std::size_t>(obW) * obH);

        for (int y = 0; y < obH; ++y) {
            const uint16_t* src = full.data() + static_cast<std::size_t>(y + marginY) * fullW + obX;
            std::copy(src, src + obW,
                      image.obData.begin() + static_cast<std::size_t>(y) * obW);
        }

    } else {

        int obY = 2;
        int obH = marginY - 4;
        obH -= obH % 2;

        if (obH >= 4) {

            int obW2 = activeW - (activeW % 2);

            image.obWidth = obW2;
            image.obHeight = obH;
            image.obData.resize(static_cast<std::size_t>(obW2) * obH);

            for (int y = 0; y < obH; ++y) {
                const uint16_t* src = full.data() + static_cast<std::size_t>(y + obY) * fullW + marginX;
                std::copy(src, src + obW2,
                          image.obData.begin() + static_cast<std::size_t>(y) * obW2);
            }
        }
    }

    image.camera = std::string(idata.make) + " " + std::string(idata.model);
    image.lens = std::string(processor.imgdata.lens.Lens);
    image.iso = processor.imgdata.other.iso_speed;
    image.shutter = processor.imgdata.other.shutter;
    image.aperture = processor.imgdata.other.aperture;
    image.focal = processor.imgdata.other.focal_len;

    image.wbR = color.cam_mul[0];
    image.wbG = color.cam_mul[1];
    image.wbB = color.cam_mul[2];

    if (image.wbG > 0.f) {
        image.wbR /= image.wbG;
        image.wbB /= image.wbG;
        image.wbG = 1.f;
    } else {
        image.wbR = image.wbG = image.wbB = 1.f;
    }

    if (image.white_level <= image.black_level) {
        std::cerr << "load_raw: suspicious levels (black=" << image.black_level
                  << ", white=" << image.white_level << ")\n";
    }

    return image;
}

inline bool save_image(const std::string &path, const Image &image) {

    return stbi_write_png(
        path.c_str(),
        image.width, image.height, image.channels,
        image.data.data(),
        image.width * image.channels // stride: one byte for line
    ) != 0;
}

inline Image demosaic_cpu(const RawImage &raw_image) {

    int outW = raw_image.width / 2;
    int outH = raw_image.height / 2;

    Image image;

    image.width = outW;
    image.height = outH;
    image.channels = 3;
    image.data.resize(static_cast<std::size_t>(outW) * outH * 3);

    float range = static_cast<float>(raw_image.white_level - raw_image.black_level);
    if (range <= 0.f) range = 1.f;

    for (int by = 0; by < outH; ++by) {

        for (int bx = 0; bx < outW; ++bx) {

            int x0 = bx * 2;
            int y0 = by *2;

            float sumR = 0.f, sumG = 0.f, sumB = 0.f;   
            int countG = 0;

            for (int dy = 0; dy < 2; ++dy){
                for (int dx = 0; dx < 2; ++dx) {

                    int x = dx + x0;
                    int y = dy + y0;
                    
                    uint16_t rawV = raw_image.data[static_cast<std::size_t>(y) 
                        * raw_image.width + x];
                    
                    float normalized = (static_cast<float>
                        (rawV - raw_image.black_level) / range);
                    
                    normalized = std::min(1.f, std::max(0.f, normalized));
                    
                    int color = raw_image.color_at(x, y);

                    if (color == 0) sumR = normalized;
                    else if (color == 2) sumB = normalized;
                    else { sumG += normalized; ++countG;}
                }
            }

            float avgG = countG > 0 ? sumG / countG : 0.f;

            float r = sumR * raw_image.wbR;
            float g = avgG * raw_image.wbG;
            float b = sumB * raw_image.wbB;

            r = std::min(1.0f, std::max(0.0f, r));
            g = std::min(1.0f, std::max(0.0f, g));
            b = std::min(1.0f, std::max(0.0f, b));

            //Gamma correction R^invGamma
            //r = std::pow(r, Vgamma);
            // g = std::pow(g, Vgamma);
            //b = std::pow(b, Vgamma);

            std::size_t idx = (static_cast<std::size_t>(by) * outW + bx) * 3;
            image.data[idx + 0] = (uint8_t)(r * 255.0f + 0.5f);
            image.data[idx + 1] = (uint8_t)(g * 255.0f + 0.5f);
            image.data[idx + 2] = (uint8_t)(b * 255.0f + 0.5f);

        }
    }

    return image;
}

inline int parallel_workers(int rows, int minimumRowsPerWorker = 64) {

    unsigned hardware = std::thread::hardware_concurrency();
    int available = hardware > 0 ? static_cast<int>(hardware) : 1;

    return std::max(1, std::min(available, std::max(1, rows / minimumRowsPerWorker)));
}


inline Image demosaic_cpu_progress(const RawImage &raw_image,
                                   const std::function<bool(int, int)> &report) {

    int outW = raw_image.width / 2;
    int outH = raw_image.height / 2;

    Image image;

    image.width = outW;
    image.height = outH;
    image.channels = 3;
    image.data.resize(static_cast<std::size_t>(outW) * outH * 3);

    if (outW <= 0 || outH <= 0) return image;

    float range = static_cast<float>(raw_image.white_level - raw_image.black_level);
    if (range <= 0.f) range = 1.f;

    std::atomic<int> rowsDone{0};
    std::atomic<bool> stopped{false};

    auto band = [&](int fromRow, int toRow) {

        for (int by = fromRow; by < toRow; ++by) {

            if (stopped.load(std::memory_order_relaxed)) return;

            for (int bx = 0; bx < outW; ++bx) {

                int x0 = bx * 2;
                int y0 = by * 2;

                float sumR = 0.f, sumG = 0.f, sumB = 0.f;
                int countG = 0;

                for (int dy = 0; dy < 2; ++dy) {
                    for (int dx = 0; dx < 2; ++dx) {

                        int x = dx + x0;
                        int y = dy + y0;

                        uint16_t rawV = raw_image.data[static_cast<std::size_t>(y)
                            * raw_image.width + x];

                        float normalized = (static_cast<float>(rawV - raw_image.black_level) / range);
                        normalized = std::min(1.f, std::max(0.f, normalized));

                        int color = raw_image.color_at(x, y);

                        if (color == 0) sumR = normalized;
                        else if (color == 2) sumB = normalized;
                        else { sumG += normalized; ++countG; }
                    }
                }

                float avgG = countG > 0 ? sumG / countG : 0.f;

                float r = std::min(1.f, std::max(0.f, sumR * raw_image.wbR));
                float g = std::min(1.f, std::max(0.f, avgG * raw_image.wbG));
                float b = std::min(1.f, std::max(0.f, sumB * raw_image.wbB));

                std::size_t idx = (static_cast<std::size_t>(by) * outW + bx) * 3;
                image.data[idx + 0] = (uint8_t)(r * 255.0f + 0.5f);
                image.data[idx + 1] = (uint8_t)(g * 255.0f + 0.5f);
                image.data[idx + 2] = (uint8_t)(b * 255.0f + 0.5f);
            }

            rowsDone.fetch_add(1, std::memory_order_relaxed);
        }
    };

    int workers = parallel_workers(outH);
    int rowsPerWorker = (outH + workers - 1) / workers;

    std::vector<std::future<void>> tasks;

    for (int i = 0; i < workers; ++i) {

        int fromRow = i * rowsPerWorker;
        int toRow = std::min(outH, fromRow + rowsPerWorker);
        if (fromRow >= toRow) break;

        tasks.push_back(std::async(std::launch::async, band, fromRow, toRow));
    }

    if (report) {

        while (true) {

            int done = rowsDone.load(std::memory_order_relaxed);

            if (!report(done, outH)) stopped.store(true);
            if (stopped.load() || done >= outH) break;

            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }

    for (auto &task : tasks) task.wait();

    if (stopped.load()) return Image();

    if (report) report(outH, outH);
    return image;
}


inline uint16_t raw_at(const RawImage &raw, int x, int y) {

    x = std::min(std::max(x, 0), raw.width - 1);
    y = std::min(std::max(y, 0), raw.height - 1);

    return raw.data[static_cast<std::size_t>(y) * raw.width + x];
}


inline Image demosaic_full_progress(const RawImage &raw, bool malvar,
                                    const std::function<bool(int, int)> &report) {

    Image image;

    image.width = raw.width;
    image.height = raw.height;
    image.channels = 3;
    image.data.resize(static_cast<std::size_t>(raw.width) * raw.height * 3);

    if (raw.width <= 0 || raw.height <= 0) return image;

    float range = static_cast<float>(raw.white_level - raw.black_level);
    if (range <= 0.f) range = 1.f;

    const float black = static_cast<float>(raw.black_level);

    std::atomic<int> rowsDone{0};
    std::atomic<bool> stopped{false};

    auto band = [&](int fromRow, int toRow) {

        auto n = [&](int x, int y) {
            return (static_cast<float>(raw_at(raw, x, y)) - black) / range;
        };

        for (int y = fromRow; y < toRow; ++y) {

            if (stopped.load(std::memory_order_relaxed)) return;

            for (int x = 0; x < raw.width; ++x) {

                float v = n(x, y);
                float r, g, b;

                int color = raw.color_at(x, y);

                if (color == 0 || color == 2) {

                    float orthogonal = n(x - 1, y) + n(x + 1, y) + n(x, y - 1) + n(x, y + 1);
                    float diagonal = n(x - 1, y - 1) + n(x + 1, y - 1) +
                                     n(x - 1, y + 1) + n(x + 1, y + 1);
                    float distant = n(x - 2, y) + n(x + 2, y) + n(x, y - 2) + n(x, y + 2);

                    float other;

                    if (malvar) {
                        g = (4.f * v + 2.f * orthogonal - distant) * 0.125f;
                        other = (6.f * v + 2.f * diagonal - 1.5f * distant) * 0.125f;
                    } else {
                        g = orthogonal * 0.25f;
                        other = diagonal * 0.25f;
                    }

                    if (color == 0) { r = v; b = other; }
                    else { b = v; r = other; }

                } else {

                    g = v;

                    float horizontal = n(x - 1, y) + n(x + 1, y);
                    float vertical = n(x, y - 1) + n(x, y + 1);

                    float alongRow, alongColumn;

                    if (malvar) {

                        float diagonal = n(x - 1, y - 1) + n(x + 1, y - 1) +
                                         n(x - 1, y + 1) + n(x + 1, y + 1);

                        float distantH = n(x - 2, y) + n(x + 2, y);
                        float distantV = n(x, y - 2) + n(x, y + 2);

                        alongRow = (5.f * v + 4.f * horizontal - diagonal
                                    - distantH + 0.5f * distantV) * 0.125f;

                        alongColumn = (5.f * v + 4.f * vertical - diagonal
                                       - distantV + 0.5f * distantH) * 0.125f;

                    } else {

                        alongRow = horizontal * 0.5f;
                        alongColumn = vertical * 0.5f;
                    }

                    if (raw.color_at(x - 1, y) == 0) { r = alongRow; b = alongColumn; }
                    else { b = alongRow; r = alongColumn; }
                }

                r = std::min(1.f, std::max(0.f, r * raw.wbR));
                g = std::min(1.f, std::max(0.f, g * raw.wbG));
                b = std::min(1.f, std::max(0.f, b * raw.wbB));

                std::size_t idx = (static_cast<std::size_t>(y) * raw.width + x) * 3;
                image.data[idx + 0] = (uint8_t)(r * 255.0f + 0.5f);
                image.data[idx + 1] = (uint8_t)(g * 255.0f + 0.5f);
                image.data[idx + 2] = (uint8_t)(b * 255.0f + 0.5f);
            }

            rowsDone.fetch_add(1, std::memory_order_relaxed);
        }
    };

    int workers = parallel_workers(raw.height);
    int rowsPerWorker = (raw.height + workers - 1) / workers;

    std::vector<std::future<void>> tasks;

    for (int i = 0; i < workers; ++i) {

        int fromRow = i * rowsPerWorker;
        int toRow = std::min(raw.height, fromRow + rowsPerWorker);
        if (fromRow >= toRow) break;

        tasks.push_back(std::async(std::launch::async, band, fromRow, toRow));
    }

    if (report) {

        while (true) {

            int done = rowsDone.load(std::memory_order_relaxed);

            if (!report(done, raw.height)) stopped.store(true);
            if (stopped.load() || done >= raw.height) break;

            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }

    for (auto &task : tasks) task.wait();

    if (stopped.load()) return Image();

    if (report) report(raw.height, raw.height);
    return image;
}


inline Image16 demosaic_cpu16(const RawImage &raw_image) {

    int outW = raw_image.width / 2;
    int outH = raw_image.height / 2;

    Image16 image;

    image.width = outW;
    image.height = outH;
    image.channels = 3;
    image.data.resize(static_cast<std::size_t>(outW) * outH * 3);

    float range = static_cast<float>(raw_image.white_level - raw_image.black_level);
    if (range <= 0.f) range = 1.f;

    for (int by = 0; by < outH; ++by) {

        for (int bx = 0; bx < outW; ++bx) {

            int x0 = bx * 2;
            int y0 = by * 2;

            float sumR = 0.f, sumG = 0.f, sumB = 0.f;
            int countG = 0;

            for (int dy = 0; dy < 2; ++dy) {
                for (int dx = 0; dx < 2; ++dx) {

                    int x = dx + x0;
                    int y = dy + y0;

                    uint16_t rawV = raw_image.data[static_cast<std::size_t>(y)
                        * raw_image.width + x];

                    float normalized = (static_cast<float>(rawV - raw_image.black_level) / range);
                    normalized = std::min(1.f, std::max(0.f, normalized));

                    int color = raw_image.color_at(x, y);

                    if (color == 0) sumR = normalized;
                    else if (color == 2) sumB = normalized;
                    else { sumG += normalized; ++countG; }
                }
            }

            float avgG = countG > 0 ? sumG / countG : 0.f;

            float r = std::min(1.f, std::max(0.f, sumR * raw_image.wbR));
            float g = std::min(1.f, std::max(0.f, avgG * raw_image.wbG));
            float b = std::min(1.f, std::max(0.f, sumB * raw_image.wbB));

            std::size_t idx = (static_cast<std::size_t>(by) * outW + bx) * 3;
            image.data[idx + 0] = static_cast<uint16_t>(r * 65535.f + 0.5f);
            image.data[idx + 1] = static_cast<uint16_t>(g * 65535.f + 0.5f);
            image.data[idx + 2] = static_cast<uint16_t>(b * 65535.f + 0.5f);
        }
    }

    return image;
}


inline bool save_rgb_tiff16(const std::string &path, const Image16 &image) {

    if (image.data.empty() || image.channels != 3) return false;

    TIFF *tif = TIFFOpen(path.c_str(), "w");
    if (!tif) return false;

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, image.width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, image.height);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 16);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, image.height);

    for (int y = 0; y < image.height; ++y) {

        const uint16_t *row = image.data.data() + static_cast<std::size_t>(y) * image.width * 3;

        if (TIFFWriteScanline(tif, const_cast<uint16_t*>(row), y) < 0) {
            TIFFClose(tif);
            return false;
        }
    }

    TIFFClose(tif);
    return true;
}


//func for saving raw(dng) to binary without data losses
inline bool save_raw_binary(const std::string &path, const RawImage &raw_image) {

    std::ofstream file(path, std::ios::binary);
    if (!file) return false;

    file.write(reinterpret_cast<const char*>(raw_image.data.data()),
    raw_image.size());

    return file.good();
}

inline bool save_raw_notes(const std::string &path, const RawImage &raw_image,
                           const std::string &cfa, const std::string &data_path) {

    std::ofstream file(path);
    if (!file) return false;

    if (!data_path.empty())
        file << "M := READBIN(\"" << data_path << "\", \"uint16\", 0, "
             << raw_image.width << ", 0, " << raw_image.height << ")\n\n";

    file << "width        " << raw_image.width << "\n"
         << "height       " << raw_image.height << "\n"
         << "type         uint16 little-endian\n"
         << "bytes        " << raw_image.size() << "\n"
         << "black level  " << raw_image.black_level << "\n"
         << "white level  " << raw_image.white_level << "\n"
         << "cfa          " << cfa << "\n"
         << "camera       " << raw_image.camera << "\n";

    return file.good();
}

inline bool save_raw_in_tiff(const std::string &path, const RawImage &raw_image) {

    TIFF *tif = TIFFOpen(path.c_str(), "w");
    if (!tif) return false;

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, raw_image.width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, raw_image.height);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 16);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, raw_image.height);

    for (int i = 0; i < raw_image.height; ++i) {

       const uint16_t *row_ptr = raw_image.data.data() + static_cast<std::size_t>(i)
        * raw_image.width; 

        if (TIFFWriteScanline(tif, const_cast<uint16_t*>(row_ptr), i) < 0) {

            TIFFClose(tif);
            return false;   

        }
    }

    TIFFClose(tif);
    return true;

}

inline Image apply_gamma(const Image &s_im, float gamma) {

    Image im;
    im.width = s_im.width;
    im.height = s_im.height;
    im.channels = s_im.channels;
    im.data.resize(s_im.data.size());
    
    float invGamma = 1.f / gamma;
    
    for (std::size_t i = 0; i < s_im.data.size(); ++i) {

        float v = s_im.data[i] / 255.f;
        v = std::pow(v, invGamma );
        v = std::min(1.f, std::max(0.f, v));
        im.data[i] = static_cast<uint8_t>(v * 255.f + 0.5f);

    }

    return im;
}

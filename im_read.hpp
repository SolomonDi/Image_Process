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

class RawImage{
public:
    int width = 0;
    int height = 0;
    int black_level = 0;
    int white_level = 0;
    unsigned cfaPattern = 0;
    float wbR = 1.f, wbG = 1.f, wbB = 1.f;
    std::vector<uint16_t> data;
    
public:
    // what color at x,y pos
    int color_at(int x, int y) const {

        return (cfaPattern >> ((((y << 1) & 14) | (x & 1)) << 1)) & 3;
    }

    std::size_t size () const {return data.size() * sizeof(uint16_t);}
};

//func for image loading
std::optional<Image> load_image(const std::string& path, int desiredChannels = 0) {

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

std::optional<RawImage> load_raw(const std::string &path) {

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

    image.width = sizes.raw_width;
    image.height = sizes.raw_height;
    image.black_level = color.black;
    image.white_level = color.maximum;
    image.cfaPattern = idata.filters;

    
    const std::size_t pitchPx = sizes.raw_pitch
        ? sizes.raw_pitch / sizeof(uint16_t)
        : static_cast<std::size_t>(sizes.raw_width);

    image.data.resize(static_cast<std::size_t>(image.width) * image.height);
    for (int y = 0; y < image.height; ++y) {
        const uint16_t* src = raw.raw_image + static_cast<std::size_t>(y) * pitchPx;
        std::copy(src, src + image.width,
                  image.data.begin() + static_cast<std::size_t>(y) * image.width);
    }

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

bool save_image(const std::string &path, const Image &image) {

    return stbi_write_png(
        path.c_str(),
        image.width, image.height, image.channels,
        image.data.data(),
        image.width * image.channels // stride: one byte for line
    ) != 0;
}

Image demosaic_cpu(const RawImage &raw_image) {

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

//func for saving raw(dng) to binary without data losses
inline bool save_raw_binary(const std::string &path, const RawImage &raw_image) {

    std::ofstream file(path, std::ios::binary);
    if (!file) return false;

    file.write(reinterpret_cast<const char*>(raw_image.data.data()), 
    raw_image.size());

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

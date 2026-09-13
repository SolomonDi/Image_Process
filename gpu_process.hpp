#pragma once 

#include"im_read.hpp"
#include"im_process.cuh"
#include<cuda_runtime.h>
#include<stdexcept>
#include<string>

inline void cudaCheck(cudaError_t status, const char* what) {

    if (status != cudaSuccess)
        throw std::runtime_error(std::string(what) + ": " + cudaGetErrorString(status));
}

inline bool cuda_device_available() {

    int count = 0;
    return cudaGetDeviceCount(&count) == cudaSuccess && count > 0;
}

inline bool cuda_memory_usage(std::size_t &usedMb, std::size_t &totalMb) {

    std::size_t freeBytes = 0;
    std::size_t totalBytes = 0;

    if (cudaMemGetInfo(&freeBytes, &totalBytes) != cudaSuccess) return false;

    usedMb = (totalBytes - freeBytes) / (1024 * 1024);
    totalMb = totalBytes / (1024 * 1024);

    return true;
}

inline std::string cuda_device_name() {

    int count = 0;
    if (cudaGetDeviceCount(&count) != cudaSuccess || count == 0) return "no CUDA device";

    cudaDeviceProp properties;
    if (cudaGetDeviceProperties(&properties, 0) != cudaSuccess) return "CUDA device";

    return std::string(properties.name);
}

inline Image process_gpu_demosaic(const RawImage& raw) {

    int outW = raw.width / 2;
    int outH = raw.height / 2;

    size_t rawBytes = raw.data.size() * sizeof(uint16_t);
    size_t outBytes = (static_cast<size_t>(outW) * outH) * 3;

    uint16_t* d_in = nullptr;
    uint8_t* d_out = nullptr;

    if ((cudaMalloc(reinterpret_cast<void**>(&d_in), rawBytes)) != cudaSuccess) 
        throw std::runtime_error("cudaMalloc failed for d_in");

    if ((cudaMalloc(reinterpret_cast<void**>(&d_out), outBytes)) != cudaSuccess) {
        cudaFree(d_in);
        throw std::runtime_error("cudaMalloc failed for d_out");
    }

    Image image;
    image.width = outW;
    image.height = outH;
    image.channels = 3;

    try {

    cudaCheck(cudaMemcpy(d_in, raw.data.data(), rawBytes, cudaMemcpyHostToDevice),
              "upload of raw data");

    launchDemosaicBin(d_in, raw.width, raw.height, d_out, outW, outH,
                    raw.black_level, raw.white_level, raw.cfaPattern,
                    raw.wbR, raw.wbG, raw.wbB);

    cudaCheck(cudaGetLastError(), "demosaic kernel launch");
    cudaCheck(cudaDeviceSynchronize(), "demosaic kernel");

    image.data.resize(outBytes);

    cudaCheck(cudaMemcpy(image.data.data(), d_out, outBytes, cudaMemcpyDeviceToHost),
              "download of demosaic result");

    } catch (...) {

        cudaFree(d_in);
        cudaFree(d_out);
        throw;
    }
    
    cudaFree(d_in);
    cudaFree(d_out);

    return image;
}

inline Image process_gpu_demosaic(const RawImage& raw,
                                  const std::function<bool(int, int)>& report) {

    int outW = raw.width / 2;
    int outH = raw.height / 2;

    size_t rawBytes = raw.data.size() * sizeof(uint16_t);
    size_t outBytes = (static_cast<size_t>(outW) * outH) * 3;

    uint16_t* d_in = nullptr;
    uint8_t* d_out = nullptr;

    if ((cudaMalloc(reinterpret_cast<void**>(&d_in), rawBytes)) != cudaSuccess)
        throw std::runtime_error("cudaMalloc failed for d_in");

    if ((cudaMalloc(reinterpret_cast<void**>(&d_out), outBytes)) != cudaSuccess) {
        cudaFree(d_in);
        throw std::runtime_error("cudaMalloc failed for d_out");
    }

    Image image;
    image.width = outW;
    image.height = outH;
    image.channels = 3;

    const int bandRows = 256;
    bool cancelled = false;

    try {

        cudaCheck(cudaMemcpy(d_in, raw.data.data(), rawBytes, cudaMemcpyHostToDevice),
                  "upload of raw data");

        for (int y = 0; y < outH; y += bandRows) {

            int rows = std::min(bandRows, outH - y);

            launchDemosaicBin(d_in + static_cast<size_t>(y) * 2 * raw.width,
                              raw.width, rows * 2,
                              d_out + static_cast<size_t>(y) * outW * 3,
                              outW, rows,
                              raw.black_level, raw.white_level, raw.cfaPattern,
                              raw.wbR, raw.wbG, raw.wbB);

            cudaCheck(cudaGetLastError(), "demosaic kernel launch");
            cudaCheck(cudaDeviceSynchronize(), "demosaic kernel");

            if (report && !report(y + rows, outH)) { cancelled = true; break; }
        }

        if (!cancelled) {

            image.data.resize(outBytes);

            cudaCheck(cudaMemcpy(image.data.data(), d_out, outBytes, cudaMemcpyDeviceToHost),
                      "download of demosaic result");
        }

    } catch (...) {

        cudaFree(d_in);
        cudaFree(d_out);
        throw;
    }

    cudaFree(d_in);
    cudaFree(d_out);

    if (cancelled) return Image();
    return image;
}

inline Image process_gpu_demosaic_full(const RawImage& raw, bool malvar,
                                       const std::function<bool(int, int)>& report) {

    size_t rawBytes = raw.data.size() * sizeof(uint16_t);
    size_t outBytes = (static_cast<size_t>(raw.width) * raw.height) * 3;

    uint16_t* d_in = nullptr;
    uint8_t* d_out = nullptr;

    if ((cudaMalloc(reinterpret_cast<void**>(&d_in), rawBytes)) != cudaSuccess)
        throw std::runtime_error("cudaMalloc failed for d_in");

    if ((cudaMalloc(reinterpret_cast<void**>(&d_out), outBytes)) != cudaSuccess) {
        cudaFree(d_in);
        throw std::runtime_error("cudaMalloc failed for d_out");
    }

    Image image;
    image.width = raw.width;
    image.height = raw.height;
    image.channels = 3;

    const int bandRows = 512;
    bool cancelled = false;

    try {

        cudaCheck(cudaMemcpy(d_in, raw.data.data(), rawBytes, cudaMemcpyHostToDevice),
                  "upload of raw data");

        for (int y = 0; y < raw.height; y += bandRows) {

            int rows = std::min(bandRows, raw.height - y);

            launchDemosaicFull(d_in, raw.width, raw.height, d_out, y, rows,
                               raw.black_level, raw.white_level, raw.cfaPattern,
                               raw.wbR, raw.wbG, raw.wbB, malvar ? 1 : 0);

            cudaCheck(cudaGetLastError(), "demosaic kernel launch");
            cudaCheck(cudaDeviceSynchronize(), "demosaic kernel");

            if (report && !report(y + rows, raw.height)) { cancelled = true; break; }
        }

        if (!cancelled) {

            image.data.resize(outBytes);

            cudaCheck(cudaMemcpy(image.data.data(), d_out, outBytes, cudaMemcpyDeviceToHost),
                      "download of demosaic result");
        }

    } catch (...) {

        cudaFree(d_in);
        cudaFree(d_out);
        throw;
    }

    cudaFree(d_in);
    cudaFree(d_out);

    if (cancelled) return Image();
    return image;
}

inline Image gammaApply_GPU(const Image &src, float gamma) {

    size_t count = src.data.size();
    float invGamma = 1.f / gamma;
    
    uint8_t * d_src = nullptr;
    uint8_t * d_out = nullptr;

    if(cudaMalloc(reinterpret_cast<void**>(&d_src), count) != cudaSuccess) 
        throw std::runtime_error("cudaMalloc failed for d_src (gamma)");
    if (cudaMalloc(reinterpret_cast<void**>(&d_out), count) != cudaSuccess){
        cudaFree(d_src);
        throw std::runtime_error("cudaMalloc failed for out (gamma)");
    }
    
    Image dst;
    dst.width = src.width;
    dst.height = src.height;
    dst.channels = src.channels;

    try {

    cudaCheck(cudaMemcpy(d_src, src.data.data(), count, cudaMemcpyHostToDevice),
              "upload for gamma");

    launchApplyGamma(d_src, d_out, count, invGamma);

    cudaCheck(cudaGetLastError(), "gamma kernel launch");
    cudaCheck(cudaDeviceSynchronize(), "gamma kernel");

    dst.data.resize(count);

    cudaCheck(cudaMemcpy(dst.data.data(), d_out, count, cudaMemcpyDeviceToHost),
              "download of gamma result");

    } catch (...) {

        cudaFree(d_src);
        cudaFree(d_out);
        throw;
    }
    cudaFree(d_src);
    cudaFree(d_out);

    return dst;
}
#pragma once 

#include"im_read.hpp"
#include"im_process.cuh"
#include<cuda_runtime.h>
#include<stdexcept>

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

    cudaMemcpy(d_in, raw.data.data(), rawBytes, cudaMemcpyHostToDevice);

    launchDemosaicBin(d_in, raw.width, raw.height, d_out, outW, outH,
                    raw.black_level, raw.white_level, raw.cfaPattern,
                    raw.wbR, raw.wbG, raw.wbB);

    Image image;
    image.width = outW;
    image.height = outH;
    image.channels = 3;
    image.data.resize(outBytes);
    cudaMemcpy(image.data.data(), d_out, outBytes, cudaMemcpyDeviceToHost);
    
    cudaFree(d_in);
    cudaFree(d_out);

    return image;
}

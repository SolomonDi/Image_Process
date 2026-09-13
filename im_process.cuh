#pragma once

#include <cstdint>
#include <cuda_runtime.h>

void launchDemosaicBin(const uint16_t* d_raw, int rawW, int rawH,
                        uint8_t* d_out, int outW, int outH,
                        int blackLevel, int whiteLevel,
                        unsigned cfaPattern,
                        float wbR, float wbG, float wbB,   
                        cudaStream_t stream = 0);

void launchDemosaicFull(const uint16_t* d_raw, int rawW, int rawH,
                        uint8_t* d_out, int yStart, int rows,
                        int blackLevel, int whiteLevel,
                        unsigned cfaPattern,
                        float wbR, float wbG, float wbB,
                        int malvar,
                        cudaStream_t stream = 0);

void launchApplyGamma(const uint8_t* d_src, uint8_t* d_dst,
                       size_t count, float invGamma,
                       cudaStream_t stream = 0);
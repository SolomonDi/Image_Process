#pragma once

#include <cstdint>
#include <cuda_runtime.h>

void launchDemosaicBin(const uint16_t* d_raw, int rawW, int rawH,
                        uint8_t* d_out, int outW, int outH,
                        int blackLevel, int whiteLevel,
                        unsigned cfaPattern,
                        float wbR, float wbG, float wbB,   
                        cudaStream_t stream = 0);


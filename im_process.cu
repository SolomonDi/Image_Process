#include "im_process.cuh"

__device__ __constant__ float kInvGamma = 1.0f / 2.2f;

__global__ void demosaicBinKernel(
    const uint16_t* d_raw, int rawW, int rawH, uint8_t* d_out,
    int outW, int outH, int blackLevel, float range,
    unsigned cfaPattern, float wbR, float wbG, float wbB)
{
    int bx = blockIdx.x * blockDim.x + threadIdx.x;
    int by = blockIdx.y * blockDim.y + threadIdx.y;
    if (bx >= outW || by >= outH) return;

    int x0 = bx * 2;
    int y0 = by * 2;

    float sumR = 0.f, sumG = 0.f, sumB = 0.f;
    int countG = 0;

    for (int dy = 0; dy < 2; ++dy) {
        for (int dx = 0; dx < 2; ++dx) {
            int x = x0 + dx;
            int y = y0 + dy;

            uint16_t raw_value = d_raw[(size_t)y * rawW + x];
            float normalized = ((float)raw_value - blackLevel) / range;
            normalized = fminf(1.0f, fmaxf(0.0f, normalized));

            int color = (cfaPattern >> ((((y << 1) & 14) | (x & 1)) << 1)) & 3;
            if (color == 0) sumR = normalized;
            else if (color == 2) sumB = normalized;
            else { sumG += normalized; ++countG; }
        }
    }

    float avgG = countG > 0 ? sumG / countG : 0.f;

    float r = sumR * wbR;
    float g = avgG * wbG;
    float b = sumB * wbB;

    r = fminf(1.0f, fmaxf(0.0f, r));
    g = fminf(1.0f, fmaxf(0.0f, g));
    b = fminf(1.0f, fmaxf(0.0f, b));

    r = powf(r, kInvGamma);
    g = powf(g, kInvGamma);
    b = powf(b, kInvGamma);

    size_t idx = ((size_t)by * outW + bx) * 3;

    d_out[idx + 0] = (uint8_t)(r * 255.0f + 0.5f);
    d_out[idx + 1] = (uint8_t)(g * 255.0f + 0.5f);
    d_out[idx + 2] = (uint8_t)(b * 255.0f + 0.5f);
}

void launchDemosaicBin(const uint16_t* d_raw,
    int rawW, int rawH, uint8_t* d_out, int outW, int outH,
    int blackLevel, int whiteLevel,
    unsigned cfaPattern, float wbR, float wbG, float wbB,
    cudaStream_t stream)
{
    float range = (float)(whiteLevel - blackLevel);
    if (range <= 0.f) range = 1.f;

    dim3 block(16, 16);
    dim3 grid((outW + block.x - 1) / block.x, (outH + block.y - 1) / block.y);
    demosaicBinKernel<<<grid, block, 0, stream>>>(
        d_raw, rawW, rawH, d_out, outW, outH, blackLevel, range, cfaPattern, wbR, wbG, wbB);
}
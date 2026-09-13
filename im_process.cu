#include "im_process.cuh"

//__device__ __constant__ float kInvGamma = 1.0f / 2.2f;

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

    //r = powf(r, kInvGamma);
    //g = powf(g, kInvGamma);
    //b = powf(b, kInvGamma);

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



__device__ __forceinline__ float sampleRaw(const uint16_t* d_raw, int rawW, int rawH,
                                           int x, int y, float black, float range)
{
    x = min(max(x, 0), rawW - 1);
    y = min(max(y, 0), rawH - 1);

    return ((float)d_raw[(size_t)y * rawW + x] - black) / range;
}

__global__ void demosaicFullKernel(
    const uint16_t* d_raw, int rawW, int rawH, uint8_t* d_out,
    int yStart, int rows, int blackLevel, float range,
    unsigned cfaPattern, float wbR, float wbG, float wbB, int malvar)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int localY = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= rawW || localY >= rows) return;

    int y = yStart + localY;
    if (y >= rawH) return;

    float black = (float)blackLevel;

#define RAWN(dx, dy) sampleRaw(d_raw, rawW, rawH, x + (dx), y + (dy), black, range)

    float v = RAWN(0, 0);
    float r, g, b;

    int color = (cfaPattern >> ((((y << 1) & 14) | (x & 1)) << 1)) & 3;

    if (color == 0 || color == 2) {

        float orthogonal = RAWN(-1, 0) + RAWN(1, 0) + RAWN(0, -1) + RAWN(0, 1);
        float diagonal = RAWN(-1, -1) + RAWN(1, -1) + RAWN(-1, 1) + RAWN(1, 1);
        float distant = RAWN(-2, 0) + RAWN(2, 0) + RAWN(0, -2) + RAWN(0, 2);

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

        float horizontal = RAWN(-1, 0) + RAWN(1, 0);
        float vertical = RAWN(0, -1) + RAWN(0, 1);

        float alongRow, alongColumn;

        if (malvar) {

            float diagonal = RAWN(-1, -1) + RAWN(1, -1) + RAWN(-1, 1) + RAWN(1, 1);
            float distantH = RAWN(-2, 0) + RAWN(2, 0);
            float distantV = RAWN(0, -2) + RAWN(0, 2);

            alongRow = (5.f * v + 4.f * horizontal - diagonal - distantH + 0.5f * distantV) * 0.125f;
            alongColumn = (5.f * v + 4.f * vertical - diagonal - distantV + 0.5f * distantH) * 0.125f;

        } else {

            alongRow = horizontal * 0.5f;
            alongColumn = vertical * 0.5f;
        }

        int leftColor = (cfaPattern >> ((((y << 1) & 14) | ((x - 1) & 1)) << 1)) & 3;

        if (leftColor == 0) { r = alongRow; b = alongColumn; }
        else { b = alongRow; r = alongColumn; }
    }

#undef RAWN

    r = fminf(1.0f, fmaxf(0.0f, r * wbR));
    g = fminf(1.0f, fmaxf(0.0f, g * wbG));
    b = fminf(1.0f, fmaxf(0.0f, b * wbB));

    size_t idx = ((size_t)y * rawW + x) * 3;

    d_out[idx + 0] = (uint8_t)(r * 255.0f + 0.5f);
    d_out[idx + 1] = (uint8_t)(g * 255.0f + 0.5f);
    d_out[idx + 2] = (uint8_t)(b * 255.0f + 0.5f);
}

void launchDemosaicFull(const uint16_t* d_raw, int rawW, int rawH,
                        uint8_t* d_out, int yStart, int rows,
                        int blackLevel, int whiteLevel,
                        unsigned cfaPattern,
                        float wbR, float wbG, float wbB,
                        int malvar,
                        cudaStream_t stream)
{
    float range = (float)(whiteLevel - blackLevel);
    if (range <= 0.f) range = 1.f;

    dim3 block(16, 16);
    dim3 grid((rawW + block.x - 1) / block.x, (rows + block.y - 1) / block.y);

    demosaicFullKernel<<<grid, block, 0, stream>>>(
        d_raw, rawW, rawH, d_out, yStart, rows, blackLevel, range,
        cfaPattern, wbR, wbG, wbB, malvar);
}


__global__ void applyGammaKernel(const uint8_t *src, uint8_t *dst, size_t count, float Gamma) {

    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;

    if (idx >= count) return;

    float v = src[idx] / 255.f;
    v = powf(v, Gamma);
    v = fminf(1.f, fmaxf(0.f, v));
    
    dst[idx] = static_cast<uint8_t>(v * 255.f + 0.5f);
}


void launchApplyGamma(const uint8_t* d_src, uint8_t* d_dst, size_t count, float Gamma, 
cudaStream_t stream) 
{
    int threads = 256;
    int blocks = static_cast<int>((count + threads - 1) / threads);
    applyGammaKernel<<<blocks, threads, 0, stream>>>(d_src, d_dst, count, Gamma);
}
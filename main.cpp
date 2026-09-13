#include <iostream>
#include "im_read.hpp"
#include "gpu_process.hpp"
#include "image_stats.hpp"


int main(int argc, char** argv) {

    const std::string path = (argc > 1) ? argv[1] : "test.dng";

    auto raw = load_raw(path);
    if (!raw) {
        std::cerr << "Can't load \"" << path << "\"\n";
        return 1;
    }

    std::cout << "RAW: " << raw->width << "x" << raw->height
              << " black=" << raw->black_level
              << " white=" << raw->white_level
              << " filters=0x" << std::hex << raw->cfaPattern << std::dec << "\n";

    if (save_raw_binary("raw_data.bin", *raw)) {
        std::cout << "OK: raw_data.bin saved ("
                  << (size_t)raw->width * raw->height * 2 << " bytes expected)\n";

        if (save_raw_notes("raw_data_mathcad.txt", *raw, cfa_pattern_name(*raw), "raw_data.bin"))
            std::cout << "OK: raw_data_mathcad.txt saved (sizes, levels and the Mathcad line)\n";
        else
            std::cerr << "FAIL: failed to save raw_data_mathcad.txt\n";

    } else {
        std::cerr << "FAIL: failed to save raw_data.bin\n";
    }

    if (save_raw_in_tiff("raw_mosaic.tiff", *raw)) {
        std::cout << "OK: raw_mosaic.tiff saved\n";
    } else {
        std::cerr << "FAIL: failed to save raw_mosaic.tiff\n";
    }

    Image resultCPU = demosaic_cpu(*raw);
    save_image("output_cpu.png", resultCPU);

    Image resultGPU = process_gpu_demosaic(*raw);
    save_image("output_gpu.png", resultGPU);

    return 0;
}
#include<iostream>
#include"im_read.hpp"
#include"gpu_process.hpp"

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

    Image resultCPU = demosaic_cpu(*raw);
    save_image("output_cpu.png", resultCPU);

    Image resultGPU = process_gpu_demosaic(*raw);
    save_image("output_gpu.png", resultGPU);

    return 0;
}

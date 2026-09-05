#pragma once 

#include "im_read.hpp"
#include "gpu_process.hpp"

enum class Backend { CPU, GPU };

Image process_demosaic(const RawImage &raw, Backend backend) {

    if (backend == Backend::CPU) {
        return demosaic_cpu(raw);
    }  else if(backend == Backend::GPU) {
        return process_gpu_demosaic(raw);
    } else {
        throw std::invalid_argument("Invalid choice");
    }

}
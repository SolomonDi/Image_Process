#pragma once 

#include "im_read.hpp"
#include "gpu_process.hpp"

enum class Backend { CPU, GPU };

inline Image process_demosaic(const RawImage &raw, Backend backend) {

    if (backend == Backend::CPU) {
        return demosaic_cpu(raw);
    }  else if(backend == Backend::GPU) {
        return process_gpu_demosaic(raw);
    } else {
        throw std::invalid_argument("Invalid choice");
    }

}

inline Image apply_gamma(const Image& src, float gamma, Backend backend) {

    if (backend == Backend::GPU) return gammaApply_GPU(src, gamma);
    else if (backend == Backend::CPU) return apply_gamma(src, gamma);
    else {throw std::invalid_argument("Invalid device");}

}
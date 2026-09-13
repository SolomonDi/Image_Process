#pragma once 

#include "im_read.hpp"
#include "gpu_process.hpp"

enum class Backend { CPU, GPU };

enum class DemosaicMethod { Binning, Bilinear, Malvar };

inline const char* demosaic_method_name(DemosaicMethod method) {

    if (method == DemosaicMethod::Bilinear) return "bilinear";
    if (method == DemosaicMethod::Malvar) return "Malvar-He-Cutler";
    return "binning 2x2";
}

inline Image process_demosaic(const RawImage &raw, Backend backend) {

    if (backend == Backend::CPU) {
        return demosaic_cpu(raw);
    }  else if(backend == Backend::GPU) {
        return process_gpu_demosaic(raw);
    } else {
        throw std::invalid_argument("Invalid choice");
    }

}

inline Image process_demosaic(const RawImage &raw, Backend backend,
                              const std::function<bool(int, int)> &report) {

    if (backend == Backend::CPU) return demosaic_cpu_progress(raw, report);

    if (backend == Backend::GPU) return process_gpu_demosaic(raw, report);

    throw std::invalid_argument("Invalid choice");
}

inline Image process_demosaic(const RawImage &raw, Backend backend, DemosaicMethod method,
                              const std::function<bool(int, int)> &report) {

    if (method == DemosaicMethod::Binning) return process_demosaic(raw, backend, report);

    bool malvar = (method == DemosaicMethod::Malvar);

    if (backend == Backend::GPU) return process_gpu_demosaic_full(raw, malvar, report);

    return demosaic_full_progress(raw, malvar, report);
}

inline Image apply_gamma(const Image& src, float gamma, Backend backend) {

    if (backend == Backend::GPU) return gammaApply_GPU(src, gamma);
    else if (backend == Backend::CPU) return apply_gamma(src, gamma);
    else {throw std::invalid_argument("Invalid device");}

}
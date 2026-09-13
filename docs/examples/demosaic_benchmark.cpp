#include "process.hpp"

#include <chrono>
#include <cstdio>

int main(int argc, char** argv) {

    if (argc < 2) {
        std::printf("usage: demosaic_benchmark <file.dng>\n");
        return 1;
    }

    std::optional<RawImage> raw = load_raw(argv[1]);
    if (!raw) return 1;

    bool gpu = cuda_device_available();

    std::printf("%dx%d  device: %s\n", raw->width, raw->height, cuda_device_name().c_str());

    if (gpu) process_demosaic(*raw, Backend::GPU, DemosaicMethod::Binning, {});

    const DemosaicMethod methods[] = { DemosaicMethod::Binning, DemosaicMethod::Bilinear, DemosaicMethod::Malvar };
    const Backend backends[] = { Backend::CPU, Backend::GPU };

    for (Backend backend : backends) {

        if (backend == Backend::GPU && !gpu) continue;

        for (DemosaicMethod method : methods) {

            auto start = std::chrono::steady_clock::now();

            Image image = process_demosaic(*raw, backend, method, {});
            Image shown = apply_gamma(image, 2.2f, backend);

            auto end = std::chrono::steady_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start).count();

            std::printf("%s  %-17s  %5dx%-5d  %8.1f ms\n", backend == Backend::GPU ? "GPU" : "CPU",
                        demosaic_method_name(method), shown.width, shown.height, ms);
        }
    }

    Backend best = gpu ? Backend::GPU : Backend::CPU;
    Image result = process_demosaic(*raw, best, DemosaicMethod::Malvar, {});

    save_image("malvar.png", apply_gamma(result, 2.2f, best));
    std::printf("saved malvar.png\n");

    return 0;
}

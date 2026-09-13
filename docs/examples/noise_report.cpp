#include "im_read.hpp"
#include "image_stats.hpp"
#include "denoise.hpp"

#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {

    if (argc < 2) {
        std::printf("usage: noise_report <file.dng> [x y w h]\n");
        return 1;
    }

    std::optional<RawImage> raw = load_raw(argv[1]);
    if (!raw) return 1;

    int x = argc > 5 ? std::atoi(argv[2]) : raw->width / 2 - 128;
    int y = argc > 5 ? std::atoi(argv[3]) : raw->height / 2 - 128;
    int w = argc > 5 ? std::atoi(argv[4]) : 256;
    int h = argc > 5 ? std::atoi(argv[5]) : 256;

    std::printf("%dx%d  CFA %s  black %d  white %d  region [%d, %d] %dx%d\n",
                raw->width, raw->height, cfa_pattern_name(*raw).c_str(),
                raw->black_level, raw->white_level, x, y, w, h);

    ChannelStats before = compute_region_channels(*raw, x, y, w, h);

    DenoiseParams params;
    params.method = DenoiseMethod::Gaussian;
    params.sigma = 1.0;

    RawImage filtered = *raw;

    denoise_frame(filtered, x, y, w, h, params, {});

    ChannelStats after = compute_region_channels(filtered, x, y, w, h);

    for (int k = 0; k < 4; ++k) {

        double change = (after.stats[k].stdDev - before.stats[k].stdDev) / before.stats[k].stdDev * 100.0;

        std::printf("%-2s  mean %7.1f  sigma %6.2f -> %6.2f  (%+.1f%%)\n",
                    before.labels[k].c_str(), before.stats[k].mean,
                    before.stats[k].stdDev, after.stats[k].stdDev, change);
    }

    Image color = demosaic_full_progress(filtered, true, {});

    save_image("malvar_gamma.png", apply_gamma(color, 2.2f));
    save_raw_in_tiff("filtered_mosaic.tiff", filtered);

    std::printf("saved malvar_gamma.png and filtered_mosaic.tiff\n");
    return 0;
}

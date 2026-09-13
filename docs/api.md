# Справочник функций

Вся обработка написана как библиотека из заголовочных файлов: функции объявлены `inline` прямо в `.hpp`. Чтобы использовать их в своей программе, подключи нужный заголовок и собери программу вместе с LibRaw и libtiff, а для функций GPU — ещё и с CUDA.

| Заголовок | Что внутри |
|---|---|
| `im_read.hpp` | типы `Image`, `Image16`, `RawImage`; загрузка RAW; демозаик на CPU; сохранение файлов; гамма |
| `image_stats.hpp` | статистика, гистограммы, профиль, двухкадровый шум, темновой кадр |
| `denoise.hpp` | фильтры шумоподавления по CFA-каналам |
| `process.hpp` | выбор устройства и метода демозаика (подключает `gpu_process.hpp`) |
| `gpu_process.hpp` | CUDA: память, запуск ядер, информация об устройстве |
| `im_process.cuh` | объявления функций, запускающих CUDA-ядра из `im_process.cu` |

> [!IMPORTANT]
> `im_read.hpp` содержит реализацию библиотеки stb_image. Подключай его (напрямую или через другие заголовки) только в **один** `.cpp` каждой программы, иначе линковщик выдаст ошибку LNK2005.

## Как собрать свою программу

### Только CPU

В консоли **x64 Native Tools Command Prompt for VS 2022**:

```bat
cl /O2 /MD /EHsc /std:c++17 /utf-8 /DNOMINMAX ^
   /I"C:\Users\RED.DOT\Desktop\PNG NOISE" ^
   /IC:\vcpkg\installed\x64-windows\include /IC:\vcpkg\installed\x64-windows\include\libraw ^
   my_tool.cpp ^
   /link C:\vcpkg\installed\x64-windows\lib\raw_r.lib C:\vcpkg\installed\x64-windows\lib\tiff.lib ^
   C:\vcpkg\installed\x64-windows\lib\lcms2.lib C:\vcpkg\installed\x64-windows\lib\z.lib ws2_32.lib
```

Перед запуском добавь в `PATH` папку с DLL: `set PATH=C:\vcpkg\installed\x64-windows\bin;%PATH%`.

### С GPU

Проще всего добавить цель в `CMakeLists.txt` рядом с существующими:

```cmake
add_executable(my_tool my_tool.cpp im_process.cu)
target_include_directories(my_tool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(my_tool PRIVATE CUDA::cudart libraw::raw_r TIFF::TIFF)
```

## Готовые примеры

Оба примера лежат в `docs/examples/`, собраны и проверены. Вывод ниже — настоящий.

### noise_report.cpp

Считает шум области по каналам, применяет Gaussian σ 1.0, сравнивает σ до и после и сохраняет цветную картинку Malvar.

```cpp
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
```

```text
> noise_report.exe test.dng 3712 1152 256 256
4032x3024  CFA RGGB  black 528  white 4095  region [3712, 1152] 256x256
R   mean   672.3  sigma  15.17 ->  12.26  (-19.2%)
G1  mean   738.1  sigma  19.96 ->  17.18  (-13.9%)
G2  mean   737.7  sigma  20.35 ->  17.79  (-12.5%)
B   mean   595.6  sigma   8.49 ->   6.00  (-29.4%)
saved malvar_gamma.png and filtered_mosaic.tiff
```

### demosaic_benchmark.cpp

Замеряет три метода демозаика на CPU и GPU и сохраняет результат Malvar.

```cpp
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
```

```text
> demosaic_benchmark.exe test_2.DNG
7328x5501  device: NVIDIA GeForce RTX 3080 Ti
CPU  binning 2x2         3664x2750      254.2 ms
CPU  bilinear            7328x5501     1012.8 ms
CPU  Malvar-He-Cutler    7328x5501     1038.5 ms
GPU  binning 2x2         3664x2750       21.2 ms
GPU  bilinear            7328x5501       56.4 ms
GPU  Malvar-He-Cutler    7328x5501       60.3 ms
saved malvar.png
```

Первый вызов на GPU перед циклом нужен для «прогрева»: при первом обращении CUDA создаёт контекст, и это заняло бы лишние сотни миллисекунд в замере.

## Типы данных

### RawImage

Мозаика сенсора и всё, что про неё известно. Объявлен в `im_read.hpp`.

| Поле | Тип | Что хранит |
|---|---|---|
| `width`, `height` | `int` | размер активной области |
| `black_level`, `white_level` | `int` | уровень чёрного и насыщения, ADU |
| `cfaPattern` | `unsigned` | узор Байера в формате LibRaw `filters` |
| `wbR`, `wbG`, `wbB` | `float` | баланс белого камеры, делённый на зелёный (`wbG = 1`) |
| `data` | `std::vector<uint16_t>` | пиксели построчно, `width × height` значений |
| `flip` | `int` | поворот, записанный камерой |
| `activeX`, `activeY` | `int` | смещение активной области в полном кадре сенсора |
| `fullWidth`, `fullHeight` | `int` | полный размер сенсора |
| `obData`, `obWidth`, `obHeight` | `std::vector<uint16_t>`, `int` | зона optical black, если найдена |
| `camera`, `lens` | `std::string` | камера и объектив |
| `iso`, `shutter`, `aperture`, `focal` | `float` | ISO, выдержка в секундах, диафрагма, фокусное в мм |

| Метод | Что возвращает |
|---|---|
| `int color_at(int x, int y) const` | цвет фильтра пикселя: `0` R, `1` G, `2` B, `3` второй G |
| `std::size_t size() const` | размер `data` в байтах: `width × height × 2` |

```cpp
std::optional<RawImage> raw = load_raw("test_2.DNG");
int color = raw->color_at(0, 0);
uint16_t value = raw->data[static_cast<std::size_t>(100) * raw->width + 200];
```

### Image и Image16

Цветная картинка: `width`, `height`, `channels` и `data` построчно (`uint8_t` у `Image`, `uint16_t` у `Image16`). Каналы идут подряд: R, G, B, R, G, B…

| Метод `Image` | Что делает |
|---|---|
| `std::size_t byte_size() const` | размер `data` в байтах |
| `bool data_is_empty() const` | `true`, если картинка пустая — например, после отмены |
| `uint8_t& at(int x, int y, int z)` | ссылка на канал `z` пикселя `(x, y)` с проверкой границ |

```cpp
Image image = demosaic_cpu(*raw);
uint8_t red = image.at(10, 20, 0);
image.at(10, 20, 1) = 255;
```

### ImageStats и ChannelStats

`ImageStats` — `min`, `max` (`uint16_t`), `mean`, `stdDev`, `rangeBits`, `usefulBits`, `snrDb` (`double`).

| Поле | Формула |
|---|---|
| `rangeBits` | `log2(max − min + 1)` — разброс значений в битах |
| `usefulBits` | `log2((white_level − black_level) / stdDev)` — полезные биты, сколько уровней различимо над шумом |
| `snrDb` | `20 · log10((mean − black_level) / stdDev)` |

Эти три поля заполняет `finish_bits(ImageStats&, const RawImage&)`; её вызывают `compute_stats`, `compute_region` и `compute_region_channels` после того, как посчитана σ.

`ChannelStats` — `stats[4]` (`ImageStats` для каждой позиции CFA), `counts[4]` (число пикселей), `labels` (`"R"`, `"G1"`, `"G2"`, `"B"` в порядке позиций).

### NoiseReport

Результат двухкадрового метода: массивы из 4 значений по каналам — `diffMean` (среднее разности кадров), `temporal` (временной шум), `total` (полный шум первого кадра), `fpn` (фиксированный узор); `labels`; `valid` — `false`, если кадры разного размера.

### DenoiseParams

| Поле | По умолчанию | Смысл |
|---|---|---|
| `method` | `DenoiseMethod::Gaussian` | `Gaussian`, `Median` или `Bilateral` |
| `sigma` | `1.0` | Spatial σ в пикселях плоскости канала (Gaussian, Bilateral) |
| `medianSize` | `3` | окно медианы: 3 или 5 (поддерживается до 7) |
| `rangeSigma` | `40.0` | Range σ в ADU (Bilateral) |

### CfaPlane и PlaneIndex

`CfaPlane` — одна плоскость канала: `width`, `height`, `std::vector<float> data`.

`PlaneIndex(const CfaPlane& plane, int reach)` заранее считает таблицы индексов с запасом `reach` по краям. После этого `at(x, y)` возвращает индекс в `data`, даже если `x` или `y` вышли за край не дальше `reach`: подставляется ближайший крайний пиксель.

### Перечисления

| Тип | Значения | Где объявлен |
|---|---|---|
| `Backend` | `CPU`, `GPU` | `process.hpp` |
| `DemosaicMethod` | `Binning`, `Bilinear`, `Malvar` | `process.hpp` |
| `DenoiseMethod` | `Gaussian`, `Median`, `Bilateral` | `denoise.hpp` |

## im_read.hpp — загрузка, демозаик, сохранение

### load_raw

```cpp
std::optional<RawImage> load_raw(const std::string& path);
```

Открывает RAW через LibRaw и возвращает мозаику активной области сенсора.

- Кадр обрезается до активной области; отступ округляется до чётного, чтобы не сбить узор Байера.
- Заполняются black и white level, узор `cfaPattern`, баланс белого, EXIF и поворот `flip`.
- Если по краю сенсора есть закрытая от света зона, она копируется в `obData`.
- Возвращает `std::nullopt` и пишет причину в `std::cerr`, если файл не открылся, сенсор X-Trans или DNG уже без мозаики.

```cpp
std::optional<RawImage> raw = load_raw("test.dng");
if (!raw) return 1;
std::printf("%dx%d black %d white %d filters 0x%x\n", raw->width, raw->height,
            raw->black_level, raw->white_level, raw->cfaPattern);
```

### load_image

```cpp
std::optional<Image> load_image(const std::string& path, int desiredChannels = 0);
```

Загружает обычную картинку (PNG, JPEG, BMP и другие) через stb_image. `desiredChannels = 3` принудительно даёт RGB, `0` — столько каналов, сколько в файле.

```cpp
std::optional<Image> picture = load_image("output.png", 3);
if (picture) std::printf("%dx%d, %d channels\n", picture->width, picture->height, picture->channels);
```

### save_image

```cpp
bool save_image(const std::string& path, const Image& image);
```

Сохраняет `Image` в PNG. Возвращает `false`, если записать не удалось.

### demosaic_cpu

```cpp
Image demosaic_cpu(const RawImage& raw);
```

Binning 2×2 в одном потоке: результат вдвое меньше, 8 бит, без гаммы.

```cpp
Image small = demosaic_cpu(*raw);
save_image("binning.png", apply_gamma(small, 2.2f));
```

### demosaic_cpu_progress

```cpp
Image demosaic_cpu_progress(const RawImage& raw, const std::function<bool(int, int)>& report);
```

Тот же Binning 2×2, но во всех потоках. `report(готово, всего)` вызывается раз в 2 мс с числом готовых строк результата; если он вернёт `false`, работа останавливается и возвращается пустой `Image`. Вместо `report` можно передать `{}`.

```cpp
Image image = demosaic_cpu_progress(*raw, [](int done, int total) {
    std::printf("\r%d / %d rows", done, total);
    return true;
});
```

### demosaic_full_progress

```cpp
Image demosaic_full_progress(const RawImage& raw, bool malvar, const std::function<bool(int, int)>& report);
```

Демозаик полного размера во всех потоках: `malvar = false` — Bilinear, `true` — Malvar-He-Cutler. Прогресс и отмена — как у `demosaic_cpu_progress`.

```cpp
Image full = demosaic_full_progress(*raw, true, {});
if (!full.data_is_empty()) save_image("malvar.png", apply_gamma(full, 2.2f));
```

### demosaic_cpu16

```cpp
Image16 demosaic_cpu16(const RawImage& raw);
```

Binning 2×2 с 16-битным результатом (0…65535), без гаммы. Используется командой **Save linear to 16-bit TIFF**.

### save_rgb_tiff16

```cpp
bool save_rgb_tiff16(const std::string& path, const Image16& image);
```

RGB TIFF, 16 бит на канал. `image.channels` должен быть равен 3.

```cpp
save_rgb_tiff16("linear16.tiff", demosaic_cpu16(*raw));
```

### save_raw_binary

```cpp
bool save_raw_binary(const std::string& path, const RawImage& raw);
```

Записывает `raw.data` как есть: `uint16`, little-endian, без заголовка. Размер файла — `raw.size()` байт.

### save_raw_notes

```cpp
bool save_raw_notes(const std::string& path, const RawImage& raw,
                    const std::string& cfa, const std::string& data_path);
```

Кладёт рядом с сохранённым файлом обычный текстовый блокнот: ширину, высоту, тип данных, размер в байтах, уровни black и white, узор CFA и название камеры. Без этих чисел двоичный файл прочитать правильно нельзя.

Если `data_path` не пустой, первой строкой пишется готовая команда `READBIN(...)` для Mathcad с этим путём; для TIFF её передают пустой, потому что размеры уже лежат в заголовке самого TIFF.

```cpp
save_raw_binary("region.bin", part);
save_raw_notes("region_mathcad.txt", part, cfa_pattern_name(part), "region.bin");
```

### save_raw_in_tiff

```cpp
bool save_raw_in_tiff(const std::string& path, const RawImage& raw);
```

Сохраняет мозаику как серый 16-битный TIFF.

### apply_gamma

```cpp
Image apply_gamma(const Image& image, float gamma);
```

`out = 255 · (in / 255)^(1/gamma)` для каждого байта в одном потоке. Версия с выбором CPU или GPU — в `process.hpp`.

### parallel_workers

```cpp
int parallel_workers(int rows, int minimumRowsPerWorker = 64);
```

Сколько потоков запускать: не больше числа логических ядер и не больше одного потока на `minimumRowsPerWorker` строк, но не меньше одного. На 12-поточном процессоре для 3024 строк получится 12, для 200 строк — 3.

### raw_at

```cpp
uint16_t raw_at(const RawImage& raw, int x, int y);
```

Значение пикселя. Координаты за краем кадра прижимаются к ближайшему краю — удобно для окон фильтров.

## image_stats.hpp — статистика и шум

### compute_region_channels

```cpp
ChannelStats compute_region_channels(const RawImage& raw, int x0, int y0, int width, int height);
```

Статистика области отдельно для каждого канала, в несколько потоков. Область автоматически обрезается по кадру.

```cpp
ChannelStats c = compute_region_channels(*raw, 3712, 1152, 256, 256);
for (int k = 0; k < 4; ++k)
    std::printf("%s mean %.1f sigma %.2f (%zu px)\n", c.labels[k].c_str(),
                c.stats[k].mean, c.stats[k].stdDev, c.counts[k]);
```

### compute_region

```cpp
ImageStats compute_region(const RawImage& raw, int x0, int y0, int width, int height);
```

Статистика области по всем пикселям вместе, в несколько потоков. Область должна лежать внутри кадра: эта функция её не обрезает. Если не уверен, сначала вызови `clamp_region`.

```cpp
int x = 3712, y = 1152, w = 256, h = 256;
clamp_region(*raw, x, y, w, h);
ImageStats s = compute_region(*raw, x, y, w, h);
std::printf("mean %.1f sigma %.2f\n", s.mean, s.stdDev);
```

### compute_stats

```cpp
ImageStats compute_stats(const RawImage& raw);
```

Статистика всего кадра по всем пикселям, в одном потоке.

### clamp_region

```cpp
void clamp_region(const RawImage& raw, int& x0, int& y0, int& width, int& height);
```

Сдвигает и обрезает область так, чтобы она целиком лежала в кадре; минимальный размер 1×1.

### region_crop

```cpp
RawImage region_crop(const RawImage& raw, int x0, int y0, int width, int height);
```

Вырезает область в отдельный `RawImage`. Начало выравнивается на чётные координаты, размер округляется до чётного, чтобы узор Байера остался прежним. Копируются уровни, узор и баланс белого. Так работает **Demosaic selected region**.

```cpp
RawImage part = region_crop(*raw, 4200, 3700, 320, 200);
Image preview = demosaic_full_progress(part, true, {});
```

### cfa_index, cfa_labels, cfa_pattern_name

```cpp
int cfa_index(int x, int y);
std::array<std::string, 4> cfa_labels(const RawImage& raw);
std::string cfa_pattern_name(const RawImage& raw);
```

- `cfa_index` — позиция пикселя в блоке 2×2: `((y & 1) << 1) | (x & 1)`, то есть 0…3.
- `cfa_labels` — имя канала для каждой позиции: для RGGB `{"R", "G1", "G2", "B"}`, для BGGR `{"B", "G1", "G2", "R"}`.
- `cfa_pattern_name` — узор строкой: `"RGGB"`, `"BGGR"`, `"GRBG"` или `"GBRG"`.

```cpp
std::array<std::string, 4> labels = cfa_labels(*raw);
std::printf("pixel (5, 8) is %s in %s\n", labels[cfa_index(5, 8)].c_str(), cfa_pattern_name(*raw).c_str());
```

### compute_histogram

```cpp
std::array<std::vector<uint32_t>, 4> compute_histogram(const RawImage& raw, int x0, int y0,
                                                        int width, int height, int bins);
```

Гистограмма значений по каналам: `bins` столбцов на диапазон 0…white level. Номер столбца — `v / white_level · (bins − 1)`.

```cpp
auto hist = compute_histogram(*raw, 0, 0, raw->width, raw->height, 256);
auto labels = cfa_labels(*raw);
for (int k = 0; k < 4; ++k) {
    auto peak = std::max_element(hist[k].begin(), hist[k].end()) - hist[k].begin();
    std::printf("%s: most pixels in bin %d\n", labels[k].c_str(), static_cast<int>(peak));
}
```

### compute_noise_histogram

```cpp
std::array<std::vector<uint32_t>, 4> compute_noise_histogram(const RawImage& raw, int x0, int y0,
        int width, int height, int bins, double span, const std::array<double, 4>& mean);
```

Гистограмма отклонений `v − mean[k]` в диапазоне ±`span`; отклонения дальше пропускаются. Если `2·span / (bins − 1)` — целое число, каждый столбец покрывает ровно целое число ADU.

```cpp
ChannelStats c = compute_region_channels(*raw, x, y, 256, 256);
std::array<double, 4> mean = { c.stats[0].mean, c.stats[1].mean, c.stats[2].mean, c.stats[3].mean };
auto noise = compute_noise_histogram(*raw, x, y, 256, 256, 161, 80.0, mean);
```

161 столбец на ±80 ADU — ровно 1 ADU на столбец.

### compute_profile

```cpp
std::vector<double> compute_profile(const RawImage& raw, int x0, int y0, int width, int height, bool alongX);
```

`alongX = true` — среднее по каждому столбцу области (длина результата `width`), `false` — по каждой строке (длина `height`).

```cpp
std::vector<double> row = compute_profile(*raw, 0, 3800, raw->width, 1, true);
```

### compute_two_frame

```cpp
NoiseReport compute_two_frame(const RawImage& first, const RawImage& second,
                              int x0, int y0, int width, int height);
```

Двухкадровый шум по каналам (формулы — в разделе [Шум](noise.md#двухкадровый-метод)). `valid = false`, если кадры пустые или разного размера.

```cpp
std::optional<RawImage> first = load_raw("flat_1.dng");
std::optional<RawImage> second = load_raw("flat_2.dng");
NoiseReport report = compute_two_frame(*first, *second, 1000, 1000, 256, 256);
if (report.valid)
    for (int k = 0; k < 4; ++k)
        std::printf("%s rd=%.2f fpn=%.2f tot=%.2f\n", report.labels[k].c_str(),
                    report.temporal[k], report.fpn[k], report.total[k]);
```

### subtract_dark

```cpp
RawImage subtract_dark(const RawImage& light, const RawImage& dark);
```

Возвращает `light − dark + black_level` с обрезкой в 0…white level. Если размеры кадров разные, возвращает `light` без изменений.

```cpp
std::optional<RawImage> dark = load_raw("dark.dng");
RawImage clean = subtract_dark(*raw, *dark);
```

## denoise.hpp — шумоподавление

### denoise_frame

```cpp
bool denoise_frame(RawImage& raw, int x0, int y0, int width, int height,
                   const DenoiseParams& params, const std::function<bool(int, int)>& report);
```

Главная функция: фильтрует прямоугольник кадра **на месте**, отдельно для каждого канала CFA (схема — в разделе [Шум](noise.md#общая-схема)).

- Возвращает `true`, если фильтр закончил, и `false`, если `report` вернул `false`. После отмены кадр не меняется.
- `report(готово, всего)` вызывается раз в 2 мс; единица работы — одна строка плоскости за один проход.

```cpp
DenoiseParams params;
params.method = DenoiseMethod::Bilateral;
params.sigma = 1.0;
params.rangeSigma = 50.0;

std::atomic<bool> cancel{false};

RawImage frame = *raw;
bool finished = denoise_frame(frame, 0, 0, frame.width, frame.height, params,
    [&](int done, int total) {
        std::printf("\r%d%%", done * 100 / std::max(1, total));
        return !cancel.load();
    });
```

### denoise_gaussian_plane, denoise_median_plane, denoise_bilateral_plane

```cpp
bool denoise_gaussian_plane(const CfaPlane& source, CfaPlane& result, double sigma,
                            std::atomic<int>& progress, int total,
                            std::atomic<bool>& stopped, const std::function<bool(int, int)>& report);

bool denoise_median_plane(const CfaPlane& source, CfaPlane& result, int size,
                          std::atomic<int>& progress, int total,
                          std::atomic<bool>& stopped, const std::function<bool(int, int)>& report);

bool denoise_bilateral_plane(const CfaPlane& source, CfaPlane& result, double sigma, double rangeSigma,
                             std::atomic<int>& progress, int total,
                             std::atomic<bool>& stopped, const std::function<bool(int, int)>& report);
```

Фильтруют одну плоскость канала из `source` в `result`. Обычно их вызывает `denoise_frame`, но можно и напрямую. Параметры `progress`, `total` и `stopped` общие для всех четырёх плоскостей, чтобы прогресс шёл от 0 до 100 % через весь кадр.

```cpp
CfaPlane plane;
plane.width = 64;
plane.height = 64;
plane.data.assign(64 * 64, 100.f);

CfaPlane smooth;
std::atomic<int> progress{0};
std::atomic<bool> stopped{false};

denoise_gaussian_plane(plane, smooth, 1.0, progress, 64 * 2, stopped, {});
```

У Gaussian два прохода (по строкам и по столбцам), поэтому `total` для плоскости 64×64 равен 128.

### denoise_run_rows

```cpp
bool denoise_run_rows(int rows, const std::function<void(int)>& body,
                      std::atomic<int>& progress, int total,
                      std::atomic<bool>& stopped, const std::function<bool(int, int)>& report);
```

Выполняет `body(y)` для каждой строки `0…rows−1` в нескольких потоках (`parallel_workers(rows, 16)`), обновляет `progress` и раз в 2 мс опрашивает `report`. Возвращает `false`, если работу остановили.

### denoise_radius, denoise_passes, denoise_method_name

```cpp
int denoise_radius(const DenoiseParams& params);
int denoise_passes(const DenoiseParams& params);
const char* denoise_method_name(DenoiseMethod method);
```

- `denoise_radius` — радиус окна: `⌈3σ⌉` для Gaussian, `⌈2σ⌉` для Bilateral, половина окна для Median.
- `denoise_passes` — число проходов на плоскость: 2 для Gaussian, 1 для остальных.
- `denoise_method_name` — `"gaussian"`, `"median"` или `"bilateral"`.

## process.hpp — выбор устройства и метода

### process_demosaic

```cpp
Image process_demosaic(const RawImage& raw, Backend backend);
Image process_demosaic(const RawImage& raw, Backend backend, const std::function<bool(int, int)>& report);
Image process_demosaic(const RawImage& raw, Backend backend, DemosaicMethod method,
                       const std::function<bool(int, int)>& report);
```

Одна точка входа для демозаика. Первые две версии всегда делают Binning 2×2, третья вызывает нужную реализацию:

| Метод | CPU | GPU |
|---|---|---|
| `Binning` | `demosaic_cpu_progress` | `process_gpu_demosaic` |
| `Bilinear` | `demosaic_full_progress(raw, false, ...)` | `process_gpu_demosaic_full(raw, false, ...)` |
| `Malvar` | `demosaic_full_progress(raw, true, ...)` | `process_gpu_demosaic_full(raw, true, ...)` |

Функции GPU бросают `std::runtime_error` при ошибке CUDA, поэтому вызов лучше обернуть:

```cpp
Backend backend = cuda_device_available() ? Backend::GPU : Backend::CPU;

try {
    Image image = process_demosaic(*raw, backend, DemosaicMethod::Malvar, {});
    save_image("result.png", apply_gamma(image, 2.2f, backend));
} catch (const std::exception& error) {
    std::printf("demosaic failed: %s\n", error.what());
}
```

### apply_gamma (с выбором устройства)

```cpp
Image apply_gamma(const Image& image, float gamma, Backend backend);
```

Гамма на CPU (`apply_gamma(image, gamma)`) или на GPU (`gammaApply_GPU`).

### demosaic_method_name

```cpp
const char* demosaic_method_name(DemosaicMethod method);
```

`"binning 2x2"`, `"bilinear"` или `"Malvar-He-Cutler"`.

## gpu_process.hpp — CUDA

### cuda_device_available, cuda_device_name, cuda_memory_usage

```cpp
bool cuda_device_available();
std::string cuda_device_name();
bool cuda_memory_usage(std::size_t& usedMb, std::size_t& totalMb);
```

- `cuda_device_available` — есть ли хотя бы одно CUDA-устройство. Без драйвера NVIDIA или со слишком старым драйвером возвращает `false`, а не падает.
- `cuda_device_name` — название видеокарты или `"no CUDA device"`.
- `cuda_memory_usage` — занятая и полная видеопамять в МБ.

```cpp
if (cuda_device_available()) {
    std::size_t used = 0, total = 0;
    if (cuda_memory_usage(used, total))
        std::printf("%s: %zu / %zu MB\n", cuda_device_name().c_str(), used, total);
}
```

### cudaCheck

```cpp
void cudaCheck(cudaError_t status, const char* what);
```

Если `status` не `cudaSuccess`, бросает `std::runtime_error` с текстом `what` и описанием ошибки CUDA.

### process_gpu_demosaic, process_gpu_demosaic_full

```cpp
Image process_gpu_demosaic(const RawImage& raw);
Image process_gpu_demosaic(const RawImage& raw, const std::function<bool(int, int)>& report);
Image process_gpu_demosaic_full(const RawImage& raw, bool malvar, const std::function<bool(int, int)>& report);
```

Демозаик на видеокарте:

1. выделить видеопамять;
2. загрузить мозаику;
3. запустить ядро — полосами по 256 строк результата для Binning и по 512 строк для полного размера, с вызовом `report` после каждой полосы;
4. скачать результат и освободить память.

При отмене возвращают пустой `Image`, при ошибке CUDA бросают исключение.

### gammaApply_GPU

```cpp
Image gammaApply_GPU(const Image& image, float gamma);
```

Гамма на видеокарте, результат совпадает с `apply_gamma(image, gamma)`.

## im_process.cuh — ядра CUDA

Функции запуска вызываются с **указателями на видеопамять** и работают **асинхронно**: после вызова нужно дождаться `cudaDeviceSynchronize()`.

```cpp
void launchDemosaicBin(const uint16_t* d_raw, int rawW, int rawH, uint8_t* d_out, int outW, int outH,
                       int blackLevel, int whiteLevel, unsigned cfaPattern,
                       float wbR, float wbG, float wbB, cudaStream_t stream = 0);

void launchDemosaicFull(const uint16_t* d_raw, int rawW, int rawH, uint8_t* d_out, int yStart, int rows,
                        int blackLevel, int whiteLevel, unsigned cfaPattern,
                        float wbR, float wbG, float wbB, int malvar, cudaStream_t stream = 0);

void launchApplyGamma(const uint8_t* d_src, uint8_t* d_dst, size_t count, float invGamma,
                      cudaStream_t stream = 0);
```

| Функция | Ядро | Что делает |
|---|---|---|
| `launchDemosaicBin` | `demosaicBinKernel` | Binning 2×2, блоки потоков 16×16 |
| `launchDemosaicFull` | `demosaicFullKernel` | Bilinear (`malvar = 0`) или Malvar (`malvar = 1`) для строк `yStart … yStart + rows − 1`; соседей читает `sampleRaw` с прижатием к краю |
| `launchApplyGamma` | `applyGammaKernel` | `255 · (v/255)^invGamma` для `count` байтов, по 256 потоков в блоке |

Так их использует `process_gpu_demosaic`:

```cpp
int outW = raw.width / 2;
int outH = raw.height / 2;
size_t outBytes = static_cast<size_t>(outW) * outH * 3;

uint16_t* d_raw = nullptr;
uint8_t* d_out = nullptr;

cudaCheck(cudaMalloc(reinterpret_cast<void**>(&d_raw), raw.size()), "cudaMalloc raw");
cudaCheck(cudaMalloc(reinterpret_cast<void**>(&d_out), outBytes), "cudaMalloc out");
cudaCheck(cudaMemcpy(d_raw, raw.data.data(), raw.size(), cudaMemcpyHostToDevice), "upload");

launchDemosaicBin(d_raw, raw.width, raw.height, d_out, outW, outH,
                  raw.black_level, raw.white_level, raw.cfaPattern, raw.wbR, raw.wbG, raw.wbB);
cudaCheck(cudaDeviceSynchronize(), "demosaic kernel");

Image image;
image.width = outW;
image.height = outH;
image.channels = 3;
image.data.resize(outBytes);

cudaCheck(cudaMemcpy(image.data.data(), d_out, outBytes, cudaMemcpyDeviceToHost), "download");

cudaFree(d_raw);
cudaFree(d_out);
```

## main.cpp — консольная проверка

```text
cuda_raw_processor.exe [файл.dng]
```

Без аргумента открывает `test.dng` в текущей папке. Печатает размер, black и white level и узор, затем сохраняет `raw_data.bin`, `raw_mosaic.tiff`, `output_cpu.png` (Binning на CPU) и `output_gpu.png` (Binning на GPU).

> [!NOTE]
> Консольная программа не проверяет наличие видеокарты: на компьютере без NVIDIA вызов `process_gpu_demosaic` завершится исключением. Приложение с окном проверяет устройство и переходит на CPU.

## gui.cpp — приложение

### Классы

| Класс | Что делает |
|---|---|
| `MainWindow` | главное окно: собирает интерфейс и связывает кнопки с обработкой |
| `TitleBar` | собственный заголовок окна с кнопками; перетаскивание и изменение размера — через `nativeEvent` |
| `BackgroundWidget` | фон центральной области |
| `ImageLabel` | область просмотра: зум, панорама, выделение с маркерами, лупа, миникарта, линейки, сетка, сравнение, перетаскивание файлов |
| `HistogramWidget` | гистограмма четырёх каналов |
| `ProfileWidget` | график профиля |
| `NoiseCompareWidget` | график «до / после» с осями, ±σ и логарифмической шкалой |
| `NoisePage` | вкладка NOISE · FILTERS: кнопки каналов, карточки, таблица, панель фильтра |
| `NoiseSnapshot` | снимок статистики и гистограмм области для «до» и «после» |
| `Document` | состояние одного открытого файла: кадр, оригинал, области, гамма, зум, поворот |
| `RegionsCommand`, `FrameCommand` | шаги истории отмены для списка областей и для кадра |

### Методы MainWindow

| Группа | Методы | Что делают |
|---|---|---|
| Файлы | `onLoad`, `loadRawFile`, `onLoadReference` | диалог выбора, загрузка файла в новую вкладку, загрузка reference-кадра |
| Вкладки файлов | `stashDocument`, `activateDocument`, `closeDocument`, `resetToEmpty` | сохранить состояние текущего файла, переключиться, закрыть, очистить окно |
| Фоновые задачи | `startTask`, `finishTask` | запустить работу в отдельном потоке с прогрессом и отменой; разблокировать кнопки и показать результат |
| Демозаик | `onProcess`, `OnProcessRegion`, `setDemosaicMethod`, `onGammaChanged`, `applyPendingGamma` | запуск демозаика для кадра или области; выбор метода; гамма с задержкой 50 мс, чтобы ползунок не тормозил |
| Области | `onRegionSelected`, `refreshAnalysis`, `applyNumericRegion`, `onSelectedRow`, `onSelectWholeColumn`, `nudgeRegion` | выделить область и пересчитать статистику, гистограмму и профиль |
| Список областей | `addRegion`, `removeRegion`, `clearRegions`, `rebuildRegionTable`, `applyRegionList` | изменить список через историю отмены и перерисовать таблицу |
| Шум | `compareFrames`, `subtractReference`, `measureOpticalBlack`, `applyMeasuredBlack`, `revertOriginal` | двухкадровый шум, темновой кадр, optical black, возврат к оригиналу |
| Вкладка шума | `showPage`, `refreshNoisePage`, `redrawNoisePage`, `makeNoiseSnapshot`, `fitHistogramRange`, `noiseChannel` | переключение вкладок; пересчёт снимков «до/после»; подбор диапазона гистограммы с шагом в целое число ADU |
| Фильтры | `applyDenoise`, `denoiseLabel`, `autoRangeSigma`, `logNoiseChange` | запуск фильтра в фоне и запись результата в историю; Auto для Range σ; запись изменения σ в журнал |
| Эталон «до» | `captureNoiseBefore`, `autoCaptureBefore`, `useOriginalAsBefore`, `resetNoiseReference` | запомнить «до» вручную, автоматически перед изменением, взять оригинал, сбросить |
| Кадр и история | `applyRawFrame` | подставить кадр из истории отмены и обновить всё, что от него зависит |
| Экспорт | `saveBin`, `saveRegionBin`, `saveTiff`, `saveRegionTiff`, `savePng`, `saveLinearTiff16`, `exportStats`, `exportRegions`, `writeCsvMeta`, `savePreset`, `loadPreset` | сохранение файлов, CSV и пресетов; `exportFolder` создаёт папку по имени файла и возвращает путь внутри неё, `writeExportNotes` кладёт туда же текстовый блокнот с размерами и уровнями |
| Вид | `rebuildPreview`, `rawToQt`, `imageToQt`, `toggleCompare`, `setRotation`, `setZoom`, `toggleFullScreen`, `togglePanels`, `copyViewToClipboard` | собрать картинку для экрана (с клиппингом и поворотом) и управлять видом |
| Состояние | `updateHud`, `updateCursorReadout`, `updateStatusFile`, `updateMetaInfo`, `updateMemoryUsage`, `logLine` | HUD, строка состояния, FILE INFO, память, журнал |
| Настройки | `loadSettings`, `saveSettings` | восстановить и сохранить настройки и расположение панелей |

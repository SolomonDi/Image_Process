# Сборка и GitHub

Как собрать программу из исходников, сделать установщик, отправить проект на GitHub и поставить программу на другой компьютер.

## Что установить для сборки

Для того чтобы **пользоваться** программой, всё это не нужно — достаточно установщика. Инструменты нужны только тому, кто собирает программу из исходников.

| Инструмент | Версия в проекте | Зачем |
|---|---|---|
| Visual Studio 2022 Build Tools, нагрузка «Разработка классических приложений на C++» | MSVC 19.39 | компилятор и линковщик |
| CMake | 4.4 (подойдёт 3.18 и новее) | описание сборки |
| Qt | 6.7.3, MSVC 2019 64-bit, в `C:\Qt\6.7.3\msvc2019_64` | окно и интерфейс |
| NVIDIA CUDA Toolkit | 13.0 | GPU-код; нужен для сборки даже без видеокарты |
| vcpkg | в `C:\vcpkg` | библиотеки LibRaw и libtiff |
| Git | любая свежая | репозиторий |
| Inno Setup | 6.7.3 | установщик |

Библиотеки через vcpkg:

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg install libraw:x64-windows tiff:x64-windows
```

> [!NOTE]
> `CMakeLists.txt` ищет Qt в `C:/Qt/6.7.3/msvc2019_64`. Если Qt стоит в другом месте, добавь к настройке CMake параметр `-DCMAKE_PREFIX_PATH=путь\к\Qt`.

## Сборка для разработки (Debug)

Все команды — в консоли **x64 Native Tools Command Prompt for VS 2022** из папки проекта. Полный путь к `cmake.exe` нужен, если в `PATH` раньше стоит CMake из MSYS2.

```bat
"C:\Program Files\CMake\bin\cmake.exe" -S . -B build -G Ninja ^
  -DCMAKE_BUILD_TYPE=Debug ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

"C:\Program Files\CMake\bin\cmake.exe" --build build --target cuda_raw_gui
```

При первой сборке рядом с exe нужно положить отладочные DLL Qt:

```bat
C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe --debug build\cuda_raw_gui.exe
build\cuda_raw_gui.exe
```

Если программа запущена, новая сборка упадёт с ошибкой `LNK1168`: закрой программу и собери снова.

## Release, папка программы и установщик

Одна команда делает всё:

```powershell
powershell -ExecutionPolicy Bypass -File installer\build_installer.ps1
```

Что делает скрипт:

1. собирает Release в `build-release` под видеокарты NVIDIA от RTX 20xx до RTX 50xx;
2. копирует `cuda_raw_gui.exe` и DLL из vcpkg в `dist\RawImageProcess`;
3. запускает `windeployqt`, который добавляет DLL и плагины Qt;
4. добавляет библиотеки Visual C++ и CUDA runtime;
5. упаковывает папку в `dist\RawImageProcess-1.0-portable.zip`;
6. если найден Inno Setup, собирает `dist\RawImageProcess-1.0-Setup.exe` по сценарию `installer\RawImageProcess.iss`.

Параметр `-SkipBuild` пропускает компиляцию и только собирает папку и установщик заново. Версия берётся из `kAppVersion` в `gui.cpp`.

### Чем Release отличается от Debug

В Release включена оптимизация компилятора и выключены отладочные проверки, поэтому фильтры шума работают в 3.4–6.2 раза быстрее. Ещё Release собирается как обычное оконное приложение, без чёрной консоли за окном. Для отладки используй Debug, людям отдавай Release.

### Проверка на «чистой» системе без видеокарты

У разработчика в `PATH` прописаны Qt и vcpkg, поэтому программа может случайно найти DLL, которых нет в папке. Проверить это можно так:

```bat
set PATH=C:\Windows\System32;C:\Windows
set CUDA_VISIBLE_DEVICES=-1
dist\RawImageProcess\cuda_raw_gui.exe
```

Первая строка оставляет только системные папки Windows, вторая прячет видеокарту от CUDA. Должно открыться окно с серой кнопкой GPU и надписью `no CUDA device` в строке состояния. После проверки закрой эту консоль.

Ещё надёжнее — **Песочница Windows** (Windows Sandbox) в Windows 10/11 Pro: чистая временная Windows без Qt, vcpkg и CUDA. Включается в «Включение или отключение компонентов Windows», после перезагрузки в неё можно скопировать Setup.exe и установить.

## GitHub

Репозиторий проекта: `https://github.com/SolomonDi/Image_Process`.

### Что сейчас лежит в репозитории

Вместе с исходниками в репозиторий когда-то попали файлы, которым там не место:

| Файл | Размер | Что с ним сделать |
|---|---|---|
| `raw_data.bin` | 80 МБ | результат работы программы; убрать из репозитория. GitHub предупреждает о файлах больше 50 МБ и не принимает файлы больше 100 МБ |
| `im_process.obj` | 57 КБ | промежуточный файл компиляции; убрать |
| `aqtinstall.log` | 26 КБ | журнал установки Qt; убрать |

В `.gitignore` уже добавлены `*.bin`, `*.log`, `build-release/`, `dist/` и `practice/`, чтобы такие файлы больше не попадали в коммиты. Новые файлы, которые нужно добавить: `denoise.hpp`, `installer/`, `docs/`, `README.md`.

### Отправить изменения

В PowerShell или обычной консоли из папки проекта:

```powershell
cd "C:\Users\RED.DOT\Desktop\PNG NOISE"
git status
git rm --cached raw_data.bin im_process.obj aqtinstall.log
git add .gitignore CMakeLists.txt README.md denoise.hpp gui.cpp gpu_process.hpp im_process.cu im_process.cuh im_read.hpp image_stats.hpp process.hpp installer docs
git status
git commit -m "Feat: Dmitriy Solomonov. Noise page, denoise filters, installer and docs"
git push origin main
```

| Команда | Зачем |
|---|---|
| `git status` | посмотреть, что изменено; перед коммитом — проверить, что в списке нет лишнего |
| `git rm --cached ...` | убрать файлы из репозитория; на диске они остаются |
| `git add ...` | отметить файлы для коммита |
| `git commit -m "..."` | сохранить снимок изменений с описанием |
| `git push origin main` | отправить коммит на GitHub |

> [!NOTE]
> `git rm --cached` убирает `raw_data.bin` из новых коммитов, но файл остаётся в истории. При клонировании он скачается один раз вместе с историей. Если репозиторий нужно сделать маленьким, историю можно почистить утилитой `git filter-repo`, но это отдельная аккуратная операция.

Если `git push` пишет `rejected ... fetch first`, значит, на GitHub есть коммиты, которых нет у тебя. Сначала подтяни их, потом отправь:

```powershell
git pull --rebase origin main
git push origin main
```

### Выложить установщик в Releases

Файлы `.exe` не хранят в самом репозитории (они в `.gitignore`), для готовых программ у GitHub есть страница **Releases**.

1. Собери установщик: `powershell -ExecutionPolicy Bypass -File installer\build_installer.ps1`.
2. Открой `https://github.com/SolomonDi/Image_Process` и справа нажми **Releases → Draft a new release**.
3. **Choose a tag** → впиши `v1.0` → **Create new tag**.
4. **Release title**: `Raw Image Process 1.0`; в описании кратко перечисли, что нового.
5. Перетащи в поле файлов `dist\RawImageProcess-1.0-Setup.exe` и `dist\RawImageProcess-1.0-portable.zip`.
6. Нажми **Publish release**.

То же самое из консоли, если установить GitHub CLI (`winget install --id GitHub.cli -e`) и войти (`gh auth login`):

```powershell
gh release create v1.0 dist\RawImageProcess-1.0-Setup.exe dist\RawImageProcess-1.0-portable.zip --title "Raw Image Process 1.0"
```

## Установка на другой компьютер

### Чтобы просто пользоваться

1. Открой `https://github.com/SolomonDi/Image_Process/releases`.
2. Скачай `RawImageProcess-1.0-Setup.exe` и установи. Если Windows покажет предупреждение SmartScreen — **Подробнее → Выполнить в любом случае**.
3. Без установки: скачай `RawImageProcess-1.0-portable.zip`, распакуй и запусти `cuda_raw_gui.exe`.

Нужна Windows 10 версии 1809 или новее либо Windows 11, 64-бит. Видеокарта не обязательна.

### Чтобы продолжать разработку

1. Установи инструменты из таблицы в начале раздела.
2. Склонируй репозиторий и собери:

   ```powershell
   git clone https://github.com/SolomonDi/Image_Process.git
   cd Image_Process
   powershell -ExecutionPolicy Bypass -File installer\build_installer.ps1
   ```

3. Тестовые RAW-файлы (`test.dng`, `test_2.DNG`) в репозитории не хранятся — они большие. Перенеси их отдельно: флешкой или через облако.

## Частые проблемы

| Сообщение | Причина и решение |
|---|---|
| `Could not find a package configuration file provided by "Qt6"` | Qt не в `C:\Qt\6.7.3\msvc2019_64`; укажи `-DCMAKE_PREFIX_PATH` |
| `No CMAKE_CUDA_COMPILER could be found` | не установлен CUDA Toolkit |
| `Could not find a package configuration file provided by "libraw"` | не выполнен `vcpkg install libraw` или не указан `CMAKE_TOOLCHAIN_FILE` |
| `LNK1168: cannot open cuda_raw_gui.exe for writing` | программа запущена; закрой её |
| `could not find or load the Qt platform plugin "windows"` | рядом с exe нет папки `platforms`; запусти `windeployqt` |
| `MSVCP140.dll` или `VCRUNTIME140_1.dll` не найден | не скопированы библиотеки Visual C++; пересобери скрипт или установи Visual C++ Redistributable |
| `File raw_data.bin is 80.00 MB; this is larger than GitHub's recommended maximum` | выполни `git rm --cached raw_data.bin` |
| «Windows защитила ваш компьютер» | у установщика нет цифровой подписи; «Подробнее → Выполнить в любом случае» |

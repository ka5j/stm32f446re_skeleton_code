# STM32F446RE Bare‑Metal Blinky (HAL, GCC, CMake, OpenOCD)
**From absolute zero to a blinking LED — reproducible, beginner‑friendly, and ready to extend.**

This README shows every step to build and flash a minimal firmware for the **NUCLEO‑F446RE** on a **Raspberry Pi 5 (Pi OS / Linux)** using:
- **arm-none-eabi-gcc** (ARM GNU toolchain)
- **CMake + Ninja** (portable, fast builds)
- **OpenOCD + ST‑LINK (SWD)** (program/debug)
- **STM32CubeF4** (HAL + CMSIS from ST)

---

## Table of Contents
1. [What You’ll Build](#what-youll-build)  
2. [Prerequisites](#prerequisites)  
3. [Create Project Folder](#create-project-folder)  
4. [Bring In STM32CubeF4 (HAL/CMSIS)](#bring-in-stm32cubef4-halcmsis)  
5. [Add Required Support Files](#add-required-support-files)  
6. [Write the Firmware Code](#write-the-firmware-code)  
7. [CMake Toolchain + Project Files](#cmake-toolchain--project-files)  
8. [Configure & Build](#configure--build)  
9. [Flash (Upload) to the Board](#flash-upload-to-the-board)  
10. [Debug Session (Optional but Recommended)](#debug-session-optional-but-recommended)  
11. [VS Code IntelliSense (Optional, Nice to Have)](#vs-code-intellisense-optional-nice-to-have)  
12. [Extend the Project (Add Peripherals)](#extend-the-project-add-peripherals)  
13. [Use this skeleton for a brand‑new project](#use-this-skeleton-for-a-brand‑new-project)  
14. [Troubleshooting](#troubleshooting)  
15. [Why This Approach (Industry Rationale)](#why-this-approach-industry-rationale)

---

## What You’ll Build
A minimal **blink** firmware using the STM32 HAL:
- LED **LD2** on the NUCLEO‑F446RE (pin **PA5**) toggles every **250 ms**.
- `HAL_Delay()` works because **SysTick** interrupt is wired via `stm32f4xx_it.c`.

You will:
1. Set up **CMake + Ninja** for clean, out‑of‑source builds.  
2. Use **STM32CubeF4** (HAL + CMSIS) as a **Git submodule**.  
3. Compile a `.elf` and **flash** it to the MCU with **OpenOCD**.  
4. Optionally **debug** with **GDB** (breakpoints, step, inspect).

---

## Prerequisites
Install these on your Raspberry Pi / Linux box:
```bash
sudo apt update
sudo apt install -y git cmake ninja-build openocd
# Install ARM GNU Toolchain (arm-none-eabi-*) appropriate for AArch64 (Pi 5).
# If already installed, ensure it's on PATH:
arm-none-eabi-gcc --version
```

Hardware:
- **NUCLEO‑F446RE** board (USB connected to the Pi; on‑board **ST‑LINK** provides SWD).

Optional but handy:
- **Visual Studio Code** with *C/C++* and *CMake Tools* extensions.

---

## Create Project Folder
Pick a workspace and make the project root:
```bash
mkdir -p ~/dev && cd ~/dev
git init stm32f446re_skeleton_code && cd stm32f446re_skeleton_code
```

Create the folders we’ll use:
```bash
mkdir -p cmake include src system startup linker third_party build
```

---

## Bring In STM32CubeF4 (HAL/CMSIS)
Add ST’s HAL/CMSIS pack as a **submodule** so your repo pins the exact version:
```bash
git submodule add https://github.com/STMicroelectronics/STM32CubeF4.git third_party/STM32CubeF4
cd third_party/STM32CubeF4
git submodule update --init --recursive
cd ../..
```

Why submodule?
- Reproducible builds: teammates clone with `--recursive` and get the same HAL version you used.
- Easy, explicit upgrades later by bumping the submodule commit.

---

## Add Required Support Files
You need **four kinds** of files in your repo to build an STM32 HAL project:

1) **HAL config header** (project‑local)  
2) **Startup assembly** (device‑specific)  
3) **System source** (CMSIS system init)  
4) **Linker script** (memory layout)

Run these from the project root:

```bash
# 1) HAL config: copy the template into your project and drop "_template"
cp third_party/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_conf_template.h \
   include/stm32f4xx_hal_conf.h

# 2) Startup assembly for STM32F446
cp third_party/STM32CubeF4/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f446xx.s \
   startup/

# 3) System file (shared by F4 devices)
cp third_party/STM32CubeF4/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c \
   system/

# 4) Linker script — choose an F446RE one from the Cube examples/templates you have
# Use the "Templates" one (ideal), or any F446RE example's .ld:
cp third_party/STM32CubeF4/Projects/STM32446E-Nucleo/Templates/STM32CubeIDE/STM32F446RETX_FLASH.ld \
   linker/
```

> If paths differ in your Cube version, discover the linker script with:
> ```bash
> find third_party/STM32CubeF4/Projects -type f -iregex '.*STM32F446RE.*FLASH\.ld'
> ```

**Edit `include/stm32f4xx_hal_conf.h`** as needed (enable modules you use):
```c
#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
/* add more as you need later */
```

---

## Write the Firmware Code

Create `src/main.c`:

Create `src/stm32f4xx_it.c`

**Why src/stm32f4xx_it.c?** 
This file holds the interrupt service routines (ISRs) for the MCU. The startup file (`startup_stm32f446xx.s`) defines a vector table with weak default handlers for every IRQ. To make an interrupt actually do something, you override the weak symbol by providing a function with the exact same name in C
---

## CMake Toolchain + Project Files

Create `cmake/arm-gcc-toolchain.cmake`:

Create `CMakeLists.txt`:

**Why CMake?**  
You describe *what* to build; CMake generates the long `arm-none-eabi-gcc -c` & `-o` commands for each file, then Ninja compiles only what changed. Portable, fast, and CI‑friendly.

---

## Configure & Build
From the project root:
```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-gcc-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
ninja -C build
```
- `-S .` = source dir (has `CMakeLists.txt`)  
- `-B build` = out‑of‑source build dir (clean and disposable)  
- `-G Ninja` = generate Ninja files (fast incremental builds)  
- Toolchain file = tells CMake which compiler/toolchain to use

Result: `build/nucleo_f446re_blinky.elf`

> If you change toolchains or want a clean re‑configure:
> ```bash
> rm -rf build
> cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-gcc-toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
> ninja -C build
> ```

---

## Flash (Upload) to the Board
Connect the NUCLEO via USB. Two solid choices:

### A) One‑liner “flash & go” (no debugger UI)
```bash
openocd -f board/st_nucleo_f4.cfg \
  -c "program build/stm32f446re_skeleton_code.elf verify reset exit"
```
This tells OpenOCD to erase/program/verify, **reset the MCU**, and exit. If successful, **LD2 (PA5)** blinks.

### B) With GDB (debug, breakpoints, inspect)
Terminal 1:
```bash
openocd -f board/st_nucleo_f4.cfg
```
Terminal 2:
```gdb
arm-none-eabi-gdb build/nucleo_f446re_blinky.elf
(gdb) target extended-remote :3333
(gdb) monitor reset halt
(gdb) load
(gdb) continue
```
- OpenOCD runs a **GDB server** on port `3333`  
- `monitor` forwards commands to OpenOCD  
- `load` programs flash via GDB

**Logic recap:** OpenOCD bridges your PC ↔ ST‑LINK ↔ MCU. The `board/st_nucleo_f4.cfg` file sets up ST‑LINK + SWD and the target (STM32F4).

---

## Debug Session (Optional but Recommended)
- Set a breakpoint in `HAL_Delay()` or `SysTick_Handler()` to see timebase behavior.
- Telnet to OpenOCD (optional): `telnet localhost 4444` → commands like `reset halt`, `reg`, `mdw`.

---

## VS Code IntelliSense (Optional, Nice to Have)
Point IntelliSense to your compiler and **compile_commands.json** so squiggles disappear and autocompletion is accurate.

Open Command Palette → **C/C++: Edit Configurations (UI)**:
- **Compiler path**: `/usr/bin/arm-none-eabi-gcc`
- **Compile Commands**: `${workspaceFolder}/build/compile_commands.json`

Or JSON:
```json
// .vscode/c_cpp_properties.json
{
  "version": 4,
  "configurations": [{
    "name": "stm32",
    "compilerPath": "/usr/bin/arm-none-eabi-gcc",
    "compileCommands": "${workspaceFolder}/build/compile_commands.json",
    "intelliSenseMode": "gcc-arm"
  }]
}
```

---

## Extend the Project (Add Peripherals)
When you add new modules (e.g., UART, SPI, I²C, ADC+DMA):

1. Create source/header files in `src/` (or a subfolder).  
2. **Update `CMakeLists.txt`**: add the new `.c` file(s) to `SRCS` and any include paths.  
3. Rebuild:
   ```bash
   cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-gcc-toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
   ninja -C build
   ```
4. Flash again (OpenOCD one‑liner or GDB).

> You can also use `file(GLOB CONFIGURE_DEPENDS ...)` to auto‑pick new files, but most teams prefer **explicit lists** for clarity and reproducibility.

---

## Use this skeleton for a **brand‑new project** (duplicate locally, *no history*)

This path gives you a clean repo that has the **same files/structure** as the skeleton, but **none of its commit history**. It works on GitHub, GitLab, or any Git server.

> Summary of what you’ll do
> 1) Copy files from the skeleton (without keeping its history)  
> 2) Start fresh Git history on your machine  
> 3) Connect your own remote and push  
> 4) Initialize any submodules this skeleton uses

### Step‑by‑step

```bash
# 0) (Recommended) Create an EMPTY repo on your Git host first
#    Example: https://github.com/<you>/<my_new_proj>
#    Leave it empty (no README/license/gitignore), or it's fine to overwrite later.

# 1) Copy the skeleton files to a new local folder
git clone https://github.com/ka5j/stm32f446re_skeleton_code.git my_new_proj
cd my_new_proj

# 2) Remove the skeleton's Git history and start fresh
rm -rf .git

# Initialize a new repo with 'main' as the default branch (Git ≥ 2.28)
git init -b main
# If your Git is older:
# git init
# git checkout -b main

# 3) Make the first commit in your NEW project
git add .
git commit -m "Start from skeleton (fresh history)"

# 4) Connect your own remote and push
git remote add origin https://github.com/<you>/<my_new_proj>.git
git push -u origin main

# 5) Pull submodule contents (if this skeleton uses any)
git submodule sync --recursive
git submodule update --init --recursive
```

### Why these exact commands?
- `rm -rf .git` wipes the old history so your project starts clean.  
- `git init -b main` creates a new repo with **main** as the default branch (works on Git ≥ 2.28).  
- `git remote add origin …` ties your local repo to your new server‑side repo.  
- `git submodule update --init --recursive` ensures **all** submodules (including nested ones) are fetched at the versions recorded by this skeleton.

### Tips
- Need to change the remote URL later?  
  ```bash
  git remote set-url origin https://github.com/<you>/<my_new_proj>.git
  ```
- Forgot to create the remote first? You can create it after Step 3, then run Step 4.  
- Prefer history **mirroring** (to copy all commits/branches)? Use your host’s “mirror/duplicate repository” instructions instead—this snippet is specifically for **no history**.

---

## Troubleshooting

**LED turns on but doesn’t blink**  
- Likely missing `SysTick_Handler` → ensure `src/stm32f4xx_it.c` exists and is added to `SRCS`.  
- Ensure `include/stm32f4xx_hal_conf.h` exists (copied from `_template`) and your include path contains `include/`.

**VS Code shows `__IO`, `uint8_t` etc. as undefined**  
- Configure IntelliSense (see section above). The build is correct; the editor just needs the same flags/includes your build uses.

**OpenOCD USB permission error**  
- Install udev rules (see Prerequisites), reload udev, unplug/re‑plug board.

**Toolchain warning about CMAKE_TOOLCHAIN_FILE**  
- That happens if you reconfigure an *existing* `build/` with a different toolchain file. Delete `build/` and re‑run the CMake command.

---

## Why This Approach (Industry Rationale)
- **CMake + Ninja**: portable, fast, and the standard way to orchestrate GCC in many embedded teams; works great in CI.  
- **Git submodule for STM32CubeF4**: pins the exact HAL/CMSIS version; reproducible for collaborators.  
- **OpenOCD + GDB**: de‑facto open‑toolchain flow for STM32 + ST‑LINK/SWD; consistent across boards.  
- **Out‑of‑source builds**: keeps your repo clean; a “clean build” is as simple as deleting `build/`.

---

# Build opons-voxd on Arch Linux (GPU)

Step-by-step guide to build **opons-voxd** — a GTK3 desktop app with whisper.cpp speech recognition and CUDA GPU acceleration.

---

## Prerequisites

### 1. NVIDIA GPU + Drivers

Verify your GPU is detected:

```bash
nvidia-smi
```

Expected output shows your GPU (e.g. RTX 3060) with CUDA version.

Install NVIDIA drivers and toolkit:

```bash
sudo pacman -S nvidia nvidia-utils nvidia-settings
```

For CUDA development tools (nvcc):

```bash
sudo pacman -S cuda
```

This installs nvcc to `/opt/cuda/bin/nvcc`. Verify:

```bash
nvcc --version
# Should show CUDA 12.x or 13.x
```

### 2. Build Dependencies

Install all required packages:

```bash
sudo pacman -S gcc cmake pkg-config git curl make \
  gtk3 libnotify cairo x11 xtst portaudio alsa-lib
```

**Note:** We deliberately do NOT install `libportaudio-dev` (the header package). The build script vendors the PortAudio header automatically. We link directly against `libportaudio.so.2` (the SONAME from the runtime package).

### 3. Clone the Repository

```bash
git clone <repo-url> $HOME/sandbox/opons-voxd
cd $HOME/sandbox/opons-voxd
```

---

## Build Steps

### Step 1: Setup (whisper.cpp + Model)

```bash
make setup
```

This does three things:

1. **Configures whisper.cpp** with CMake, detecting CUDA automatically
2. **Compiles whisper.cpp** with `GGML_CUDA=ON` targeting your GPU's compute capability
3. **Downloads the whisper medium model** (~1.4 GB) to `whisper.cpp/models/ggml-medium.bin`

What happens under the hood:

- The Makefile auto-detects `nvcc` at `/opt/cuda/bin/nvcc`
- It queries your GPU's compute capability via `nvidia-smi` (e.g. `8.6` for RTX 30xx)
- It passes `-DCMAKE_CUDA_ARCHITECTURES=86-real` to compile only SASS for your exact GPU (no embedded PTX, faster compilation)
- NCCL is optional — if not installed, multi-GPU scaling is limited but single-GPU works fine

### Step 2: Build the Binary

```bash
make
```

This compiles `opons_voxd.c` and links it against:

- whisper.cpp static libraries (CPU + CUDA backends)
- GTK3, Pango, Cairo for the GUI
- libnotify for desktop notifications
- X11/Xtst for keyboard/mouse control
- PortAudio for audio capture
- CUDA runtime libraries (cudart, cublas)

The output binary is `opons-voxd` in the project root (~135 MB).

### Step 3: Run

```bash
./opons-voxd
```

---

## GPU Verification

To confirm CUDA is active, run the whisper CLI directly:

```bash
./whisper.cpp/build/bin/whisper-cli \
  -m whisper.cpp/models/ggml-medium.bin \
  -f samples/jfk.wav \
  -t 4
```

Watch `nvidia-smi` in another terminal — you should see GPU memory usage and compute activity during transcription.

---

## Troubleshooting

### "nvcc not found"

The Makefile checks these paths in order:
1. `/usr/local/cuda/bin/nvcc`
2. `/opt/cuda/bin/nvcc`
3. `nvcc` in `$PATH`

If none are found, CUDA support is disabled and the build falls back to CPU-only. Install `cuda` via pacman or set `CUDA_DIR` manually:

```bash
make setup CUDA_DIR=/path/to/cuda
```

### "CUDART not found"

The Makefile checks:
1. `/usr/local/cuda/lib64/libcudart.so`
2. `/opt/cuda/targets/x86_64-linux/lib/libcudart.so`

If missing, ensure `nvidia-utils` is installed and the CUDA library path is in ldconfig:

```bash
echo '/opt/cuda/targets/x86_64-linux/lib' | sudo tee /etc/ld.so.conf.d/cuda.conf
sudo ldconfig
```

### "NCCL not found"

NCCL is only needed for multi-GPU setups. For a single GPU, this warning is harmless:

```
Warning: NCCL not found, performance for multiple CUDA GPUs will be suboptimal
```

Install it if you have multiple GPUs:

```bash
sudo pacman -S nccl
```

### "ccache not found"

ccache speeds up recompilation. Optional but recommended:

```bash
sudo pacman -S ccache
```

Or disable the warning with:

```bash
make setup GGML_CCACHE=OFF
```

### CUDA Architecture Mismatch

If you get PTX compilation errors, the embedded PTX version doesn't match your nvcc. The Makefile uses `-real` suffix by default (SASS only), which avoids this issue. To override:

```bash
# PTX + SASS for broader compatibility (slower compile)
make setup CUDA_ARCH=86

# SASS only for your GPU (faster compile)
make setup CUDA_ARCH=86-real
```

### PortAudio Issues

The build vendors `portaudio.h` automatically on first run. If it fails:

```bash
mkdir -p vendor/portaudio
curl -o vendor/portaudio.h \
  https://raw.githubusercontent.com/PortAudio/portaudio/v19.7.0/include/portaudio.h
```

Then rebuild:

```bash
make
```

### Clean Builds

Remove the binary only:

```bash
make clean
```

Remove whisper.cpp and rebuild from scratch:

```bash
make distclean
make setup
make
```

---

## System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| GPU | NVIDIA with CUDA 11.8+ | RTX 3060 or better |
| VRAM | 4 GB | 6 GB+ |
| RAM | 8 GB | 16 GB+ |
| Disk | 3 GB (binary + model) | 5 GB (with build artifacts) |
| CUDA Toolkit | 11.8+ | 12.x or 13.x |
| Drivers | 525+ (for CUDA 11.8) | 535+ (for CUDA 12.x+) |

---

## Quick Reference

```bash
# Full rebuild from scratch
make distclean && make setup && make

# Just rebuild the binary (skip model download)
make

# Run with verbose CUDA output
export WHISPER_DEBUG=1
./opons-voxd

# Check GPU status during use
watch -n 1 nvidia-smi
```

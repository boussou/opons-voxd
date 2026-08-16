# Makefile for opons-voxd
#
# Usage:
#     make setup      # clone whisper.cpp, build it, download the model
#     make            # build the opons-voxd binary
#     make run        # launch it
#     make clean      # remove opons-voxd binary
#     make distclean  # also remove whisper.cpp
#
# CUDA is auto-detected. If nvcc and libcudart are found, whisper.cpp is
# built with GPU support. Otherwise everything runs on CPU.
#
# System deps (apt):
#     build-essential cmake pkg-config git curl
#     libgtk-3-dev libnotify-dev libcairo2-dev libx11-dev libxtst-dev
#     libportaudio2          # runtime only — header is vendored (see below)
#     xclip libnotify-bin
#     (optional) nvidia-cuda-toolkit or equivalent providing nvcc
#
# Note: we deliberately avoid libportaudio-dev. On multiarch Debian/Ubuntu
# it can force the removal of i386 packages used by wine. Instead we vendor
# portaudio.h on first build and link directly against libportaudio.so.2
# (the SONAME shipped by libportaudio2).

CC       := gcc
CFLAGS   := -O2 -Wall -Wextra -Wno-unused-parameter -Wno-deprecated-declarations -std=c11
LDFLAGS  :=

WHISPER_DIR   := whisper.cpp
WHISPER_BUILD := $(WHISPER_DIR)/build

MODEL_NAME := medium
MODEL_FILE := $(WHISPER_DIR)/models/ggml-$(MODEL_NAME).bin

PKG_CFLAGS := $(shell pkg-config --cflags gtk+-3.0 libnotify x11 xtst)
PKG_LIBS   := $(shell pkg-config --libs   gtk+-3.0 libnotify x11 xtst)

# Vendored PortAudio header (see top-of-file note). Fetched on first build,
# pinned to v19.7.0 — ABI-compatible with any libportaudio.so.2 in distros.
PA_VENDOR_DIR := vendor/portaudio
PA_HEADER     := $(PA_VENDOR_DIR)/portaudio.h
PA_HEADER_URL := https://raw.githubusercontent.com/PortAudio/portaudio/v19.7.0/include/portaudio.h
PA_LIBS       := -l:libportaudio.so.2

INCLUDES     := -I$(WHISPER_DIR)/include -I$(WHISPER_DIR)/ggml/include -I$(PA_VENDOR_DIR)
WHISPER_LIBS := $(shell find $(WHISPER_BUILD) -name 'lib*.a' 2>/dev/null)

# --- CUDA auto-detection ---
# Prefer /usr/local/cuda/bin/nvcc over PATH: distros often ship an older
# nvcc (e.g. apt nvidia-cuda-toolkit 11.5) in /usr/bin while a newer
# toolkit lives under /usr/local/cuda. The older nvcc can't emit PTX
# features used by current whisper.cpp (e.g. movmatrix needs PTX 7.8 / CUDA 11.8+).
NVCC_FOUND   := $(or $(wildcard /usr/local/cuda/bin/nvcc),$(wildcard /opt/cuda/bin/nvcc),$(shell command -v nvcc 2>/dev/null))
CUDART_FOUND := $(or $(wildcard /usr/local/cuda/lib64/libcudart.so),$(wildcard /opt/cuda/targets/x86_64-linux/lib/libcudart.so))

# Detect GPU compute capability via nvidia-smi (e.g. "7.5" -> "75-real").
# "-real" suffix => only SASS for this arch, no embedded PTX. Avoids inline-asm
# PTX version mismatches (e.g. movmatrix requires PTX 7.8) when targeting only
# the local GPU. Override with: make setup CUDA_ARCH=75 (PTX+SASS) or =75-real.
CUDA_ARCH ?= $(shell nvidia-smi --query-gpu=compute_cap --format=csv,noheader 2>/dev/null | head -n1 | awk -F. '{ if ($$1 != "") print $$1 $$2 "-real" }')

ifneq ($(NVCC_FOUND),)
ifneq ($(CUDART_FOUND),)
    HAS_CUDA     := 1
    CUDA_DIR     := /opt/cuda
    CUDA_LIBS    := -L/opt/cuda/targets/x86_64-linux/lib -lcudart -lcublas -lcublasLt -lcuda
    CUDA_CMAKE := -DGGML_CUDA=ON -DCMAKE_CUDA_COMPILER=$(NVCC_FOUND)
    ifneq ($(strip $(CUDA_ARCH)),)
        CUDA_CMAKE += -DCMAKE_CUDA_ARCHITECTURES=$(CUDA_ARCH)
    endif
else
    HAS_CUDA     := 0
    CUDA_LIBS    :=
    CUDA_CMAKE   :=
endif
else
    HAS_CUDA     := 0
    CUDA_LIBS    :=
    CUDA_CMAKE   :=
endif

# ---- targets ----

all: opons-voxd

$(PA_HEADER):
	@mkdir -p $(PA_VENDOR_DIR)
	@echo "[fetch] portaudio.h ($(PA_HEADER_URL))"
	curl -fsSL -o $@ $(PA_HEADER_URL)

opons-voxd: opons_voxd.c $(PA_HEADER)
ifeq ($(HAS_CUDA),1)
	@echo "[build] CUDA detected — linking with GPU support"
else
	@echo "[build] CUDA not found — CPU only"
endif
	$(CC) $(CFLAGS) $(INCLUDES) $(PKG_CFLAGS) \
	    opons_voxd.c -o $@ \
	    -Wl,--start-group $(WHISPER_LIBS) -Wl,--end-group \
	    $(PKG_LIBS) $(PA_LIBS) $(CUDA_LIBS) $(LDFLAGS) \
	    -lm -lpthread -lstdc++ -fopenmp

setup:
	@if [ ! -d "$(WHISPER_DIR)" ]; then \
	    git clone --depth 1 https://github.com/ggerganov/whisper.cpp.git $(WHISPER_DIR); \
	fi
	@# Patch: disable -compress-mode=... which leaks to host compiler (gcc)
	@# in some CMake/nvcc combos and breaks the CUDA build.
	@sed -i 's|if (CUDAToolkit_VERSION VERSION_GREATER_EQUAL "12.8")|if (FALSE AND CUDAToolkit_VERSION VERSION_GREATER_EQUAL "12.8")|' \
	    $(WHISPER_DIR)/ggml/src/ggml-cuda/CMakeLists.txt 2>/dev/null || true
ifeq ($(HAS_CUDA),1)
	@echo "[setup] CUDA detected — building whisper.cpp with GPU support (nvcc=$(NVCC_FOUND), arch=$(or $(CUDA_ARCH),default))"
else
	@echo "[setup] CUDA not found — building whisper.cpp for CPU only"
endif
	cd $(WHISPER_DIR) && cmake -B build \
	    -DCMAKE_BUILD_TYPE=Release \
	    -DBUILD_SHARED_LIBS=OFF \
	    -DWHISPER_BUILD_EXAMPLES=OFF \
	    -DWHISPER_BUILD_TESTS=OFF \
	    $(CUDA_CMAKE)
	cmake --build $(WHISPER_BUILD) -j$$(nproc) --target whisper
	@if [ ! -f "$(MODEL_FILE)" ]; then \
	    cd $(WHISPER_DIR) && bash ./models/download-ggml-model.sh $(MODEL_NAME); \
	fi
	@echo ""
	@echo "setup done. Run: make && make run"

run: opons-voxd
	OPONS_VOXD_MODEL=$(MODEL_FILE) ./opons-voxd

clean:
	rm -f opons-voxd

distclean: clean
	rm -rf $(WHISPER_DIR) vendor

.PHONY: all setup run clean distclean

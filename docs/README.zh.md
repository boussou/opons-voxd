# opons-voxd

> Note: opons-voxd is unrelated to jakovius/voxd. The name was chosen independently — `opons` = O. Pons (author), `voxd` = vox + daemon (Latin/Unix tradition).

> **⚠ Backwards compatibility intentionally broken.** This project was previously `voice_in_linux`. The rename touched the binary name, the source filename, every environment variable (`VOICE_IN_*` → `OPONS_VOXD_*`), and the runtime dependency on `xdotool` (replaced by direct XTest synthesis via `libxtst-dev`). No aliases — update your launch scripts, autostart entries, and `apt install` lines.

**Linux 本地语音转文字工具 ── 您的声音永远不会离开您的电脑。**

🌍 **语言：**
[English](../README.md) · [Français](README.fr.md) · [Deutsch](README.de.md) · [日本語](README.ja.md) · [Español](README.es.md) · [Italiano](README.it.md)

---

## 为什么做这个项目？

我希望能在 Linux 上对任何应用进行语音输入，而不需要将音频发送到第三方服务器。现有的所有方案——浏览器扩展、云端 API、SaaS 平台——都会将您的音频流式传输到远程服务器进行转录。这意味着您说出的每一个字都会经过互联网传输：邮件、机密文件、客户数据、个人笔记，全部如此。

本项目是一个轻量级的、完全本地化的替代方案。它完全运行在您的本地机器上，使用 [whisper.cpp](https://github.com/ggerganov/whisper.cpp)——OpenAI 开源的语音识别模型。初始安装完成后无需任何网络连接，数据永远不会离开您的电脑。

借助人工智能开发完成。

---

## 工作原理

1. 系统托盘中出现一个小图标
2. **左键单击** → 开始录音（图标变红）
3. **再次左键单击** → 停止录音，通过 Whisper 在本地进行转录
4. 转录文本自动复制到剪贴板（X11 的 PRIMARY 和 CLIPBOARD 两种选区）
5. 桌面通知显示文本 10 秒
6. 使用 `Ctrl+Shift+V`、`Shift+Insert` 或鼠标中键粘贴

---


### Screenshots

| 空闲 | 录音中 |
|:---:|:---:|
| ![空闲](../screenshots/opons_voxd_inactive.png) | ![录音中](../screenshots/opons_voxd_active.png) |

## 功能特点

- **100% 本地运行** ── 音频在 CPU 或 GPU 上处理，绝不外传
- **GPU 加速** ── 支持 NVIDIA CUDA，实现近乎即时的转录
- **99 种语言** ── 基于 OpenAI Whisper

> ⚠️ **NVIDIA GPU + CUDA 对于可接受的响应时间至关重要。**
> 没有 CUDA：15 秒语音需要 **10-15 秒**处理。
> 使用 CUDA：同样的转录**不到 1 秒**。
> 如果您有 NVIDIA GPU（GTX 1060 或更新），请优先配置 CUDA。
- **轻量级** ── 单个 C 语言二进制文件（约 100 KB），无需 Python
- **系统托盘集成** ── 任务栏中的小图标
- **双剪贴板** ── 文本同时写入 PRIMARY 和 CLIPBOARD
- **语音命令** ── 每种语言一个命令文件（`commands/` 目录），默认禁用（`OPONS_VOXD_COMMANDS=1` 启用）。添加新语言只需创建 `commands/xx.txt`，无需重新编译。
- **瞬态通知** ── 通知出现后自动消失。设置 `OPONS_VOXD_NOTIFY_PERSIST=1` 可保留在通知历史中。
- **Notification mode** — `OPONS_VOXD_NOTIFY=normal|quiet|silent|off` controls success notifications (default `quiet`: bubble, no sound). Errors always show with sound, in every mode, without exception. See main README for the full table.
- **自动大写** ── 句首字母自动大写
- **开机自启** ── 可配置为登录时自动启动

---

## 安装

```bash
# 1. 系统依赖
sudo apt update
sudo apt install -y \
    build-essential cmake pkg-config git \
    libgtk-3-dev libnotify-dev libportaudio-dev libcairo2-dev libxtst-dev \
    xclip libnotify-bin

# 2. 克隆项目
git clone https://github.com/olivierpons/opons-voxd.git
cd opons-voxd

# 3. 编译 whisper.cpp 并下载模型
make setup

# 4. 编译二进制文件
make

# 5. 运行
./opons-voxd
```

---

## 许可证

全部采用 MIT 许可证：Whisper 模型（OpenAI）、whisper.cpp（Georgi Gerganov）以及本项目。

完整文档请参阅[主 README](../README.md)。

---

## 代码风格

本项目遵循 **Linux 内核编码风格**（`Documentation/process/coding-style.rst`），由 Olivier Pons 进行适配：K&R 花括号风格、4 空格缩进、直接使用 `struct name`（不使用 typedef）、无人为前缀（`s_`、`t_`、`e_`）、`return value;` 不加括号、kernel-doc 注释风格、每行最多 80 个字符。完整详情请参阅[主 README](../README.md)。

## Push-to-talk hotkey

The generated `~/.config/opons-voxd.conf` uses `ctrl+alt+w` (recommended on AZERTY) or `ctrl+alt+z` (recommended on QWERTY). The key sits next to `A`, so the whole combo is reachable with the left hand alone, and isn't bound by default in most desktop environments. Configure with `OPONS_VOXD_PTT_HOTKEY`. See the [main README](../README.md#push-to-talk-hotkey) for full details.


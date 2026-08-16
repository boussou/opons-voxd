# opons-voxd

> Note: opons-voxd is unrelated to jakovius/voxd. The name was chosen independently — `opons` = O. Pons (author), `voxd` = vox + daemon (Latin/Unix tradition).

> **⚠ Backwards compatibility intentionally broken.** This project was previously `voice_in_linux`. The rename touched the binary name, the source filename, every environment variable (`VOICE_IN_*` → `OPONS_VOXD_*`), and the runtime dependency on `xdotool` (replaced by direct XTest synthesis via `libxtst-dev`). No aliases — update your launch scripts, autostart entries, and `apt install` lines.

**Linux向けローカル音声テキスト変換ツール ── 音声データがマシンの外に出ることは一切ありません。**

🌍 **言語：**
[English](../README.md) · [Français](README.fr.md) · [Deutsch](README.de.md) · [中文](README.zh.md) · [Español](README.es.md) · [Italiano](README.it.md)

---

## なぜこのプロジェクトを作ったのか

Linux上のあらゆるアプリケーションにテキストを音声入力したい。しかし、音声をサードパーティのサーバーに送信したくない。既存のソリューション（ブラウザ拡張機能、クラウドAPI、SaaSプラットフォーム）はすべて、音声をリモートサーバーにストリーミングして文字起こしを行います。つまり、話したすべての言葉がインターネットを経由します：メール、機密文書、顧客データ、個人メモ、すべてです。

このプロジェクトは、軽量で完全にローカルな代替手段です。[whisper.cpp](https://github.com/ggerganov/whisper.cpp)（OpenAIのオープンソース音声認識モデル）を使用して、完全にローカルマシン上で動作します。初回セットアップ後はネットワーク接続は不要です。データがコンピューターの外に出ることはありません。

AIの助けを借りて開発しました。

---

## 使い方

1. システムトレイに小さなアイコンが表示されます
2. **左クリック** → 録音開始（アイコンが赤に変化）
3. **再度左クリック** → 録音停止、Whisperによるローカル文字起こし
4. 文字起こしされたテキストが自動的にクリップボードにコピーされます（X11のPRIMARYとCLIPBOARD両方）
5. デスクトップ通知にテキストが10秒間表示されます
6. `Ctrl+Shift+V`、`Shift+Insert`、または中クリックで貼り付け

---


### Screenshots

| 待機中 | 録音中 |
|:---:|:---:|
| ![待機中](../screenshots/opons_voxd_inactive.png) | ![録音中](../screenshots/opons_voxd_active.png) |

## 機能

- **100%ローカル** ── 音声はCPUまたはGPUで処理され、外部に送信されません
- **GPU高速化** ── NVIDIA CUDAサポートによるほぼ瞬時の文字起こし
- **99言語対応** ── OpenAI Whisperによる多言語サポート

> ⚠️ **実用的な応答速度にはNVIDIA GPU + CUDAが不可欠です。**
> CUDAなし：15秒の音声に対して**10〜15秒**の処理時間。
> CUDAあり：同じ文字起こしが**1秒未満**。
> NVIDIA GPU（GTX 1060以降）をお持ちの場合、CUDAのセットアップを最優先にしてください。
- **軽量** ── 単一のCバイナリ（約100 KB）、Pythonは不要
- **システムトレイ統合** ── タスクバーの目立たないアイコン
- **デュアルクリップボード** ── PRIMARYとCLIPBOARDの両方にテキストをコピー
- **音声コマンド** ── 言語ごとのコマンドファイル（`commands/`）、デフォルトで無効（`OPONS_VOXD_COMMANDS=1` で有効化）。新言語追加は `commands/xx.txt` を作成するだけ、再コンパイル不要。
- **一時的通知** ── 通知は表示後完全に消えます。`OPONS_VOXD_NOTIFY_PERSIST=1` で通知履歴に残す。
- **Notification mode** — `OPONS_VOXD_NOTIFY=normal|quiet|silent|off` controls success notifications (default `quiet`: bubble, no sound). Errors always show with sound, in every mode, without exception. See main README for the full table.
- **自動大文字化** ── 文頭が自動的に大文字になります
- **自動起動** ── ログイン時に自動起動を設定可能

---

## インストール

```bash
# 1. システム依存パッケージ
sudo apt update
sudo apt install -y \
    build-essential cmake pkg-config git \
    libgtk-3-dev libnotify-dev libportaudio-dev libcairo2-dev libxtst-dev \
    xclip libnotify-bin

# 2. リポジトリをクローン
git clone https://github.com/olivierpons/opons-voxd.git
cd opons-voxd

# 3. whisper.cppをビルドしモデルをダウンロード
make setup

# 4. バイナリをビルド
make

# 5. 実行
./opons-voxd
```

---

## ライセンス

すべてMITライセンスです：Whisperモデル（OpenAI）、whisper.cpp（Georgi Gerganov）、および本プロジェクト。

詳細なドキュメントは[メインREADME](../README.md)をご覧ください。

---

## コーディングスタイル

本プロジェクトは **Linuxカーネルコーディングスタイル**（`Documentation/process/coding-style.rst`）に従い、Olivier Pons による調整を加えています：K&R波括弧、4スペースインデント、typedef なしの `struct name`、人為的プレフィックスなし（`s_`、`t_`、`e_`）、括弧なしの `return value;`、kernel-docコメント、1行最大80文字。詳細は[メインREADME](../README.md)をご覧ください。

## Push-to-talk hotkey

The generated `~/.config/opons-voxd.conf` uses `ctrl+alt+w` (recommended on AZERTY) or `ctrl+alt+z` (recommended on QWERTY). The key sits next to `A`, so the whole combo is reachable with the left hand alone, and isn't bound by default in most desktop environments. Configure with `OPONS_VOXD_PTT_HOTKEY`. See the [main README](../README.md#push-to-talk-hotkey) for full details.


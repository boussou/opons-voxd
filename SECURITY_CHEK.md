# security analysis

---
(please remove it after reading)

## Security Analysis: opons-voxd

### Overview
A single-C-file (~1873 lines) Linux desktop daemon for local speech-to-text dictation. Captures audio via PortAudio, transcribes with whisper.cpp (local Whisper model), outputs text to clipboard or types at cursor via X11 XTest. No network connections whatsoever — confirmed by code review (no socket calls, no HTTP libraries).

---

### 🔴 Critical / High Severity

**1. Symlink attack on lock file (CVE-worthy)**
- **Location:** `opons_voxd.c:1632` — `acquire_instance_lock()`
- **Issue:** Lock file at `/tmp/opons-voxd-$UID.lock` is opened with `O_CREAT | O_CLOEXEC` but **missing `O_NOFOLLOW`**. An attacker who can predict the UID-based path and create a symlink before the app starts could redirect writes to an arbitrary file.
- **Fix:** Add `O_NOFOLLOW` flag: `open(path, O_RDWR | O_CREAT | O_NOFOLLOW | O_CLOEXEC, 0600)`

**2. X11 thread-safety race condition**
- **Location:** `opons_voxd.c:570` (`type_text`) called from transcribe thread; GTK main loop also uses the same `Display*`
- **Issue:** Xlib is **not thread-safe** for concurrent operations on the same `Display` pointer. The transcribe thread calls `type_text()` (which makes XTest/XChangeKeyboardMapping calls) while the GTK main loop is still running and may also make X11 calls (e.g., tray icon updates, notifications). This is undefined behavior and could cause crashes or corrupted state.
- **Mitigation:** Serialize all X11 calls through the main thread using `g_idle_add`/`g_main_context_invoke`, or use a mutex around all X11 calls on `g_app.xdpy`.

---

### 🟡 Medium Severity

**3. PATH injection via `execlp("xclip")`**
- **Location:** `opons_voxd.c:323` — `copy_to_selection()`
- **Issue:** Uses `execlp("xclip", ...)` which resolves via PATH. If an attacker can inject a malicious `xclip` binary into the user's PATH, it will be executed with the user's privileges. The child inherits all environment variables including `OPONS_VOXD_*`.
- **Fix:** Use `execl("/usr/bin/xclip", ...)` with an absolute path, or at minimum use `execle()` to clear/override PATH.

**4. No checksum verification on dependencies**
- **Location:** `Makefile:89,104-125`
- **Issue:** Three external downloads with no integrity verification:
  - `curl` for portaudio.h (no checksum)
  - `git clone` for whisper.cpp (no pinned commit, no verification)
  - `download-ggml-model.sh` for Whisper model (no checksum)
- **Fix:** Pin whisper.cpp to a specific commit SHA. Verify portaudio.h download with a known hash.

**5. Hardcoded path in `launch.sh` (resolved — file removed)**
- **Location:** formerly `launch.sh:6` — `cd /home/olivier/opons-voxd`
- **Issue:** Was hardcoded to a specific user's home directory. If copied verbatim, the daemon silently fails (cd to nonexistent dir) or could be manipulated if `/home/olivier` is a symlink. Not exploitable but confusing for deployment.
- **Status:** Resolved — `launch.sh` has been removed; the binary is run directly and configuration now lives in `~/.config/opons-voxd.conf`.

---

### 🟢 Low Severity / Informational

**6. Commands file path unrestricted**
- **Location:** `opons_voxd.c:763` — `load_commands()`
- **Issue:** `OPONS_VOXD_CMDS_FILE` accepts any path with no validation. An attacker who can set this env var could cause the app to read arbitrary files (though only for parsing, not execution).

**7. Silent hotkey grab failure**
- **Location:** `opons_voxd.c:1449` — `grab_ptt_hotkey()`
- **Issue:** XGrabKey failures (BadAccess if another client holds the grab) are silently swallowed. The hotkey may not work without any indication to the user.

**8. Unencrypted audio in memory**
- Audio buffer (`g_app.audio_buf`) holds raw PCM data in RAM. While this is acceptable for a local dictation tool, it means audio is readable via `/proc/$PID/mem` if the attacker has root access. Not a practical concern for this threat model.

---

### ✅ Positive Findings (Security Strengths)

| Area | Assessment |
|---|---|
| **Network isolation** | Zero network calls — confirmed by full code review. Audio never leaves the machine. |
| **Audio on disk** | Never written to disk — only held in RAM during recording/transcription. |
| **Atomic state** | `g_app.state` and `g_app.audio_len` use `_Atomic` types correctly. |
| **GTK main loop** | All UI updates go through `g_idle_add` — correct GTK threading pattern. |
| **Memory safety** | All `malloc`/`realloc` calls check for NULL. Buffer overflows guarded (e.g., `str_replace_alloc` has SIZE_MAX overflow check). |
| **Input bounds** | All env var copies use bounded operations (`strncpy` + null, `snprintf`). |
| **Single instance** | Proper flock-based singleton lock prevents duplicate instances. |
| **X error handling** | Custom Xlib error handler prevents crashes on protocol errors — app stays alive. |
| **No shell injection** | Voice commands pipeline has no `system()`/`popen()` calls — replacements are typed via XTest, not executed. |
| **Permissions** | Lock file created with `0600`. `.gitignore` excludes logs and binaries. |

---

### Recommended Fixes (Priority Order)

1. **Add `O_NOFOLLOW` to lock file open** — one-line fix, eliminates symlink attack
2. **Serialize X11 calls from transcribe thread** — move `type_text()` to main thread via `g_main_context_invoke`
3. **Use absolute path for xclip** — change `execlp("xclip", ...)` to `execl("/usr/bin/xclip", ...)`
4. **Pin whisper.cpp commit** in Makefile instead of `--depth 1` HEAD clone
5. **Add `O_NOFOLLOW` to lock file** — this is the highest-impact, lowest-effort fix
# opons-voxd

> Note: opons-voxd is unrelated to jakovius/voxd. The name was chosen independently — `opons` = O. Pons (author), `voxd` = vox + daemon (Latin/Unix tradition).

> **⚠ Backwards compatibility intentionally broken.** This project was previously `voice_in_linux`. The rename touched the binary name, the source filename, every environment variable (`VOICE_IN_*` → `OPONS_VOXD_*`), and the runtime dependency on `xdotool` (replaced by direct XTest synthesis via `libxtst-dev`). No aliases — update your launch scripts, autostart entries, and `apt install` lines.

**Lokale Sprache-zu-Text-Diktierlösung für Linux — Ihre Stimme verlässt niemals Ihren Rechner.**

🌍 **Sprachen:**
[English](../README.md) · [Français](README.fr.md) · [中文](README.zh.md) · [日本語](README.ja.md) · [Español](README.es.md) · [Italiano](README.it.md)

---

## Warum?

Ich wollte Text in jede Linux-Anwendung diktieren können, ohne meine Stimme an einen Drittanbieter-Server zu senden. Jede bestehende Lösung — Browser-Erweiterungen, Cloud-APIs, SaaS-Plattformen — streamt Ihr Audio an entfernte Server zur Transkription. Das bedeutet, dass jedes gesprochene Wort über das Internet übertragen wird: E-Mails, vertrauliche Dokumente, Kundendaten, persönliche Notizen — alles.

Dieses Projekt ist eine leichtgewichtige, vollständig lokale Alternative. Es läuft ausschließlich auf Ihrem Rechner mit [whisper.cpp](https://github.com/ggerganov/whisper.cpp), OpenAIs Open-Source-Spracherkennungsmodell. Nach der Ersteinrichtung ist keine Netzwerkverbindung erforderlich. Keine Daten verlassen jemals Ihren Computer.

Erstellt mit Hilfe von künstlicher Intelligenz.

---


### Screenshots

| Inaktiv | Aufnahme |
|:---:|:---:|
| ![Inaktiv](../screenshots/opons_voxd_inactive.png) | ![Aufnahme](../screenshots/opons_voxd_active.png) |

## Funktionsweise

1. Ein kleines Symbol erscheint im System-Tray
2. **Linksklick** → Aufnahme startet (Symbol wird rot)
3. **Erneuter Linksklick** → Aufnahme stoppt, Audio wird lokal via Whisper transkribiert
4. Der transkribierte Text wird automatisch in die Zwischenablage kopiert (beide X11-Selectionen)
5. Eine Desktop-Benachrichtigung zeigt den Text 10 Sekunden lang an
6. Einfügen mit `Ctrl+Shift+V`, `Shift+Insert` oder Mittelklick

---


### Screenshots

| Inaktiv | Aufnahme |
|:---:|:---:|
| ![Inaktiv](../screenshots/opons_voxd_inactive.png) | ![Aufnahme](../screenshots/opons_voxd_active.png) |

## Funktionen

- **100% lokal** — Audio wird auf CPU oder GPU verarbeitet, nie versendet
- **GPU-beschleunigt** — NVIDIA CUDA-Unterstützung für nahezu sofortige Transkription
- **99 Sprachen** — unterstützt durch OpenAI Whisper

> ⚠️ **NVIDIA GPU mit CUDA ist entscheidend für akzeptable Antwortzeiten.**
> Ohne CUDA: **10–15 Sekunden** Verarbeitung pro 15 Sekunden Sprache.
> Mit CUDA: **unter 1 Sekunde**. Wenn Sie eine NVIDIA GPU haben (GTX 1060+), sollte die CUDA-Einrichtung höchste Priorität haben.
- **Leichtgewichtig** — einzelne C-Binärdatei (~100 KB), kein Python nötig
- **System-Tray-Integration** — unauffälliges Symbol in der Taskleiste
- **Doppelte Zwischenablage** — Text in PRIMARY und CLIPBOARD
- **Sprachbefehle** — Befehlsdateien pro Sprache in `commands/`, standardmäßig deaktiviert (`OPONS_VOXD_COMMANDS=1`). Neue Sprache: einfach `commands/xx.txt` erstellen, keine Neukompilierung nötig.
- **Transiente Benachrichtigungen** — erscheinen und verschwinden vollständig. `OPONS_VOXD_NOTIFY_PERSIST=1` um sie im Verlauf zu behalten.
- **Notification mode** — `OPONS_VOXD_NOTIFY=normal|quiet|silent|off` controls success notifications (default `quiet`: bubble, no sound). Errors always show with sound, in every mode, without exception. See main README for the full table.
- **Automatische Großschreibung** — Sätze werden automatisch kapitalisiert
- **Autostart** — beim Login konfigurierbar

---

## Installation

```bash
# 1. Systemabhängigkeiten
sudo apt update
sudo apt install -y \
    build-essential cmake pkg-config git \
    libgtk-3-dev libnotify-dev libportaudio-dev libcairo2-dev libxtst-dev \
    xclip libnotify-bin

# 2. Repository klonen
git clone https://github.com/olivierpons/opons-voxd.git
cd opons-voxd

# 3. whisper.cpp kompilieren + Modell herunterladen
make setup

# 4. Binärdatei kompilieren
make

# 5. Starten
./opons-voxd
```

---

## Lizenz

Alles unter MIT-Lizenz: Whisper-Modell (OpenAI), whisper.cpp (Georgi Gerganov) und dieses Projekt.

Vollständige Dokumentation: siehe [Haupt-README](../README.md).

---

## Codierungsstil

Dieses Projekt folgt dem **Linux-Kernel-Codierungsstil** (`Documentation/process/coding-style.rst`) mit Anpassungen von Olivier Pons: K&R-Klammern, 4 Leerzeichen Einrückung, `struct name` ohne Typedef, keine Präfixe (`s_`, `t_`, `e_`), `return value;` ohne Klammern, kernel-doc-Kommentare, maximal 80 Zeichen pro Zeile. Vollständige Details im [Haupt-README](../README.md).

## Push-to-talk hotkey

The generated `~/.config/opons-voxd.conf` uses `ctrl+alt+w` (recommended on AZERTY) or `ctrl+alt+z` (recommended on QWERTY). The key sits next to `A`, so the whole combo is reachable with the left hand alone, and isn't bound by default in most desktop environments. Configure with `OPONS_VOXD_PTT_HOTKEY`. See the [main README](../README.md#push-to-talk-hotkey) for full details.


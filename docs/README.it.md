# opons-voxd

> Note: opons-voxd is unrelated to jakovius/voxd. The name was chosen independently — `opons` = O. Pons (author), `voxd` = vox + daemon (Latin/Unix tradition).

> **⚠ Backwards compatibility intentionally broken.** This project was previously `voice_in_linux`. The rename touched the binary name, the source filename, every environment variable (`VOICE_IN_*` → `OPONS_VOXD_*`), and the runtime dependency on `xdotool` (replaced by direct XTest synthesis via `libxtst-dev`). No aliases — update your launch scripts, autostart entries, and `apt install` lines.

**Dettatura vocale 100% locale per Linux — la vostra voce non lascia mai il vostro computer.**

🌍 **Lingue:**
[English](../README.md) · [Français](README.fr.md) · [Deutsch](README.de.md) · [中文](README.zh.md) · [日本語](README.ja.md) · [Español](README.es.md)

---

## Perché?

Volevo poter dettare testo in qualsiasi applicazione Linux senza inviare la mia voce a un server di terze parti. Tutte le soluzioni esistenti — estensioni del browser, API cloud, piattaforme SaaS — trasmettono il vostro audio a server remoti per la trascrizione. Ciò significa che ogni parola pronunciata transita attraverso Internet: email, documenti riservati, dati dei clienti, note personali — tutto.

Questo progetto è un'alternativa leggera e completamente locale. Funziona interamente sulla vostra macchina utilizzando [whisper.cpp](https://github.com/ggerganov/whisper.cpp), il modello di riconoscimento vocale open source di OpenAI. Non è necessaria alcuna connessione di rete dopo l'installazione iniziale. Nessun dato lascia mai il vostro computer.

Sviluppato con l'aiuto dell'intelligenza artificiale.

---

## Come funziona

1. Un'icona appare nel system tray
2. **Clic sinistro** → inizia la registrazione (l'icona diventa rossa)
3. **Clic sinistro di nuovo** → stop, l'audio viene trascritto localmente con Whisper
4. Il testo trascritto viene copiato automaticamente negli appunti (entrambe le selezioni X11)
5. Una notifica desktop mostra il testo per 10 secondi
6. Incollate con `Ctrl+Shift+V`, `Shift+Insert` o clic centrale

---


### Screenshots

| Inattivo | Registrazione |
|:---:|:---:|
| ![Inattivo](../screenshots/opons_voxd_inactive.png) | ![Registrazione](../screenshots/opons_voxd_active.png) |

## Caratteristiche

- **100% locale** — l'audio viene elaborato su CPU o GPU, mai inviato
- **Accelerazione GPU** — supporto NVIDIA CUDA per trascrizione quasi istantanea
- **99 lingue** — basato su OpenAI Whisper

> ⚠️ **NVIDIA GPU con CUDA è fondamentale per tempi di risposta accettabili.**
> Senza CUDA: **10–15 secondi** di elaborazione per 15 secondi di parlato.
> Con CUDA: **meno di 1 secondo**. Se avete una GPU NVIDIA (GTX 1060+), configurare CUDA deve essere la priorità assoluta.
- **Leggero** — singolo binario C (~100 KB), senza Python
- **Integrazione nel system tray** — icona discreta nella barra delle applicazioni
- **Doppi appunti** — testo copiato in PRIMARY e CLIPBOARD
- **Comandi vocali** — file di comandi per lingua in `commands/`, disabilitati di default (`OPONS_VOXD_COMMANDS=1`). Per aggiungere una lingua, creare `commands/xx.txt` — nessuna ricompilazione necessaria.
- **Notifiche transitorie** — appaiono e scompaiono completamente. `OPONS_VOXD_NOTIFY_PERSIST=1` per mantenerle nella cronologia.
- **Notification mode** — `OPONS_VOXD_NOTIFY=normal|quiet|silent|off` controls success notifications (default `quiet`: bubble, no sound). Errors always show with sound, in every mode, without exception. See main README for the full table.
- **Maiuscole automatiche** — le frasi vengono capitalizzate automaticamente
- **Avvio automatico** — configurabile all'accesso

---

## Installazione

```bash
# 1. Dipendenze di sistema
sudo apt update
sudo apt install -y \
    build-essential cmake pkg-config git \
    libgtk-3-dev libnotify-dev libportaudio-dev libcairo2-dev libxtst-dev \
    xclip libnotify-bin

# 2. Clonare il repository
git clone https://github.com/olivierpons/opons-voxd.git
cd opons-voxd

# 3. Compilare whisper.cpp e scaricare il modello
make setup

# 4. Compilare il binario
make

# 5. Avviare
./opons-voxd
```

---

## Licenza

Tutto sotto licenza MIT: modello Whisper (OpenAI), whisper.cpp (Georgi Gerganov) e questo progetto.

Documentazione completa: vedere il [README principale](../README.md).

---

## Stile di codifica

Questo progetto segue lo **stile di codifica del kernel Linux** (`Documentation/process/coding-style.rst`) con adattamenti di Olivier Pons: parentesi graffe K&R, indentazione a 4 spazi, `struct name` senza typedef, nessun prefisso artificiale (`s_`, `t_`, `e_`), `return value;` senza parentesi, commenti kernel-doc, massimo 80 caratteri per riga. Dettagli completi nel [README principale](../README.md).

## Push-to-talk hotkey

The generated `~/.config/opons-voxd.conf` uses `ctrl+alt+w` (recommended on AZERTY) or `ctrl+alt+z` (recommended on QWERTY). The key sits next to `A`, so the whole combo is reachable with the left hand alone, and isn't bound by default in most desktop environments. Configure with `OPONS_VOXD_PTT_HOTKEY`. See the [main README](../README.md#push-to-talk-hotkey) for full details.


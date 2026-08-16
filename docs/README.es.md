# opons-voxd

> Note: opons-voxd is unrelated to jakovius/voxd. The name was chosen independently — `opons` = O. Pons (author), `voxd` = vox + daemon (Latin/Unix tradition).

> **⚠ Backwards compatibility intentionally broken.** This project was previously `voice_in_linux`. The rename touched the binary name, the source filename, every environment variable (`VOICE_IN_*` → `OPONS_VOXD_*`), and the runtime dependency on `xdotool` (replaced by direct XTest synthesis via `libxtst-dev`). No aliases — update your launch scripts, autostart entries, and `apt install` lines.

**Dictado de voz a texto 100% local para Linux — su voz nunca sale de su máquina.**

🌍 **Idiomas:**
[English](../README.md) · [Français](README.fr.md) · [Deutsch](README.de.md) · [中文](README.zh.md) · [日本語](README.ja.md) · [Italiano](README.it.md)

---

## ¿Por qué?

Quería poder dictar texto en cualquier aplicación de Linux sin enviar mi voz a un servidor externo. Todas las soluciones existentes — extensiones de navegador, APIs en la nube, plataformas SaaS — transmiten su audio a servidores remotos para la transcripción. Esto significa que cada palabra que pronuncia transita por Internet: correos electrónicos, documentos confidenciales, datos de clientes, notas personales — todo.

Este proyecto es una alternativa ligera y completamente local. Se ejecuta íntegramente en su máquina utilizando [whisper.cpp](https://github.com/ggerganov/whisper.cpp), el modelo de reconocimiento de voz de código abierto de OpenAI. No se necesita conexión a Internet después de la instalación inicial. Ningún dato sale de su ordenador.

Desarrollado con la ayuda de inteligencia artificial.

---

## Funcionamiento

1. Un icono aparece en la bandeja del sistema
2. **Clic izquierdo** → comienza la grabación (el icono se vuelve rojo)
3. **Clic izquierdo de nuevo** → se detiene, el audio se transcribe localmente con Whisper
4. El texto transcrito se copia automáticamente al portapapeles (ambas selecciones X11)
5. Una notificación muestra el texto durante 10 segundos
6. Pegue con `Ctrl+Shift+V`, `Shift+Insert` o clic central

---


### Screenshots

| Inactivo | Grabando |
|:---:|:---:|
| ![Inactivo](../screenshots/opons_voxd_inactive.png) | ![Grabando](../screenshots/opons_voxd_active.png) |

## Características

- **100% local** — el audio se procesa en CPU o GPU, nunca se envía
- **Aceleración GPU** — soporte NVIDIA CUDA para transcripción casi instantánea
- **99 idiomas** — impulsado por OpenAI Whisper

> ⚠️ **NVIDIA GPU con CUDA es crítico para tiempos de respuesta aceptables.**
> Sin CUDA: **10–15 segundos** de procesamiento por cada 15 segundos de habla.
> Con CUDA: **menos de 1 segundo**. Si tiene una GPU NVIDIA (GTX 1060+), configurar CUDA debe ser su máxima prioridad.
- **Ligero** — binario C único (~100 KB), sin Python ni dependencias de ejecución
- **Integración en la bandeja** — icono discreto en la barra de tareas
- **Doble portapapeles** — texto copiado en PRIMARY y CLIPBOARD
- **Comandos de voz** — archivos de comandos por idioma en `commands/`, desactivados por defecto (`OPONS_VOXD_COMMANDS=1`). Para añadir un idioma, cree `commands/xx.txt` — sin recompilar.
- **Notificaciones transitorias** — aparecen y desaparecen completamente. `OPONS_VOXD_NOTIFY_PERSIST=1` para mantenerlas en el historial.
- **Notification mode** — `OPONS_VOXD_NOTIFY=normal|quiet|silent|off` controls success notifications (default `quiet`: bubble, no sound). Errors always show with sound, in every mode, without exception. See main README for the full table.
- **Mayúsculas automáticas** — las oraciones se capitalizan automáticamente
- **Inicio automático** — configurable al iniciar sesión

---

## Instalación

```bash
# 1. Dependencias del sistema
sudo apt update
sudo apt install -y \
    build-essential cmake pkg-config git \
    libgtk-3-dev libnotify-dev libportaudio-dev libcairo2-dev libxtst-dev \
    xclip libnotify-bin

# 2. Clonar el repositorio
git clone https://github.com/olivierpons/opons-voxd.git
cd opons-voxd

# 3. Compilar whisper.cpp y descargar el modelo
make setup

# 4. Compilar el binario
make

# 5. Ejecutar
./opons-voxd
```

---

## Licencia

Todo bajo licencia MIT: modelo Whisper (OpenAI), whisper.cpp (Georgi Gerganov) y este proyecto.

Documentación completa: ver el [README principal](../README.md).

---

## Estilo de código

Este proyecto sigue el **estilo de codificación del kernel Linux** (`Documentation/process/coding-style.rst`) con adaptaciones de Olivier Pons: llaves K&R, indentación de 4 espacios, `struct name` sin typedef, sin prefijos artificiales (`s_`, `t_`, `e_`), `return value;` sin paréntesis, comentarios kernel-doc, máximo 80 caracteres por línea. Detalles completos en el [README principal](../README.md).

## Push-to-talk hotkey

The generated `~/.config/opons-voxd.conf` uses `ctrl+alt+w` (recommended on AZERTY) or `ctrl+alt+z` (recommended on QWERTY). The key sits next to `A`, so the whole combo is reachable with the left hand alone, and isn't bound by default in most desktop environments. Configure with `OPONS_VOXD_PTT_HOTKEY`. See the [main README](../README.md#push-to-talk-hotkey) for full details.


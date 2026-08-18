// SPDX-License-Identifier: MIT
/*
 * opons_voxd.c - System-tray push-to-talk dictation for Linux (X11).
 *
 * Copyright (C) 2026 Olivier Pons
 *
 * Two activation modes:
 *   1. Tray icon (toggle): left-click starts recording, click again
 *      stops it. Transcript is copied to PRIMARY + CLIPBOARD via xclip.
 *   2. Push-to-talk hotkey: hold the configured key combo (default
 *      ctrl+shift+space) to record, release to stop. Transcript is
 *      typed at the keyboard cursor via the X11 XTest extension.
 *      A second, independently configurable hotkey does the same
 *      but runs Whisper's translate task instead of plain
 *      transcription, so speech in the configured source language
 *      is typed out in English (Whisper always translates to
 *      English; there is no other target language).
 *
 * Audio is captured via PortAudio, transcribed locally with
 * whisper.cpp, and shown as a desktop notification through libnotify.
 *
 * Environment variables:
 *   OPONS_VOXD_MODEL       path to ggml model
 *   OPONS_VOXD_TRANSLATE_MODEL   path to the ggml model used for
 *                            the translate hotkey (default: same
 *                            as OPONS_VOXD_MODEL's default). Speed-
 *                            oriented models such as large-v3-turbo
 *                            were not trained on the translate task
 *                            and translate poorly, so this is
 *                            intentionally a separate, independently
 *                            configurable model. When it resolves to
 *                            the same path as OPONS_VOXD_MODEL, only
 *                            one model is loaded and both hotkeys
 *                            share it.
 *   OPONS_VOXD_LANGUAGE    ISO code or "auto" (default: "fr")
 *   OPONS_VOXD_DEVICE      PortAudio device index
 *   OPONS_VOXD_COMMANDS        "1" to enable voice commands
 *   OPONS_VOXD_CMDS_FILE       explicit path to commands file
 *   OPONS_VOXD_NOTIFY_PERSIST  "1" to keep notifications in history
 *   OPONS_VOXD_NOTIFY          "normal" | "quiet" (default) |
 *                              "silent" | "off" — applies to
 *                              success notifications only; errors
 *                              are ALWAYS shown with sound
 *   OPONS_VOXD_PTT_HOTKEY      push-to-talk hotkey spec, e.g.
 *                            "ctrl+shift+space" (default), "super+space"
 *   OPONS_VOXD_PTT_TRANSLATE_HOTKEY   push-to-talk-and-translate
 *                            hotkey spec, same syntax as
 *                            OPONS_VOXD_PTT_HOTKEY (default:
 *                            "ctrl+alt+t"). Speech is translated
 *                            to English instead of transcribed
 *                            verbatim.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <libnotify/notify.h>
#include <portaudio.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "whisper.h"

/* ---- defines ---- */

#ifndef M_PI
#define M_PI                3.14159265358979323846
#endif

#define SAMPLE_RATE         16000
#define NUM_CHANNELS        1
#define FRAMES_PER_BUFFER   1024
#define MAX_RECORD_SEC      600
#define AUDIO_CAPACITY      ((size_t)SAMPLE_RATE * MAX_RECORD_SEC)
#define ICON_PX             22
#define NOTIFY_TIMEOUT_MS   10000
#define WHISPER_THREADS     8
#define MIN_AUDIO_SAMPLES   (SAMPLE_RATE / 4)
#define CMD_LINE_MAX        1024
#define CMD_INITIAL_CAP     32

#define DEFAULT_MODEL       "whisper.cpp/models/ggml-medium.bin"
#define DEFAULT_TRANSLATE_MODEL  "whisper.cpp/models/ggml-medium.bin"
#define DEFAULT_LANG        "fr"
#define CMDS_DIR            "commands"
#define DEFAULT_PTT_HOTKEY  "ctrl+shift+space"
#define DEFAULT_PTT_TRANSLATE_HOTKEY  "ctrl+alt+t"
#define HOTKEY_TOKENS_MAX   8

/* ---- types ---- */

enum app_state {
    STATE_IDLE,
    STATE_RECORDING,
    STATE_PROCESSING,
};

enum notify_mode {
    NOTIFY_NORMAL,  /* bubble + sound */
    NOTIFY_QUIET,   /* bubble, suppressed sound (default) */
    NOTIFY_SILENT,  /* bubble, suppressed sound, low urgency */
    NOTIFY_OFF,     /* no bubble at all */
};

struct state_msg {
    enum app_state state;
};

struct notify_msg {
    char *title;
    char *body;
    bool is_error;
};

struct voice_cmd {
    char *spoken;
    char *replacement;
};

/* One grabbed X11 key combo: either the plain PTT hotkey or the
 * PTT-translate hotkey. Both are handled through the same grab/
 * ungrab/match logic, parameterized on one of these. */
struct ptt_binding {
    unsigned int mods;
    unsigned int keycode;
    bool grabbed;
};

struct app {
    GtkStatusIcon           *status_icon;
    GdkPixbuf               *icon_idle;
    GdkPixbuf               *icon_recording;
    GdkPixbuf               *icon_processing;
    GtkWidget               *menu;
    PaStream                *stream;
    int                     input_device;
    float                   *audio_buf;
    atomic_size_t           audio_len;
    struct whisper_context   *wctx;
    struct whisper_context   *wctx_translate;
    char                    lang[16];
    bool                    commands_on;
    bool                    notify_persist;
    enum notify_mode        notify_mode;
    struct voice_cmd        *commands;
    int                     cmd_count;
    int                     cmd_cap;
    _Atomic enum app_state  state;
    Display                 *xdpy;
    struct ptt_binding      ptt;
    struct ptt_binding      ptt_translate;
    bool                    ptt_filter_installed;
    bool                    via_hotkey;
    bool                    translate_active;
    unsigned int            active_ptt_keycode;
};

/* ---- static prototypes ---- */

static GdkPixbuf *make_icon(double r, double g, double b,
                            gboolean filled);
static void set_state(enum app_state state);
static gboolean set_state_cb(gpointer data);
static void request_state(enum app_state state);
static void copy_to_selection(const char *text,
                              const char *sel);
static void copy_to_clipboards(const char *text);
static int type_text(const char *text);
static int utf8_decode(const unsigned char *s, uint32_t *cp);
static KeySym codepoint_to_keysym(uint32_t cp);
static int find_free_keycode(Display *dpy);
static int keysym_index_for_keycode(Display *dpy,
                                    int keycode,
                                    KeySym ks);
static int synthesize_keysym(Display *dpy, KeySym ks,
                             int free_kc);
static gboolean notify_cb(gpointer data);
static void request_notify(const char *title,
                           const char *body,
                           bool is_error);
static void process_escapes(char *s);
static int cmd_cmp_len_desc(const void *a, const void *b);
static void load_commands(void);
static void free_commands(void);
static char *str_replace_alloc(const char *in,
                               const char *needle,
                               const char *repl);
static void str_lower_ascii(char *s);
static char *apply_voice_cmds(const char *text);
static void capitalize_sentences(char *text);
static int audio_cb(const void *in, void *out,
                    unsigned long count,
                    const PaStreamCallbackTimeInfo *ti,
                    PaStreamCallbackFlags flags,
                    void *udata);
static int audio_start(void);
static void audio_stop(void);
static char *run_whisper(const float *samples,
                         size_t n_samples,
                         bool translate);
static void *transcribe_thread(void *arg);
static void on_activate(GtkStatusIcon *icon, gpointer data);
static void on_toggle(GtkMenuItem *item, gpointer data);
static void on_quit(GtkMenuItem *item, gpointer data);
static void on_popup(GtkStatusIcon *icon, guint btn,
                     guint time, gpointer data);
static void rec_start(void);
static void rec_stop(void);
static void notify_critical(const char *title, const char *body);
static int acquire_instance_lock(void);
static int init_whisper(void);
static void init_whisper_translate(void);
static void init_lang(void);
static void init_device(void);
static void init_options(void);
static void build_menu(void);
static void build_tray(void);
static int parse_hotkey(const char *spec, unsigned int *mods,
                        KeySym *ks);
static int x_silent_error_handler(Display *dpy, XErrorEvent *ev);
static void grab_hotkey_combo(struct ptt_binding *hk,
                              unsigned int extra);
static void ungrab_hotkey_combo(struct ptt_binding *hk,
                                unsigned int extra);
static int resolve_ptt_hotkey(const char *spec,
                              struct ptt_binding *hk);
static int open_ptt_display(void);
static void grab_ptt_hotkey(struct ptt_binding *hk);
static void ungrab_ptt_hotkey(struct ptt_binding *hk);
static void init_hotkey(void);
static void free_hotkey(void);
static GdkFilterReturn ptt_event_filter(GdkXEvent *xev,
                                        GdkEvent *gev,
                                        gpointer data);

/* ---- global state ---- */

static struct app g_app;
static int g_lock_fd = -1;

/* ---- icons ---- */

/**
 * make_icon - Render a colored circle as a GdkPixbuf.
 * @r: red component (0.0 to 1.0).
 * @g: green component (0.0 to 1.0).
 * @b: blue component (0.0 to 1.0).
 * @filled: TRUE for solid fill, FALSE for outline only.
 *
 * Return: newly allocated GdkPixbuf (caller owns the ref).
 */
static GdkPixbuf *make_icon(double r, double g, double b,
                            gboolean filled)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    GdkPixbuf *pb;
    double cx = ICON_PX / 2.0;
    double radius = cx - 2.0;

    surface = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, ICON_PX, ICON_PX);
    cr = cairo_create(surface);
    cairo_set_source_rgba(cr, r, g, b, 1.0);
    cairo_set_line_width(cr, 2.5);
    cairo_arc(cr, cx, cx, radius, 0, 2 * M_PI);
    if (filled)
        cairo_fill(cr);
    else
        cairo_stroke(cr);
    cairo_destroy(cr);
    pb = gdk_pixbuf_get_from_surface(
        surface, 0, 0, ICON_PX, ICON_PX);
    cairo_surface_destroy(surface);
    return pb;
}

/* ---- state management ---- */

static void set_state(enum app_state state)
{
    GdkPixbuf *pb;

    atomic_store(&g_app.state, state);
    pb = g_app.icon_idle;
    if (state == STATE_RECORDING)
        pb = g_app.icon_recording;
    else if (state == STATE_PROCESSING)
        pb = g_app.icon_processing;
    gtk_status_icon_set_from_pixbuf(g_app.status_icon, pb);
}

static gboolean set_state_cb(gpointer data)
{
    struct state_msg *msg = data;

    set_state(msg->state);
    g_free(msg);
    return G_SOURCE_REMOVE;
}

static void request_state(enum app_state state)
{
    struct state_msg *msg;

    msg = g_new(struct state_msg, 1);
    msg->state = state;
    g_idle_add(set_state_cb, msg);
}

/* ---- clipboard / notifications ---- */

/**
 * copy_to_selection - Pipe text into xclip for one X11 selection.
 * @text: NUL-terminated string to copy.
 * @sel: "primary" or "clipboard".
 */
static void copy_to_selection(const char *text, const char *sel)
{
    int pipefd[2];
    pid_t pid;
    size_t len;
    const char *p;
    ssize_t w;

    if (pipe(pipefd) != 0)
        return;
    pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }
    if (pid == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execlp("xclip", "xclip", "-selection",
               sel, (char *)NULL);
        _exit(127);
    }
    close(pipefd[0]);
    len = strlen(text);
    p = text;
    while (len > 0) {
        w = write(pipefd[1], p, len);
        if (w <= 0)
            break;
        p += w;
        len -= (size_t)w;
    }
    close(pipefd[1]);
    waitpid(pid, NULL, 0);
}

static void copy_to_clipboards(const char *text)
{
    copy_to_selection(text, "primary");
    copy_to_selection(text, "clipboard");
}

/**
 * utf8_decode - Decode a single UTF-8 sequence into a codepoint.
 * @s: pointer to the next byte to decode.
 * @cp: out, decoded codepoint.
 *
 * Return: number of bytes consumed (1-4) on success, 0 on NUL,
 * -1 on a malformed sequence.
 */
static int utf8_decode(const unsigned char *s, uint32_t *cp)
{
    if (s[0] == 0) {
        *cp = 0;
        return 0;
    }
    if (s[0] < 0x80) {
        *cp = s[0];
        return 1;
    }
    if ((s[0] & 0xE0) == 0xC0 &&
        (s[1] & 0xC0) == 0x80) {
        *cp = ((uint32_t)(s[0] & 0x1F) << 6) |
              (s[1] & 0x3F);
        return 2;
    }
    if ((s[0] & 0xF0) == 0xE0 &&
        (s[1] & 0xC0) == 0x80 &&
        (s[2] & 0xC0) == 0x80) {
        *cp = ((uint32_t)(s[0] & 0x0F) << 12) |
              ((uint32_t)(s[1] & 0x3F) << 6) |
              (s[2] & 0x3F);
        return 3;
    }
    if ((s[0] & 0xF8) == 0xF0 &&
        (s[1] & 0xC0) == 0x80 &&
        (s[2] & 0xC0) == 0x80 &&
        (s[3] & 0xC0) == 0x80) {
        *cp = ((uint32_t)(s[0] & 0x07) << 18) |
              ((uint32_t)(s[1] & 0x3F) << 12) |
              ((uint32_t)(s[2] & 0x3F) << 6) |
              (s[3] & 0x3F);
        return 4;
    }
    return -1;
}

/**
 * codepoint_to_keysym - Map a Unicode codepoint to an X11 KeySym.
 * @cp: Unicode codepoint.
 *
 * Newline and tab map to XK_Return and XK_Tab respectively. ASCII
 * and Latin-1 (U+0020..U+00FF) map directly to themselves. Any
 * higher codepoint uses the X11 Unicode-keysym convention
 * (0x01000000 | codepoint).
 *
 * Return: the KeySym to synthesize.
 */
static KeySym codepoint_to_keysym(uint32_t cp)
{
    if (cp == '\n')
        return XK_Return;
    if (cp == '\t')
        return XK_Tab;
    if (cp < 0x100)
        return cp;
    return cp | 0x01000000;
}

/**
 * find_free_keycode - Find a keycode whose every index is NoSymbol.
 * @dpy: X display.
 *
 * Walks the keyboard mapping from the highest keycode downward and
 * returns the first one that has no keysyms bound to it. This slot
 * is later used to temporarily bind keysyms that are not on the
 * physical layout, so XTestFakeKeyEvent can synthesize them.
 *
 * Return: keycode on success, -1 if no free slot is available.
 */
static int find_free_keycode(Display *dpy)
{
    int min_kc;
    int max_kc;
    int per;
    KeySym *km;
    int kc;
    int i;
    int found = -1;

    XDisplayKeycodes(dpy, &min_kc, &max_kc);
    km = XGetKeyboardMapping(
        dpy, (KeyCode)min_kc, max_kc - min_kc + 1, &per);
    if (!km)
        return -1;
    for (kc = max_kc; kc >= min_kc; --kc) {
        bool free_slot = true;

        for (i = 0; i < per; i++) {
            if (km[(kc - min_kc) * per + i] != NoSymbol) {
                free_slot = false;
                break;
            }
        }
        if (free_slot) {
            found = kc;
            break;
        }
    }
    XFree(km);
    return found;
}

/**
 * keysym_index_for_keycode - Find which shift level produces @ks.
 * @dpy: X display.
 * @keycode: keycode to inspect.
 * @ks: keysym to locate.
 *
 * Return: 0 (no modifier) or 1 (shift) if @ks is at one of those
 * shift levels for @keycode, -1 otherwise (including AltGr-level
 * mappings, which we don't synthesize directly).
 */
static int keysym_index_for_keycode(Display *dpy,
                                    int keycode, KeySym ks)
{
    KeySym *km;
    int per;
    int found = -1;
    int i;

    km = XGetKeyboardMapping(dpy, (KeyCode)keycode, 1, &per);
    if (!km)
        return -1;
    for (i = 0; i < per && i < 2; i++) {
        if (km[i] == ks) {
            found = i;
            break;
        }
    }
    XFree(km);
    return found;
}

/**
 * synthesize_keysym - Press and release the keys producing @ks.
 * @dpy: X display.
 * @ks: keysym to type.
 * @free_kc: keycode reserved for runtime remapping (-1 if none).
 *
 * Tries the existing keyboard mapping first (no shift, then shift).
 * Falls back to binding @ks to @free_kc via XChangeKeyboardMapping
 * for symbols that are not on the physical layout. Used for the
 * Latin-1 accents (é è à ç ù â ê î ô û ï ë ü ö) on layouts that
 * don't have them, and for anything else whisper might transcribe.
 *
 * Return: 0 on success, -1 on failure.
 */
static int synthesize_keysym(Display *dpy, KeySym ks, int free_kc)
{
    KeyCode kc;
    KeyCode shift_kc;
    KeySym remap[2];

    kc = XKeysymToKeycode(dpy, ks);
    if (kc != 0) {
        int idx = keysym_index_for_keycode(dpy, kc, ks);

        if (idx == 0) {
            XTestFakeKeyEvent(dpy, kc, True, 0);
            XTestFakeKeyEvent(dpy, kc, False, 0);
            XFlush(dpy);
            return 0;
        }
        if (idx == 1) {
            shift_kc = XKeysymToKeycode(dpy, XK_Shift_L);
            if (shift_kc == 0)
                return -1;
            XTestFakeKeyEvent(dpy, shift_kc, True, 0);
            XTestFakeKeyEvent(dpy, kc, True, 0);
            XTestFakeKeyEvent(dpy, kc, False, 0);
            XTestFakeKeyEvent(dpy, shift_kc, False, 0);
            XFlush(dpy);
            return 0;
        }
    }
    if (free_kc <= 0)
        return -1;
    /*
     * Pass two identical keysyms to defeat Xlib's automatic
     * lowercase/uppercase folding: when only one alphabetic keysym
     * is supplied, X treats the pair as (lower, upper), so pressing
     * with no modifier yields the lowercase form even when @ks is
     * uppercase. Setting both shift levels explicitly to @ks makes
     * the unshifted press emit @ks verbatim.
     */
    remap[0] = ks;
    remap[1] = ks;
    XChangeKeyboardMapping(dpy, free_kc, 1, remap, 2);
    XSync(dpy, False);
    XTestFakeKeyEvent(dpy, (KeyCode)free_kc, True, 0);
    XTestFakeKeyEvent(dpy, (KeyCode)free_kc, False, 0);
    XSync(dpy, False);
    return 0;
}

/**
 * type_text - Synthesize keystrokes for @text via XTest.
 * @text: NUL-terminated UTF-8 string.
 *
 * Replaces the previous fork+exec of xdotool. Uses the same X
 * display already opened for the push-to-talk grab, so no extra
 * connection or process is needed. Handles ASCII, Latin-1 accents
 * directly, and any other Unicode codepoint via temporary
 * remapping of a free keycode.
 *
 * Per-codepoint failures (e.g. a keysym that resolves nowhere even
 * after remapping) are logged on stderr so the issue is observable,
 * but do not abort the rest of the string. The caller relies on the
 * return value to decide whether to fall back to the clipboard:
 *
 * Return: 0 if at least one character was typed (or @text was empty),
 * -1 if @text was non-empty and every character failed (typically
 * because @g_app.xdpy is NULL, or every keysym was unmappable).
 */
static int type_text(const char *text)
{
    const unsigned char *p;
    uint32_t cp;
    int free_kc;
    int typed = 0;
    int failed = 0;
    KeySym ks;
    KeySym restore;
    struct timespec delay;

    if (!text || !*text)
        return 0;
    if (!g_app.xdpy) {
        fprintf(stderr,
                "type: no X display, cannot synthesize "
                "keystrokes\n");
        return -1;
    }
    free_kc = find_free_keycode(g_app.xdpy);
    delay.tv_sec = 0;
    delay.tv_nsec = 1000000;
    p = (const unsigned char *)text;
    while (*p) {
        int n = utf8_decode(p, &cp);

        if (n <= 0) {
            p++;
            continue;
        }
        ks = codepoint_to_keysym(cp);
        if (synthesize_keysym(g_app.xdpy, ks, free_kc) == 0) {
            typed++;
        } else {
            failed++;
            fprintf(stderr,
                    "type: cannot synthesize U+%04X "
                    "(keysym 0x%lx)\n",
                    (unsigned)cp, (unsigned long)ks);
        }
        p += n;
        nanosleep(&delay, NULL);
    }
    if (free_kc > 0) {
        restore = NoSymbol;
        XChangeKeyboardMapping(
            g_app.xdpy, free_kc, 1, &restore, 1);
        XSync(g_app.xdpy, False);
    }
    if (failed > 0)
        fprintf(stderr,
                "type: %d/%d codepoint(s) could not be typed\n",
                failed, typed + failed);
    return (typed > 0 || failed == 0) ? 0 : -1;
}

/**
 * notify_cb - GLib idle callback that builds and shows the bubble.
 * @data: heap-allocated struct notify_msg (we own it).
 *
 * Honours OPONS_VOXD_NOTIFY for non-error notifications:
 *   normal — bubble + sound (the libnotify default)
 *   quiet  — bubble, suppressed sound (default)
 *   silent — bubble, suppressed sound, low urgency
 *   off    — no bubble at all
 *
 * Errors are NEVER suppressed and NEVER silent. is_error=true is
 * treated like NORMAL in every mode: bubble shown, sound played.
 */
static gboolean notify_cb(gpointer data)
{
    struct notify_msg *msg = data;
    NotifyNotification *n;
    GError *err = NULL;
    enum notify_mode mode;

    mode = msg->is_error ? NOTIFY_NORMAL : g_app.notify_mode;
    if (mode == NOTIFY_OFF) {
        g_free(msg->title);
        g_free(msg->body);
        g_free(msg);
        return G_SOURCE_REMOVE;
    }
    n = notify_notification_new(
        msg->title, msg->body, "audio-input-microphone");
    notify_notification_set_timeout(n, NOTIFY_TIMEOUT_MS);
    if (!g_app.notify_persist)
        notify_notification_set_hint(
            n, "transient",
            g_variant_new_boolean(TRUE));
    if (mode == NOTIFY_QUIET || mode == NOTIFY_SILENT)
        notify_notification_set_hint(
            n, "suppress-sound",
            g_variant_new_boolean(TRUE));
    if (mode == NOTIFY_SILENT)
        notify_notification_set_urgency(
            n, NOTIFY_URGENCY_LOW);
    if (!notify_notification_show(n, &err)) {
        fprintf(stderr, "notify: %s\n",
                err ? err->message : "unknown error");
        if (err)
            g_error_free(err);
    }
    g_object_unref(n);
    g_free(msg->title);
    g_free(msg->body);
    g_free(msg);
    return G_SOURCE_REMOVE;
}

/**
 * request_notify - Queue a notification on the GLib main loop.
 * @title: notification title.
 * @body: notification body.
 * @is_error: if true, the notification is shown unconditionally with
 *            sound, regardless of OPONS_VOXD_NOTIFY. Use for any
 *            failure mode (mic error, empty recording, no speech,
 *            typing failure, etc.) so the user is never left in the
 *            dark when something went wrong.
 */
static void request_notify(const char *title, const char *body,
                           bool is_error)
{
    struct notify_msg *msg;

    msg = g_new(struct notify_msg, 1);
    msg->title = g_strdup(title);
    msg->body = g_strdup(body);
    msg->is_error = is_error;
    g_idle_add(notify_cb, msg);
}

/* ---- voice commands (file-based) ---- */

/**
 * process_escapes - Expand \n and \t escape sequences in place.
 * @s: NUL-terminated string to modify.
 */
static void process_escapes(char *s)
{
    char *r = s;
    char *w = s;

    while (*r) {
        if (*r == '\\' && *(r + 1) == 'n') {
            *w++ = '\n';
            r += 2;
        } else if (*r == '\\' && *(r + 1) == 't') {
            *w++ = '\t';
            r += 2;
        } else {
            *w++ = *r++;
        }
    }
    *w = '\0';
}

/*
 * Sort comparator: longest spoken form first, so "open parenthesis"
 * is tried before "open" and partial matches are avoided.
 */
static int cmd_cmp_len_desc(const void *a, const void *b)
{
    const struct voice_cmd *ca = a;
    const struct voice_cmd *cb = b;
    size_t la = strlen(ca->spoken);
    size_t lb = strlen(cb->spoken);

    if (la > lb)
        return -1;
    if (la < lb)
        return 1;
    return 0;
}

/**
 * load_commands - Read voice command pairs from a text file.
 *
 * Looks for OPONS_VOXD_CMDS_FILE first, then commands/<lang>.txt
 * where <lang> is the first two characters of OPONS_VOXD_LANGUAGE.
 * Does nothing if commands are disabled or no file is found.
 */
static void load_commands(void)
{
    const char *path;
    char auto_path[256];
    FILE *fp;
    char line[CMD_LINE_MAX];
    char *sep;
    size_t slen;

    if (!g_app.commands_on)
        return;
    path = getenv("OPONS_VOXD_CMDS_FILE");
    if (!path || !*path) {
        snprintf(auto_path, sizeof(auto_path),
                 "%s/%.2s.txt", CMDS_DIR, g_app.lang);
        path = auto_path;
    }
    fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "voice commands: %s not found\n",
                path);
        return;
    }
    g_app.cmd_cap = CMD_INITIAL_CAP;
    g_app.commands = malloc(
        g_app.cmd_cap * sizeof(*g_app.commands));
    if (!g_app.commands) {
        fclose(fp);
        return;
    }
    g_app.cmd_count = 0;
    while (fgets(line, CMD_LINE_MAX, fp)) {
        slen = strlen(line);
        while (slen > 0 &&
               (line[slen - 1] == '\n' ||
                line[slen - 1] == '\r'))
            line[--slen] = '\0';
        if (slen == 0 || line[0] == '#')
            continue;
        sep = strchr(line, '|');
        if (!sep)
            continue;
        *sep = '\0';
        process_escapes(sep + 1);
        if (g_app.cmd_count >= g_app.cmd_cap) {
            g_app.cmd_cap *= 2;
            g_app.commands = realloc(g_app.commands,
                g_app.cmd_cap * sizeof(*g_app.commands));
            if (!g_app.commands)
                break;
        }
        g_app.commands[g_app.cmd_count].spoken =
            strdup(line);
        g_app.commands[g_app.cmd_count].replacement =
            strdup(sep + 1);
        g_app.cmd_count++;
    }
    fclose(fp);
    qsort(g_app.commands, g_app.cmd_count,
          sizeof(*g_app.commands), cmd_cmp_len_desc);
    fprintf(stderr, "voice commands: loaded %d from %s\n",
            g_app.cmd_count, path);
}

static void free_commands(void)
{
    int i;

    for (i = 0; i < g_app.cmd_count; i++) {
        free(g_app.commands[i].spoken);
        free(g_app.commands[i].replacement);
    }
    free(g_app.commands);
    g_app.commands = NULL;
    g_app.cmd_count = 0;
}

/* ---- text processing ---- */

/**
 * str_replace_alloc - Replace every occurrence of @needle by @repl in @in.
 * @in:     source NUL-terminated string.
 * @needle: substring to match, ASCII-case-insensitive. Empty needle
 *          yields a verbatim copy.
 * @repl:   replacement string (inserted verbatim).
 *
 * Matching is case-insensitive on ASCII letters so voice commands can
 * fire regardless of how Whisper capitalised the spoken phrase, while
 * non-matched portions of @in keep their original casing — which
 * preserves proper nouns mid-sentence. The output buffer is grown
 * with realloc as needed, so no caller-side assumption about the
 * expansion ratio is required.
 *
 * Return: malloc'd NUL-terminated result, or NULL on allocation failure
 *         or size_t overflow. Caller must free.
 */
static char *str_replace_alloc(const char *in,
                               const char *needle, const char *repl)
{
    size_t nlen = strlen(needle);
    size_t rlen = strlen(repl);
    size_t cap = strlen(in) + 1;
    size_t used = 0;
    const char *p = in;
    char *out;

    out = malloc(cap);
    if (!out)
        return NULL;
    while (*p) {
        bool match = (nlen > 0 && strncasecmp(p, needle, nlen) == 0);
        size_t need = match ? rlen : 1;

        if (used + need + 1 > cap) {
            size_t want = used + need + 1;
            size_t ncap;
            char *tmp;

            if (want < used || cap > SIZE_MAX / 2) {
                free(out);
                return NULL;
            }
            ncap = cap * 2;
            if (ncap < want)
                ncap = want;
            tmp = realloc(out, ncap);
            if (!tmp) {
                free(out);
                return NULL;
            }
            out = tmp;
            cap = ncap;
        }
        if (match) {
            memcpy(out + used, repl, rlen);
            used += rlen;
            p += nlen;
        } else {
            out[used++] = *p++;
        }
    }
    out[used] = '\0';
    return out;
}

static void str_lower_ascii(char *s)
{
    for (; *s; ++s) {
        if ((unsigned char)*s < 128 && *s >= 'A' && *s <= 'Z')
            *s = (char)(*s + ('a' - 'A'));
    }
}

/**
 * apply_voice_cmds - Replace spoken commands from the loaded table.
 * @text: raw transcript from whisper.
 *
 * Return: newly allocated string (caller must free).
 */
static char *apply_voice_cmds(const char *text)
{
    char *cur;
    int i;

    cur = strdup(text);
    if (!cur)
        return NULL;
    for (i = 0; i < g_app.cmd_count; i++) {
        char *next = str_replace_alloc(cur,
                                       g_app.commands[i].spoken,
                                       g_app.commands[i].replacement);

        if (!next) {
            free(cur);
            return strdup(text);
        }
        free(cur);
        cur = next;
    }
    return cur;
}

/**
 * capitalize_sentences - Uppercase the first letter of each sentence.
 * @text: NUL-terminated UTF-8 string to modify in place.
 *
 * Handles ASCII a-z and any Unicode lowercase letter whose uppercase
 * form encodes to the same number of UTF-8 bytes — which covers every
 * accented Latin letter used in French, German, Spanish, Italian, etc.
 * Requires setlocale(LC_CTYPE, "") to have been called at startup.
 */
static void capitalize_sentences(char *text)
{
    bool cap_next = true;
    mbstate_t st_in;
    char *p;

    if (!text || !*text)
        return;
    memset(&st_in, 0, sizeof(st_in));
    p = text;
    while (*p) {
        wchar_t wc;
        size_t n = mbrtowc(&wc, p, MB_CUR_MAX, &st_in);

        if (n == 0 || n == (size_t)-1 || n == (size_t)-2) {
            memset(&st_in, 0, sizeof(st_in));
            ++p;
            continue;
        }
        if (cap_next && iswlower((wint_t)wc)) {
            char buf[MB_LEN_MAX];
            mbstate_t st_out;
            wchar_t up = towupper((wint_t)wc);
            size_t m;

            memset(&st_out, 0, sizeof(st_out));
            m = wcrtomb(buf, up, &st_out);
            if (m != (size_t)-1 && m == n)
                memcpy(p, buf, m);
            cap_next = false;
        } else if (wc == L'.' || wc == L'!' || wc == L'?') {
            cap_next = true;
        } else if (wc != L' ' && wc != L'\n' && wc != L'\t') {
            cap_next = false;
        }
        p += n;
    }
}

/* ---- PortAudio ---- */

static int audio_cb(const void *in, void *out,
                    unsigned long count,
                    const PaStreamCallbackTimeInfo *ti,
                    PaStreamCallbackFlags flags, void *udata)
{
    size_t pos;

    (void)out;
    (void)ti;
    (void)flags;
    (void)udata;
    if (!in)
        return paContinue;
    pos = atomic_fetch_add(&g_app.audio_len, count);
    if (pos + count > AUDIO_CAPACITY) {
        atomic_fetch_sub(&g_app.audio_len, count);
        return paContinue;
    }
    memcpy(g_app.audio_buf + pos, in,
           count * sizeof(float));
    return paContinue;
}

static int audio_start(void)
{
    PaStreamParameters params;
    PaError err;

    atomic_store(&g_app.audio_len, 0);
    memset(&params, 0, sizeof(params));
    if (g_app.input_device >= 0)
        params.device = g_app.input_device;
    else
        params.device = Pa_GetDefaultInputDevice();
    if (params.device == paNoDevice) {
        fprintf(stderr, "no input device\n");
        return -1;
    }
    params.channelCount = NUM_CHANNELS;
    params.sampleFormat = paFloat32;
    params.suggestedLatency =
        Pa_GetDeviceInfo(params.device)
            ->defaultLowInputLatency;
    err = Pa_OpenStream(&g_app.stream, &params, NULL,
                        SAMPLE_RATE, FRAMES_PER_BUFFER,
                        paNoFlag, audio_cb, NULL);
    if (err != paNoError) {
        fprintf(stderr, "Pa_OpenStream: %s\n",
                Pa_GetErrorText(err));
        return -1;
    }
    err = Pa_StartStream(g_app.stream);
    if (err != paNoError) {
        fprintf(stderr, "Pa_StartStream: %s\n",
                Pa_GetErrorText(err));
        Pa_CloseStream(g_app.stream);
        g_app.stream = NULL;
        return -1;
    }
    return 0;
}

static void audio_stop(void)
{
    if (!g_app.stream)
        return;
    Pa_StopStream(g_app.stream);
    Pa_CloseStream(g_app.stream);
    g_app.stream = NULL;
}

/* ---- whisper transcription ---- */

/**
 * run_whisper - Transcribe (or translate) a float32 PCM buffer.
 * @samples: mono 16 kHz float32 audio.
 * @n_samples: number of samples.
 * @translate: if true, run Whisper's translate task so the output
 * is English regardless of the spoken (source) language, instead
 * of a verbatim transcription in that language.
 *
 * Return: newly allocated string (caller must free).
 */
static char *run_whisper(const float *samples, size_t n_samples,
                         bool translate)
{
    struct whisper_context *ctx;
    struct whisper_full_params wp;
    int n_seg;
    int i;
    const char *seg_text;
    size_t cap = 256;
    size_t used = 0;
    size_t tlen;
    char *out;
    char *nout;

    ctx = translate ? g_app.wctx_translate : g_app.wctx;
    wp = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    wp.print_realtime = false;
    wp.print_progress = false;
    wp.print_timestamps = false;
    wp.print_special = false;
    wp.translate = translate;
    wp.single_segment = false;
    wp.no_context = true;
    wp.suppress_blank = true;
    wp.n_threads = WHISPER_THREADS;
    wp.language = NULL;
    if (strcmp(g_app.lang, "auto") != 0)
        wp.language = g_app.lang;
    if (whisper_full(ctx, wp, samples, (int)n_samples) != 0)
        return strdup("");
    n_seg = whisper_full_n_segments(ctx);
    out = malloc(cap);
    if (!out)
        return NULL;
    out[0] = '\0';
    for (i = 0; i < n_seg; i++) {
        seg_text = whisper_full_get_segment_text(
            ctx, i);
        while (*seg_text == ' ')
            seg_text++;
        tlen = strlen(seg_text);
        if (used + tlen + 2 >= cap) {
            cap = (used + tlen + 2) * 2;
            nout = realloc(out, cap);
            if (!nout) {
                free(out);
                return NULL;
            }
            out = nout;
        }
        if (used > 0)
            out[used++] = ' ';
        memcpy(out + used, seg_text, tlen);
        used += tlen;
        out[used] = '\0';
    }
    return out;
}

/**
 * transcribe_thread - Worker: stop audio, run whisper, publish.
 * @arg: unused.
 *
 * Runs on a detached thread spawned by rec_stop().
 */
static void *transcribe_thread(void *arg)
{
    size_t n;
    double audio_sec;
    double elapsed;
    struct timespec t0;
    struct timespec t1;
    char *raw;
    char *text;
    bool translate;

    (void)arg;
    translate = g_app.translate_active;
    n = atomic_load(&g_app.audio_len);
    if (n < MIN_AUDIO_SAMPLES) {
        request_notify("opons-voxd", "Empty recording", true);
        g_app.via_hotkey = false;
        g_app.translate_active = false;
        request_state(STATE_IDLE);
        return NULL;
    }
    audio_sec = (double)n / SAMPLE_RATE;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    raw = run_whisper(g_app.audio_buf, n, translate);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    elapsed = (t1.tv_sec - t0.tv_sec)
            + (t1.tv_nsec - t0.tv_nsec) / 1e9;
    fprintf(stderr,
            "[perf] audio: %.1f s | transcription: %.2f s"
            " | ratio: %.1fx realtime\n",
            audio_sec, elapsed, audio_sec / elapsed);
    if (translate)
        fprintf(stderr, "[perf] translate: %s->en\n", g_app.lang);
    if (!raw || !*raw) {
        free(raw);
        request_notify("opons-voxd", "No speech detected", true);
        g_app.via_hotkey = false;
        g_app.translate_active = false;
        request_state(STATE_IDLE);
        return NULL;
    }
    if (g_app.commands_on && g_app.cmd_count > 0) {
        text = apply_voice_cmds(raw);
        free(raw);
    } else {
        text = raw;
    }
    capitalize_sentences(text);
    if (text && *text) {
        if (g_app.via_hotkey) {
            if (type_text(text) != 0) {
                /*
                 * XTest synthesis failed (no display, every keysym
                 * unmappable, …). Don't lose the transcript: copy
                 * it to the clipboards so the user can paste it,
                 * and surface the failure with an error notification
                 * (always sounds, regardless of OPONS_VOXD_NOTIFY).
                 */
                copy_to_clipboards(text);
                request_notify(
                    "opons-voxd",
                    "Typing failed — transcript copied to clipboard",
                    true);
            }
        } else {
            copy_to_clipboards(text);
            request_notify("opons-voxd", text, false);
        }
    } else {
        request_notify("opons-voxd", "No speech detected", true);
    }
    free(text);
    g_app.via_hotkey = false;
    g_app.translate_active = false;
    request_state(STATE_IDLE);
    return NULL;
}

/* ---- recording control ---- */

static void rec_start(void)
{
    if (audio_start() != 0) {
        request_notify("opons-voxd",
                       "Mic error: cannot open stream", true);
        return;
    }
    set_state(STATE_RECORDING);
}

static void rec_stop(void)
{
    pthread_t tid;

    audio_stop();
    set_state(STATE_PROCESSING);
    if (pthread_create(&tid, NULL,
                       transcribe_thread, NULL) != 0) {
        perror("pthread_create");
        request_state(STATE_IDLE);
        return;
    }
    pthread_detach(tid);
}

/* ---- GTK tray handlers ---- */

static void on_activate(GtkStatusIcon *icon, gpointer data)
{
    enum app_state s;

    (void)icon;
    (void)data;
    s = atomic_load(&g_app.state);
    if (s == STATE_IDLE)
        rec_start();
    else if (s == STATE_RECORDING)
        rec_stop();
}

static void on_toggle(GtkMenuItem *item, gpointer data)
{
    (void)item;
    (void)data;
    on_activate(NULL, NULL);
}

static void on_quit(GtkMenuItem *item, gpointer data)
{
    (void)item;
    (void)data;
    gtk_main_quit();
}

static void on_popup(GtkStatusIcon *icon, guint btn,
                     guint time, gpointer data)
{
    (void)icon;
    (void)btn;
    (void)time;
    (void)data;
    gtk_menu_popup_at_pointer(GTK_MENU(g_app.menu), NULL);
}

/* ---- push-to-talk hotkey ---- */

/**
 * parse_hotkey - Decode "ctrl+shift+space" into modifiers + keysym.
 * @spec: hotkey description, tokens separated by '+'.
 * @mods: out, X11 modifier mask.
 * @ks:   out, X11 keysym.
 *
 * Modifier tokens (all but the last) are matched case-insensitively
 * against ctrl, shift, alt, super. The last token is the key, and
 * X11 keysym names are case-sensitive (XF86Tools and Return are
 * different from xf86tools and return), so we try the spelling
 * as-given first, then a lowercase form, then the canonical
 * "first letter uppercase, rest lowercase" form. That covers
 * "Space" / "space" / "SPACE" for letters, and preserves things
 * like "XF86Tools" exactly.
 *
 * Return: 0 on success, -1 if @spec is malformed or the key is
 * unknown.
 */
static int parse_hotkey(const char *spec, unsigned int *mods,
                        KeySym *ks)
{
    char buf[128];
    char *tokens[HOTKEY_TOKENS_MAX];
    int n_tokens = 0;
    char *p;
    char *save;
    const char *key_token;
    int i;

    if (!spec || !*spec)
        return -1;
    strncpy(buf, spec, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    for (p = strtok_r(buf, "+", &save); p;
         p = strtok_r(NULL, "+", &save)) {
        while (*p == ' ' || *p == '\t')
            p++;
        if (n_tokens >= HOTKEY_TOKENS_MAX)
            return -1;
        tokens[n_tokens++] = p;
    }
    if (n_tokens == 0)
        return -1;
    *mods = 0;
    for (i = 0; i < n_tokens - 1; i++) {
        if (strcasecmp(tokens[i], "ctrl") == 0 ||
            strcasecmp(tokens[i], "control") == 0)
            *mods |= ControlMask;
        else if (strcasecmp(tokens[i], "shift") == 0)
            *mods |= ShiftMask;
        else if (strcasecmp(tokens[i], "alt") == 0 ||
                 strcasecmp(tokens[i], "mod1") == 0)
            *mods |= Mod1Mask;
        else if (strcasecmp(tokens[i], "super") == 0 ||
                 strcasecmp(tokens[i], "win") == 0 ||
                 strcasecmp(tokens[i], "mod4") == 0)
            *mods |= Mod4Mask;
        else
            return -1;
    }
    key_token = tokens[n_tokens - 1];
    *ks = XStringToKeysym(key_token);
    if (*ks == NoSymbol && key_token[0]) {
        char alt[64];

        strncpy(alt, key_token, sizeof(alt) - 1);
        alt[sizeof(alt) - 1] = '\0';
        str_lower_ascii(alt);
        *ks = XStringToKeysym(alt);
        if (*ks == NoSymbol &&
            alt[0] >= 'a' && alt[0] <= 'z') {
            alt[0] = (char)(alt[0] - ('a' - 'A'));
            *ks = XStringToKeysym(alt);
        }
    }
    if (*ks == NoSymbol)
        return -1;
    return 0;
}

/*
 * X error handler installed globally in main(). Default Xlib
 * behaviour is to print on stderr and call exit(1) on protocol
 * errors, which is hostile for a long-running tray app. We log
 * verbosely (request opcode, error code, resource id) so the
 * user can pinpoint what failed, and return 0 so Xlib swallows
 * the error and the program keeps running.
 */
static int x_silent_error_handler(Display *dpy, XErrorEvent *ev)
{
    char buf[256];

    XGetErrorText(dpy, ev->error_code, buf, sizeof(buf));
    fprintf(stderr,
            "Xlib error: %s "
            "(serial=%lu, request=%u.%u, resourceid=0x%lx)\n",
            buf,
            ev->serial,
            (unsigned)ev->request_code,
            (unsigned)ev->minor_code,
            (unsigned long)ev->resourceid);
    return 0;
}

/**
 * grab_hotkey_combo - Grab @hk's key with one mod overlay.
 * @hk: hotkey binding to grab (its mods/keycode must already be
 * resolved).
 * @extra: extra modifier bits to OR into the base modifier mask
 * (typically combinations of LockMask and Mod2Mask, so the grab
 * still triggers when CapsLock or NumLock is on).
 */
static void grab_hotkey_combo(struct ptt_binding *hk,
                              unsigned int extra)
{
    XGrabKey(g_app.xdpy, (int)hk->keycode,
             hk->mods | extra,
             DefaultRootWindow(g_app.xdpy),
             True, GrabModeAsync, GrabModeAsync);
}

/**
 * ungrab_hotkey_combo - Symmetric counterpart to grab_hotkey_combo.
 * @hk: hotkey binding to ungrab.
 * @extra: same extra modifier bits used at grab time.
 */
static void ungrab_hotkey_combo(struct ptt_binding *hk,
                                unsigned int extra)
{
    XUngrabKey(g_app.xdpy, (int)hk->keycode,
               hk->mods | extra,
               DefaultRootWindow(g_app.xdpy));
}

/**
 * resolve_ptt_hotkey - Parse a hotkey spec into @hk.
 * @spec: hotkey spec string (env or default).
 * @hk: out, receives the parsed modifier mask and resolved keycode.
 *
 * Return: 0 on success, -1 if @spec is malformed (unknown modifier
 * or unknown keysym name), -2 if the keysym is valid but is not
 * mapped to any keycode on the current X server (typically a key
 * the kernel/firmware sends but the active XKB layout doesn't
 * declare; xev shows it but xmodmap -pke does not). The caller
 * uses the distinction to print a useful diagnostic.
 */
static int resolve_ptt_hotkey(const char *spec, struct ptt_binding *hk)
{
    KeySym ks;

    if (parse_hotkey(spec, &hk->mods, &ks) != 0)
        return -1;
    hk->keycode = (unsigned int)XKeysymToKeycode(g_app.xdpy, ks);
    if (hk->keycode == 0)
        return -2;
    return 0;
}

/**
 * open_ptt_display - Acquire the X11 display from GDK.
 *
 * Stores the display pointer in g_app.xdpy. Also disables detectable
 * auto-repeat so that holding the hotkey produces a single
 * KeyPress/KeyRelease pair instead of synthetic repeat events.
 *
 * Return: 0 on success, -1 if no X display is available.
 */
static int open_ptt_display(void)
{
    g_app.xdpy = gdk_x11_get_default_xdisplay();
    if (!g_app.xdpy)
        return -1;
    XkbSetDetectableAutoRepeat(g_app.xdpy, True, NULL);
    XSync(g_app.xdpy, False);
    return 0;
}

/**
 * grab_ptt_hotkey - Grab @hk's key on the X root window.
 * @hk: hotkey binding to grab (its mods/keycode must already be
 * resolved).
 *
 * Grabs four mod-overlay variants so the hotkey works regardless of
 * CapsLock/NumLock state. Errors (e.g. BadAccess if another client
 * already holds the grab) are silently swallowed by an error handler
 * scoped to this function.
 */
static void grab_ptt_hotkey(struct ptt_binding *hk)
{
    int (*old_handler)(Display *, XErrorEvent *);

    old_handler = XSetErrorHandler(x_silent_error_handler);
    grab_hotkey_combo(hk, 0);
    grab_hotkey_combo(hk, LockMask);
    grab_hotkey_combo(hk, Mod2Mask);
    grab_hotkey_combo(hk, LockMask | Mod2Mask);
    XSync(g_app.xdpy, False);
    XSetErrorHandler(old_handler);
    hk->grabbed = true;
}

/**
 * ungrab_ptt_hotkey - Release @hk's grabs.
 * @hk: hotkey binding to release.
 *
 * Symmetric counterpart to grab_ptt_hotkey, called from free_hotkey
 * during shutdown.
 */
static void ungrab_ptt_hotkey(struct ptt_binding *hk)
{
    int (*old_handler)(Display *, XErrorEvent *);

    if (!hk->grabbed)
        return;
    old_handler = XSetErrorHandler(x_silent_error_handler);
    ungrab_hotkey_combo(hk, 0);
    ungrab_hotkey_combo(hk, LockMask);
    ungrab_hotkey_combo(hk, Mod2Mask);
    ungrab_hotkey_combo(hk, LockMask | Mod2Mask);
    XSync(g_app.xdpy, False);
    XSetErrorHandler(old_handler);
    hk->grabbed = false;
}

/**
 * init_hotkey - Parse and grab both PTT hotkeys.
 *
 * Reads OPONS_VOXD_PTT_HOTKEY (plain dictation) and
 * OPONS_VOXD_PTT_TRANSLATE_HOTKEY (dictation translated to
 * English), grabbing each independently on the X root window.
 * Failures are non-fatal and scoped to the affected hotkey: if a
 * spec fails to parse, no X display is available, a key is already
 * grabbed by another client, or the two hotkeys resolve to the same
 * combo, that hotkey (or both, if the display is unavailable) is
 * silently disabled and the tray mode keeps working.
 */
static void init_hotkey(void)
{
    const char *spec;
    const char *tr_spec;
    int rc;

    spec = getenv("OPONS_VOXD_PTT_HOTKEY");
    if (!spec || !*spec)
        spec = DEFAULT_PTT_HOTKEY;
    tr_spec = getenv("OPONS_VOXD_PTT_TRANSLATE_HOTKEY");
    if (!tr_spec || !*tr_spec)
        tr_spec = DEFAULT_PTT_TRANSLATE_HOTKEY;

    if (open_ptt_display() != 0) {
        fprintf(stderr,
                "ptt: no X display, "
                "push-to-talk disabled\n");
        return;
    }

    rc = resolve_ptt_hotkey(spec, &g_app.ptt);
    if (rc == -1) {
        fprintf(stderr,
                "ptt: cannot parse hotkey '%s' (unknown "
                "modifier or unknown keysym name), "
                "push-to-talk disabled\n", spec);
    } else if (rc == -2) {
        fprintf(stderr,
                "ptt: keysym in '%s' is not mapped to any "
                "keycode on this X server. Run `xev` to see "
                "what your key actually sends, or check "
                "`xmodmap -pke`. Push-to-talk disabled.\n",
                spec);
    } else {
        grab_ptt_hotkey(&g_app.ptt);
        fprintf(stderr, "ptt: hotkey '%s' active\n", spec);
    }

    rc = resolve_ptt_hotkey(tr_spec, &g_app.ptt_translate);
    if (rc == -1) {
        fprintf(stderr,
                "ptt: cannot parse translate hotkey '%s' "
                "(unknown modifier or unknown keysym name), "
                "translate push-to-talk disabled\n", tr_spec);
    } else if (rc == -2) {
        fprintf(stderr,
                "ptt: keysym in translate hotkey '%s' is not "
                "mapped to any keycode on this X server. Run "
                "`xev` to see what your key actually sends, or "
                "check `xmodmap -pke`. Translate push-to-talk "
                "disabled.\n", tr_spec);
    } else if (g_app.ptt.grabbed &&
              g_app.ptt.mods == g_app.ptt_translate.mods &&
              g_app.ptt.keycode == g_app.ptt_translate.keycode) {
        fprintf(stderr,
                "ptt: translate hotkey '%s' is identical to the "
                "push-to-talk hotkey, translate push-to-talk "
                "disabled\n", tr_spec);
    } else {
        grab_ptt_hotkey(&g_app.ptt_translate);
        fprintf(stderr,
                "ptt: translate hotkey '%s' active (speech -> "
                "English)\n", tr_spec);
    }

    if (g_app.ptt.grabbed || g_app.ptt_translate.grabbed) {
        gdk_window_add_filter(NULL, ptt_event_filter, NULL);
        g_app.ptt_filter_installed = true;
    }
}

/**
 * free_hotkey - Symmetric counterpart to init_hotkey.
 *
 * Called from main() at shutdown, mirroring whisper_init/whisper_free,
 * Pa_Initialize/Pa_Terminate, notify_init/notify_uninit, and
 * load_commands/free_commands.
 */
static void free_hotkey(void)
{
    if (g_app.ptt_filter_installed) {
        gdk_window_remove_filter(NULL, ptt_event_filter, NULL);
        g_app.ptt_filter_installed = false;
    }
    ungrab_ptt_hotkey(&g_app.ptt);
    ungrab_ptt_hotkey(&g_app.ptt_translate);
}

/*
 * GDK event filter: peek at every X event delivered to this client
 * and react to KeyPress/KeyRelease for either grabbed hotkey (plain
 * PTT or PTT-translate). We mask out LockMask/Mod2Mask before
 * comparing modifiers so the filter matches regardless of CapsLock
 * or NumLock state. Whichever key started the current recording is
 * remembered in g_app.active_ptt_keycode so that, if the other
 * hotkey is also pressed while the first is still held, its release
 * is ignored instead of prematurely stopping the wrong recording.
 */
static GdkFilterReturn ptt_event_filter(GdkXEvent *xev,
                                        GdkEvent *gev,
                                        gpointer data)
{
    XEvent *e = xev;
    unsigned int mods;
    enum app_state s;
    bool is_ptt;
    bool is_translate;

    (void)gev;
    (void)data;
    if (!g_app.ptt.grabbed && !g_app.ptt_translate.grabbed)
        return GDK_FILTER_CONTINUE;
    if (e->type != KeyPress && e->type != KeyRelease)
        return GDK_FILTER_CONTINUE;
    mods = e->xkey.state & ~(LockMask | Mod2Mask);
    is_ptt = g_app.ptt.grabbed &&
             e->xkey.keycode == g_app.ptt.keycode &&
             mods == g_app.ptt.mods;
    is_translate = g_app.ptt_translate.grabbed &&
                   e->xkey.keycode == g_app.ptt_translate.keycode &&
                   mods == g_app.ptt_translate.mods;
    if (!is_ptt && !is_translate)
        return GDK_FILTER_CONTINUE;
    s = atomic_load(&g_app.state);
    if (e->type == KeyPress) {
        if (s == STATE_IDLE) {
            g_app.via_hotkey = true;
            g_app.translate_active = is_translate;
            g_app.active_ptt_keycode = e->xkey.keycode;
            rec_start();
        }
    } else {
        if (s == STATE_RECORDING && g_app.via_hotkey &&
            e->xkey.keycode == g_app.active_ptt_keycode)
            rec_stop();
    }
    return GDK_FILTER_REMOVE;
}

/* ---- initialization ---- */

/**
 * notify_critical - Show a synchronous critical desktop notification.
 * @title: bubble title.
 * @body:  bubble body.
 *
 * Used for fatal startup errors that happen before gtk_main() is
 * running, so the asynchronous request_notify() path is not yet
 * usable. Critical urgency keeps the bubble visible until the user
 * dismisses it.
 */
static void notify_critical(const char *title, const char *body)
{
    NotifyNotification *n;
    GError *err = NULL;

    n = notify_notification_new(title, body, "dialog-error");
    if (!n)
        return;
    notify_notification_set_urgency(n, NOTIFY_URGENCY_CRITICAL);
    if (!notify_notification_show(n, &err)) {
        fprintf(stderr, "notify: %s\n",
                err ? err->message : "unknown");
        if (err)
            g_error_free(err);
    }
    g_object_unref(n);
}

/**
 * acquire_instance_lock - Refuse to start if another instance is up.
 *
 * Tries to take an exclusive non-blocking flock on a per-user lock
 * file under /tmp. The kernel releases the lock automatically when
 * the process exits (clean or crashed), so there is no stale-lock
 * issue and no manual cleanup. The fd is kept in @g_lock_fd for the
 * lifetime of the process.
 *
 * Return: 0 on success. -1 if the lock is already held or the file
 *         cannot be opened; in that case a critical notification has
 *         been shown to the user.
 */
static int acquire_instance_lock(void)
{
    char path[64];
    char body[256];
    char pidbuf[32];
    int pidlen;
    int fd;

    snprintf(path, sizeof(path), "/tmp/opons-voxd-%u.lock",
             (unsigned)getuid());
    fd = open(path, O_RDWR | O_CREAT | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (fd < 0) {
        snprintf(body, sizeof(body),
                 "Cannot open lock file %s: %s",
                 path, strerror(errno));
        notify_critical("opons-voxd: cannot start", body);
        return -1;
    }
    if (flock(fd, LOCK_EX | LOCK_NB) != 0) {
        int saved = errno;

        close(fd);
        if (saved == EWOULDBLOCK) {
            snprintf(body, sizeof(body),
                     "Another opons-voxd instance is already "
                     "running. Quit it first (right-click the "
                     "tray icon, or pkill opons-voxd) before "
                     "starting a new one.");
        } else {
            snprintf(body, sizeof(body),
                     "Cannot lock %s: %s",
                     path, strerror(saved));
        }
        notify_critical("opons-voxd: cannot start", body);
        return -1;
    }
    /*
     * Best-effort: write our PID into the lock file so an admin can
     * tell which process holds it. Failures here do not invalidate
     * the lock, which is what protects single-instance semantics.
     */
    pidlen = snprintf(pidbuf, sizeof(pidbuf), "%u\n",
                      (unsigned)getpid());
    if (pidlen > 0 && pidlen < (int)sizeof(pidbuf)) {
        if (ftruncate(fd, 0) == 0)
            (void)!write(fd, pidbuf, (size_t)pidlen);
    }
    g_lock_fd = fd;
    return 0;
}

static int init_whisper(void)
{
    const char *model;
    struct whisper_context_params cp;

    model = getenv("OPONS_VOXD_MODEL");
    if (!model || !*model)
        model = DEFAULT_MODEL;
    fprintf(stderr, "loading whisper model: %s\n", model);
    cp = whisper_context_default_params();
    cp.use_gpu = true;
    g_app.wctx =
        whisper_init_from_file_with_params(model, cp);
    if (!g_app.wctx) {
        char body[512];

        fprintf(stderr, "failed to load model: %s\n",
                model);
        snprintf(body, sizeof(body),
                 "Failed to load Whisper model:\n%s\n\n"
                 "Check that the file exists and is readable. "
                 "Set OPONS_VOXD_MODEL to override the path.",
                 model);
        notify_critical("opons-voxd: cannot start", body);
        return -1;
    }
    fprintf(stderr, "whisper ready\n");
    return 0;
}

/**
 * init_whisper_translate - Load the model used by the translate
 * hotkey.
 *
 * Reads OPONS_VOXD_TRANSLATE_MODEL (default: DEFAULT_TRANSLATE_MODEL).
 * When it resolves to the same path as the main model
 * (OPONS_VOXD_MODEL), reuses g_app.wctx instead of loading a second
 * copy. Must run after init_whisper(), which g_app.wctx depends on.
 *
 * Failure to load is non-fatal: the translate hotkey then falls
 * back to the main model, which may translate poorly (e.g. speed-
 * oriented models such as large-v3-turbo) but keeps working.
 */
static void init_whisper_translate(void)
{
    const char *model;
    const char *main_model;
    struct whisper_context_params cp;

    model = getenv("OPONS_VOXD_TRANSLATE_MODEL");
    if (!model || !*model)
        model = DEFAULT_TRANSLATE_MODEL;
    main_model = getenv("OPONS_VOXD_MODEL");
    if (!main_model || !*main_model)
        main_model = DEFAULT_MODEL;
    if (strcmp(model, main_model) == 0) {
        g_app.wctx_translate = g_app.wctx;
        return;
    }
    fprintf(stderr, "loading translate whisper model: %s\n", model);
    cp = whisper_context_default_params();
    cp.use_gpu = true;
    g_app.wctx_translate =
        whisper_init_from_file_with_params(model, cp);
    if (!g_app.wctx_translate) {
        fprintf(stderr,
                "failed to load translate model: %s, "
                "translate hotkey will use the main model "
                "instead\n", model);
        g_app.wctx_translate = g_app.wctx;
        return;
    }
    fprintf(stderr, "translate whisper ready\n");
}

static void init_lang(void)
{
    const char *lang;

    lang = getenv("OPONS_VOXD_LANGUAGE");
    if (!lang || !*lang)
        lang = DEFAULT_LANG;
    strncpy(g_app.lang, lang, sizeof(g_app.lang) - 1);
    g_app.lang[sizeof(g_app.lang) - 1] = '\0';
}

static void init_device(void)
{
    const char *dev;

    dev = getenv("OPONS_VOXD_DEVICE");
    g_app.input_device = (dev && *dev) ? atoi(dev) : -1;
}

/**
 * init_options - Read all opt-in environment variables.
 *
 * OPONS_VOXD_COMMANDS         "1" enables voice commands
 * OPONS_VOXD_NOTIFY_PERSIST   "1" keeps notifications in history
 * OPONS_VOXD_NOTIFY           "normal" | "quiet" (default) |
 *                             "silent" | "off" — affects success
 *                             notifications only; errors always
 *                             show with sound regardless.
 */
static void init_options(void)
{
    const char *cmds;
    const char *persist;
    const char *mode;
    const char *mode_label = "quiet";

    cmds = getenv("OPONS_VOXD_COMMANDS");
    g_app.commands_on = (cmds && strcmp(cmds, "1") == 0);
    fprintf(stderr, "voice commands: %s\n",
            g_app.commands_on ? "enabled" : "disabled");
    persist = getenv("OPONS_VOXD_NOTIFY_PERSIST");
    g_app.notify_persist =
        (persist && strcmp(persist, "1") == 0);
    fprintf(stderr, "notifications: %s\n",
            g_app.notify_persist
                ? "persistent" : "transient");
    g_app.notify_mode = NOTIFY_QUIET;
    mode = getenv("OPONS_VOXD_NOTIFY");
    if (mode && *mode) {
        if (strcmp(mode, "normal") == 0) {
            g_app.notify_mode = NOTIFY_NORMAL;
            mode_label = "normal";
        } else if (strcmp(mode, "quiet") == 0) {
            g_app.notify_mode = NOTIFY_QUIET;
            mode_label = "quiet";
        } else if (strcmp(mode, "silent") == 0) {
            g_app.notify_mode = NOTIFY_SILENT;
            mode_label = "silent";
        } else if (strcmp(mode, "off") == 0) {
            g_app.notify_mode = NOTIFY_OFF;
            mode_label = "off";
        } else {
            fprintf(stderr,
                    "OPONS_VOXD_NOTIFY: unknown value '%s', "
                    "falling back to 'quiet'\n", mode);
        }
    }
    fprintf(stderr, "notify mode: %s "
            "(errors always force normal)\n", mode_label);
}

static void build_menu(void)
{
    GtkWidget *toggle;
    GtkWidget *quit;

    g_app.menu = gtk_menu_new();
    toggle = gtk_menu_item_new_with_label(
        "Toggle recording");
    quit = gtk_menu_item_new_with_label("Quit");
    g_signal_connect(toggle, "activate",
                     G_CALLBACK(on_toggle), NULL);
    g_signal_connect(quit, "activate",
                     G_CALLBACK(on_quit), NULL);
    gtk_menu_shell_append(
        GTK_MENU_SHELL(g_app.menu), toggle);
    gtk_menu_shell_append(
        GTK_MENU_SHELL(g_app.menu), quit);
    gtk_widget_show_all(g_app.menu);
}

static void build_tray(void)
{
    g_app.icon_idle =
        make_icon(1.0, 1.0, 1.0, FALSE);
    g_app.icon_recording =
        make_icon(0.94, 0.33, 0.31, TRUE);
    g_app.icon_processing =
        make_icon(1.0, 0.65, 0.15, TRUE);
    g_app.status_icon =
        gtk_status_icon_new_from_pixbuf(g_app.icon_idle);
    gtk_status_icon_set_tooltip_text(
        g_app.status_icon, "opons-voxd");
    gtk_status_icon_set_visible(g_app.status_icon, TRUE);
    g_signal_connect(g_app.status_icon, "activate",
                     G_CALLBACK(on_activate), NULL);
    g_signal_connect(g_app.status_icon, "popup-menu",
                     G_CALLBACK(on_popup), NULL);
}

/* ---- main ---- */

int main(int argc, char **argv)
{
    PaError err;

    setlocale(LC_CTYPE, "");
    if (!XInitThreads()) {
        fprintf(stderr, "XInitThreads failed\n");
        return 1;
    }
    XSetErrorHandler(x_silent_error_handler);
    gtk_init(&argc, &argv);
    if (!notify_init("opons-voxd")) {
        fprintf(stderr, "notify_init failed\n");
        return 1;
    }
    if (acquire_instance_lock() != 0)
        return 1;
    init_lang();
    init_device();
    init_options();
    load_commands();
    g_app.audio_buf = calloc(AUDIO_CAPACITY,
                             sizeof(*g_app.audio_buf));
    if (!g_app.audio_buf) {
        fprintf(stderr, "cannot allocate audio buffer\n");
        return 1;
    }
    atomic_store(&g_app.audio_len, 0);
    atomic_store(&g_app.state, STATE_IDLE);
    err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "Pa_Initialize: %s\n",
                Pa_GetErrorText(err));
        return 1;
    }
    if (init_whisper() != 0) {
        Pa_Terminate();
        return 1;
    }
    init_whisper_translate();
    build_menu();
    build_tray();
    init_hotkey();
    gtk_main();

    /* shutdown */
    if (g_app.stream)
        audio_stop();
    free_hotkey();
    Pa_Terminate();
    if (g_app.wctx_translate && g_app.wctx_translate != g_app.wctx)
        whisper_free(g_app.wctx_translate);
    whisper_free(g_app.wctx);
    free_commands();
    notify_uninit();
    free(g_app.audio_buf);
    g_app.audio_buf = NULL;
    g_object_unref(g_app.icon_idle);
    g_object_unref(g_app.icon_recording);
    g_object_unref(g_app.icon_processing);
    return 0;
}

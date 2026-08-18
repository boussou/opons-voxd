# My Workflow — Dictate in Your Language, Type in English

🌍 **Translations:**
[Français](docs/WORKFLOW.fr.md) · [Deutsch](docs/WORKFLOW.de.md) · [中文](docs/WORKFLOW.zh.md) · [日本語](docs/WORKFLOW.ja.md) · [Español](docs/WORKFLOW.es.md) · [Italiano](docs/WORKFLOW.it.md) · [हिन्दी](docs/WORKFLOW.hi.md) · [العربية](docs/WORKFLOW.ar.md) · [বাংলা](docs/WORKFLOW.bn.md) · [Português](docs/WORKFLOW.pt.md) · [Русский](docs/WORKFLOW.ru.md) · [اردو](docs/WORKFLOW.ur.md) · [Bahasa Indonesia](docs/WORKFLOW.id.md) · [Kiswahili](docs/WORKFLOW.sw.md) · [मराठी](docs/WORKFLOW.mr.md) · [తెలుగు](docs/WORKFLOW.te.md) · [Türkçe](docs/WORKFLOW.tr.md) · [தமிழ்](docs/WORKFLOW.ta.md) · [Tiếng Việt](docs/WORKFLOW.vi.md) · [한국어](docs/WORKFLOW.ko.md)

---

I'm a native French speaker, but most of the software work I do — commit messages, code comments, GitHub issues, chat with international teams, documentation — happens in English. Typing fluent English isn't the hard part; *thinking* in English while the rest of your brain is running in your native language is the actual bottleneck. Dictation removes that bottleneck entirely if you don't have to translate yourself.

## The setup

I run opons-voxd with two independent push-to-talk hotkeys:

- **`Ctrl+Alt+W` — normal dictation.** Hold it, speak French, release it, French text is typed at the cursor. For notes, chat with French colleagues, anything meant to stay in French.
- **`Ctrl+Alt+K` — dictate-and-translate.** Same gesture, but I speak French and *English* comes out at the cursor. For anything English-facing: commit messages, PR descriptions, code comments, GitHub issues, English-speaking chat channels.

No copy-pasting into a translator, no context switch, no re-typing. I speak, English appears where my cursor already is.

## Why two hotkeys, not one

Translation only works well with a model actually trained on the task. I use `large-v3-turbo` for the everyday French hotkey because it's fast — but its translation quality is poor, since it was optimized for transcription speed, not translation. The translate hotkey loads a second model (`medium`) that translates properly instead. Both models sit in GPU memory at once; on a card with more than a few GB of headroom, the extra cost is a non-issue.

## Try it

This is standard opons-voxd configuration — no fork, no patch required. See [Configuration](README.md#configuration) and [Push-to-talk-and-translate hotkey](README.md#push-to-talk-and-translate-hotkey) in the main README for `OPONS_VOXD_PTT_HOTKEY`, `OPONS_VOXD_PTT_TRANSLATE_HOTKEY`, `OPONS_VOXD_TRANSLATE_MODEL`, and `OPONS_VOXD_LANGUAGE`.

If you speak a language other than English day-to-day but work in English, this is probably the single most useful feature in this project.

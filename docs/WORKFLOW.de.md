# Mein Workflow — In deiner Sprache diktieren, auf Englisch tippen

🌍 **Sprachen:**
[English](../WORKFLOW.md) · [Français](WORKFLOW.fr.md) · [中文](WORKFLOW.zh.md) · [日本語](WORKFLOW.ja.md) · [Español](WORKFLOW.es.md) · [Italiano](WORKFLOW.it.md) · [हिन्दी](WORKFLOW.hi.md) · [العربية](WORKFLOW.ar.md) · [বাংলা](WORKFLOW.bn.md) · [Português](WORKFLOW.pt.md) · [Русский](WORKFLOW.ru.md) · [اردو](WORKFLOW.ur.md) · [Bahasa Indonesia](WORKFLOW.id.md) · [Kiswahili](WORKFLOW.sw.md) · [मराठी](WORKFLOW.mr.md) · [తెలుగు](WORKFLOW.te.md) · [Türkçe](WORKFLOW.tr.md) · [தமிழ்](WORKFLOW.ta.md) · [Tiếng Việt](WORKFLOW.vi.md) · [한국어](WORKFLOW.ko.md)

---

Ich bin französischer Muttersprachler, aber der Großteil meiner Software-Arbeit — Commit-Nachrichten, Code-Kommentare, GitHub-Issues, Chats mit internationalen Teams, Dokumentation — findet auf Englisch statt. Flüssig Englisch zu tippen ist nicht das Problem; das eigentliche Nadelöhr ist, auf Englisch zu *denken*, während der Rest des Gehirns in der Muttersprache läuft. Diktieren beseitigt dieses Nadelöhr vollständig, wenn man nicht selbst übersetzen muss.

## Der Aufbau

Ich nutze opons-voxd mit zwei unabhängigen Push-to-Talk-Hotkeys:

- **`Ctrl+Alt+W` — normales Diktat.** Halten, Französisch sprechen, loslassen, französischer Text wird am Cursor eingetippt. Für Notizen, Chats mit französischen Kollegen, alles, was Französisch bleiben soll.
- **`Ctrl+Alt+K` — Diktat-und-Übersetzung.** Gleiche Geste, aber ich spreche Französisch und am Cursor erscheint *Englisch*. Für alles, was sich an ein englischsprachiges Publikum richtet: Commit-Nachrichten, PR-Beschreibungen, Code-Kommentare, GitHub-Issues, englischsprachige Chat-Kanäle.

Kein Kopieren in einen Übersetzer, kein Kontextwechsel, kein Neutippen. Ich spreche, Englisch erscheint dort, wo mein Cursor bereits steht.

## Warum zwei Hotkeys, nicht einer

Übersetzung funktioniert nur gut mit einem Modell, das tatsächlich für diese Aufgabe trainiert wurde. Für den alltäglichen französischen Hotkey nutze ich `large-v3-turbo`, weil es schnell ist — aber seine Übersetzungsqualität ist schlecht, da es auf Transkriptionsgeschwindigkeit optimiert wurde, nicht auf Übersetzung. Der Übersetzungs-Hotkey lädt stattdessen ein zweites Modell (`medium`), das korrekt übersetzt. Beide Modelle liegen gleichzeitig im GPU-Speicher; bei einer Karte mit mehr als ein paar GB Spielraum ist der zusätzliche Aufwand vernachlässigbar.

## Ausprobieren

Das ist Standard-Konfiguration von opons-voxd — kein Fork, kein Patch nötig. Siehe [Configuration](../README.md#configuration) und [Push-to-talk-and-translate hotkey](../README.md#push-to-talk-and-translate-hotkey) im Haupt-README für `OPONS_VOXD_PTT_HOTKEY`, `OPONS_VOXD_PTT_TRANSLATE_HOTKEY`, `OPONS_VOXD_TRANSLATE_MODEL` und `OPONS_VOXD_LANGUAGE`.

Wenn du im Alltag eine andere Sprache als Englisch sprichst, aber auf Englisch arbeitest, ist das wahrscheinlich das nützlichste Feature dieses Projekts.

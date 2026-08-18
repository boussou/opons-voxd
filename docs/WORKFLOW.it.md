# Il mio flusso di lavoro — Detta nella tua lingua, scrivi in inglese

🌍 **Lingue:**
[English](../WORKFLOW.md) · [Français](WORKFLOW.fr.md) · [Deutsch](WORKFLOW.de.md) · [中文](WORKFLOW.zh.md) · [日本語](WORKFLOW.ja.md) · [Español](WORKFLOW.es.md) · [हिन्दी](WORKFLOW.hi.md) · [العربية](WORKFLOW.ar.md) · [বাংলা](WORKFLOW.bn.md) · [Português](WORKFLOW.pt.md) · [Русский](WORKFLOW.ru.md) · [اردو](WORKFLOW.ur.md) · [Bahasa Indonesia](WORKFLOW.id.md) · [Kiswahili](WORKFLOW.sw.md) · [मराठी](WORKFLOW.mr.md) · [తెలుగు](WORKFLOW.te.md) · [Türkçe](WORKFLOW.tr.md) · [தமிழ்](WORKFLOW.ta.md) · [Tiếng Việt](WORKFLOW.vi.md) · [한국어](WORKFLOW.ko.md)

---

Sono madrelingua francese, ma la maggior parte del mio lavoro con il software — messaggi di commit, commenti al codice, issue su GitHub, chat con team internazionali, documentazione — avviene in inglese. Scrivere un inglese fluente non è il problema; il vero collo di bottiglia è *pensare* in inglese mentre il resto del cervello funziona nella lingua madre. La dettatura elimina del tutto questo collo di bottiglia se non devi tradurre tu stesso.

## La configurazione

Uso opons-voxd con due scorciatoie push-to-talk indipendenti:

- **`Ctrl+Alt+W` — dettatura normale.** La tengo premuta, parlo in francese, la rilascio, il testo francese viene digitato al cursore. Per appunti, chat con colleghi francofoni, tutto ciò che deve restare in francese.
- **`Ctrl+Alt+K` — dettatura e traduzione.** Stesso gesto, ma parlo in francese e al cursore compare l'*inglese*. Per tutto ciò che è rivolto a un pubblico anglofono: messaggi di commit, descrizioni di PR, commenti al codice, issue su GitHub, canali chat in inglese.

Niente copia-incolla in un traduttore, nessun cambio di contesto, nessuna ridigitazione. Parlo, e l'inglese compare esattamente dove si trova già il cursore.

## Perché due scorciatoie e non una

La traduzione funziona bene solo con un modello effettivamente addestrato per quel compito. Uso `large-v3-turbo` per la scorciatoia francese quotidiana perché è veloce — ma la sua qualità di traduzione è scarsa, essendo ottimizzato per la velocità di trascrizione, non per la traduzione. La scorciatoia di traduzione carica invece un secondo modello (`medium`), che traduce correttamente. Entrambi i modelli restano in memoria GPU contemporaneamente; su una scheda con più di qualche GB di margine, il costo aggiuntivo è trascurabile.

## Provalo

È configurazione standard di opons-voxd — nessun fork, nessuna patch necessaria. Consulta [Configuration](../README.md#configuration) e [Push-to-talk-and-translate hotkey](../README.md#push-to-talk-and-translate-hotkey) nel README principale per `OPONS_VOXD_PTT_HOTKEY`, `OPONS_VOXD_PTT_TRANSLATE_HOTKEY`, `OPONS_VOXD_TRANSLATE_MODEL` e `OPONS_VOXD_LANGUAGE`.

Se ogni giorno parli una lingua diversa dall'inglese ma lavori in inglese, questa è probabilmente la funzionalità più utile dell'intero progetto.

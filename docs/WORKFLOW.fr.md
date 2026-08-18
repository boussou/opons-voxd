# Mon workflow — Dicter dans sa langue, taper en anglais

🌍 **Langues :**
[English](../WORKFLOW.md) · [Deutsch](WORKFLOW.de.md) · [中文](WORKFLOW.zh.md) · [日本語](WORKFLOW.ja.md) · [Español](WORKFLOW.es.md) · [Italiano](WORKFLOW.it.md) · [हिन्दी](WORKFLOW.hi.md) · [العربية](WORKFLOW.ar.md) · [বাংলা](WORKFLOW.bn.md) · [Português](WORKFLOW.pt.md) · [Русский](WORKFLOW.ru.md) · [اردو](WORKFLOW.ur.md) · [Bahasa Indonesia](WORKFLOW.id.md) · [Kiswahili](WORKFLOW.sw.md) · [मराठी](WORKFLOW.mr.md) · [తెలుగు](WORKFLOW.te.md) · [Türkçe](WORKFLOW.tr.md) · [தமிழ்](WORKFLOW.ta.md) · [Tiếng Việt](WORKFLOW.vi.md) · [한국어](WORKFLOW.ko.md)

---

Je suis francophone de naissance, mais l'essentiel de mon travail logiciel — messages de commit, commentaires de code, issues GitHub, discussions avec des équipes internationales, documentation — se fait en anglais. Taper un anglais correct n'est pas le problème ; c'est *penser* en anglais pendant que le reste du cerveau tourne dans sa langue maternelle qui est le vrai goulot d'étranglement. La dictée supprime entièrement ce goulot d'étranglement si vous n'avez pas à traduire vous-même.

## La configuration

J'utilise opons-voxd avec deux raccourcis push-to-talk indépendants :

- **`Ctrl+Alt+W` — dictée normale.** Je maintiens, je parle en français, je relâche, le texte français est tapé au curseur. Pour les notes, les discussions avec des collègues francophones, tout ce qui doit rester en français.
- **`Ctrl+Alt+K` — dictée-et-traduction.** Même geste, mais je parle en français et c'est de l'*anglais* qui sort au curseur. Pour tout ce qui s'adresse à un public anglophone : messages de commit, descriptions de PR, commentaires de code, issues GitHub, canaux de discussion anglophones.

Pas de copier-coller dans un traducteur, pas de changement de contexte, pas de retaper. Je parle, l'anglais apparaît là où se trouve déjà mon curseur.

## Pourquoi deux raccourcis, pas un seul

La traduction ne fonctionne bien qu'avec un modèle réellement entraîné pour cette tâche. J'utilise `large-v3-turbo` pour le raccourci français du quotidien parce qu'il est rapide — mais sa qualité de traduction est mauvaise, car il a été optimisé pour la vitesse de transcription, pas pour la traduction. Le raccourci de traduction charge un second modèle (`medium`) qui traduit correctement à la place. Les deux modèles tiennent en mémoire GPU en même temps ; sur une carte avec plus de quelques Go de marge, ce coût supplémentaire est négligeable.

## À essayer

C'est une configuration standard d'opons-voxd — aucun fork, aucun patch nécessaire. Voir [Configuration](../README.md#configuration) et [Push-to-talk-and-translate hotkey](../README.md#push-to-talk-and-translate-hotkey) dans le README principal pour `OPONS_VOXD_PTT_HOTKEY`, `OPONS_VOXD_PTT_TRANSLATE_HOTKEY`, `OPONS_VOXD_TRANSLATE_MODEL` et `OPONS_VOXD_LANGUAGE`.

Si vous parlez au quotidien une autre langue que l'anglais mais travaillez en anglais, c'est probablement la fonctionnalité la plus utile de ce projet.

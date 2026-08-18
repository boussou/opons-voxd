# Meu fluxo de trabalho — Dite no seu idioma, digite em inglês

🌍 **Idiomas:**
[English](../WORKFLOW.md) · [Français](WORKFLOW.fr.md) · [Deutsch](WORKFLOW.de.md) · [中文](WORKFLOW.zh.md) · [日本語](WORKFLOW.ja.md) · [Español](WORKFLOW.es.md) · [Italiano](WORKFLOW.it.md) · [हिन्दी](WORKFLOW.hi.md) · [العربية](WORKFLOW.ar.md) · [বাংলা](WORKFLOW.bn.md) · [Русский](WORKFLOW.ru.md) · [اردو](WORKFLOW.ur.md) · [Bahasa Indonesia](WORKFLOW.id.md) · [Kiswahili](WORKFLOW.sw.md) · [मराठी](WORKFLOW.mr.md) · [తెలుగు](WORKFLOW.te.md) · [Türkçe](WORKFLOW.tr.md) · [தமிழ்](WORKFLOW.ta.md) · [Tiếng Việt](WORKFLOW.vi.md) · [한국어](WORKFLOW.ko.md)

---

Sou francófono nativo, mas a maior parte do meu trabalho com software — mensagens de commit, comentários de código, issues no GitHub, conversas com equipes internacionais, documentação — acontece em inglês. Digitar um inglês fluente não é o difícil; o verdadeiro gargalo é *pensar* em inglês enquanto o resto do cérebro funciona no idioma materno. O ditado elimina completamente esse gargalo quando você não precisa traduzir sozinho.

## A configuração

Uso o opons-voxd com dois atalhos push-to-talk independentes:

- **`Ctrl+Alt+W` — ditado normal.** Seguro, falo em francês, solto, o texto em francês é digitado no cursor. Para anotações, conversas com colegas francófonos, tudo o que deve permanecer em francês.
- **`Ctrl+Alt+K` — ditar e traduzir.** Mesmo gesto, mas falo em francês e é *inglês* que aparece no cursor. Para tudo voltado a um público de língua inglesa: mensagens de commit, descrições de PR, comentários de código, issues no GitHub, canais de chat em inglês.

Sem copiar e colar em um tradutor, sem trocar de contexto, sem redigitar. Eu falo, e o inglês aparece exatamente onde já está o meu cursor.

## Por que dois atalhos, não um

A tradução só funciona bem com um modelo realmente treinado para essa tarefa. Uso o `large-v3-turbo` no atalho francês do dia a dia porque é rápido — mas sua qualidade de tradução é ruim, já que foi otimizado para velocidade de transcrição, não para tradução. O atalho de tradução carrega, em vez disso, um segundo modelo (`medium`), que traduz corretamente. Os dois modelos ficam na memória da GPU ao mesmo tempo; em uma placa com mais de alguns GB de folga, esse custo extra é irrelevante.

## Experimente

Isso é configuração padrão do opons-voxd — nenhum fork, nenhum patch necessário. Veja [Configuration](../README.md#configuration) e [Push-to-talk-and-translate hotkey](../README.md#push-to-talk-and-translate-hotkey) no README principal para `OPONS_VOXD_PTT_HOTKEY`, `OPONS_VOXD_PTT_TRANSLATE_HOTKEY`, `OPONS_VOXD_TRANSLATE_MODEL` e `OPONS_VOXD_LANGUAGE`.

Se no seu dia a dia você fala um idioma diferente do inglês, mas trabalha em inglês, esse é provavelmente o recurso mais útil deste projeto.

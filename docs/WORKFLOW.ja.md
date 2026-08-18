# 私のワークフロー — 自分の言語で話し、英語で入力する

🌍 **言語：**
[English](../WORKFLOW.md) · [Français](WORKFLOW.fr.md) · [Deutsch](WORKFLOW.de.md) · [中文](WORKFLOW.zh.md) · [Español](WORKFLOW.es.md) · [Italiano](WORKFLOW.it.md) · [हिन्दी](WORKFLOW.hi.md) · [العربية](WORKFLOW.ar.md) · [বাংলা](WORKFLOW.bn.md) · [Português](WORKFLOW.pt.md) · [Русский](WORKFLOW.ru.md) · [اردو](WORKFLOW.ur.md) · [Bahasa Indonesia](WORKFLOW.id.md) · [Kiswahili](WORKFLOW.sw.md) · [मराठी](WORKFLOW.mr.md) · [తెలుగు](WORKFLOW.te.md) · [Türkçe](WORKFLOW.tr.md) · [தமிழ்](WORKFLOW.ta.md) · [Tiếng Việt](WORKFLOW.vi.md) · [한국어](WORKFLOW.ko.md)

---

私はフランス語のネイティブスピーカーですが、コミットメッセージ、コードコメント、GitHub の issue、国際チームとのチャット、ドキュメントなど、ソフトウェア作業の大半は英語で行われます。流暢な英語を入力すること自体は難しくありません。本当のボトルネックは、脳の他の部分が母語で動いている間に英語で*考える*ことです。自分で翻訳する必要がなければ、口述筆記はこのボトルネックを完全に取り除いてくれます。

## セットアップ

opons-voxd を、独立した2つのプッシュ・トゥ・トークのホットキーで使っています。

- **`Ctrl+Alt+W` — 通常の口述筆記。** 押しながらフランス語で話し、離すとフランス語のテキストがカーソル位置に入力されます。メモ、フランス語圏の同僚とのチャットなど、フランス語のままにしたい内容に使います。
- **`Ctrl+Alt+K` — 口述筆記と翻訳。** 操作は同じですが、フランス語で話すとカーソル位置には*英語*が入力されます。コミットメッセージ、PR の説明、コードコメント、GitHub の issue、英語のチャットチャンネルなど、英語圏向けの内容に使います。

翻訳ツールへのコピー&ペーストも、コンテキストの切り替えも、打ち直しも不要です。話すだけで、カーソルの位置に英語が現れます。

## なぜホットキーが1つではなく2つなのか

翻訳は、そのタスク用に実際に学習されたモデルでなければうまく機能しません。日常のフランス語用ホットキーには `large-v3-turbo` を使っています。速いからです——ただし翻訳の品質は低く、これは文字起こしの速度用に最適化されたモデルであり、翻訳用ではないためです。翻訳用ホットキーは代わりに2つ目のモデル（`medium`）を読み込み、正しく翻訳します。2つのモデルは同時に GPU メモリ上に置かれますが、数 GB 以上の余裕があるカードであれば、この追加コストは問題になりません。

## 試してみる

これは opons-voxd の標準的な設定であり、フォークもパッチも不要です。`OPONS_VOXD_PTT_HOTKEY`、`OPONS_VOXD_PTT_TRANSLATE_HOTKEY`、`OPONS_VOXD_TRANSLATE_MODEL`、`OPONS_VOXD_LANGUAGE` については、メインの README の [Configuration](../README.md#configuration) と [Push-to-talk-and-translate hotkey](../README.md#push-to-talk-and-translate-hotkey) を参照してください。

日常的に英語以外の言語を話しつつ、仕事は英語で行っているなら、これはおそらくこのプロジェクトで最も便利な機能です。

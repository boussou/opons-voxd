# 我的工作流程 — 用母语口述，输出英文

🌍 **语言：**
[English](../WORKFLOW.md) · [Français](WORKFLOW.fr.md) · [Deutsch](WORKFLOW.de.md) · [日本語](WORKFLOW.ja.md) · [Español](WORKFLOW.es.md) · [Italiano](WORKFLOW.it.md) · [हिन्दी](WORKFLOW.hi.md) · [العربية](WORKFLOW.ar.md) · [বাংলা](WORKFLOW.bn.md) · [Português](WORKFLOW.pt.md) · [Русский](WORKFLOW.ru.md) · [اردو](WORKFLOW.ur.md) · [Bahasa Indonesia](WORKFLOW.id.md) · [Kiswahili](WORKFLOW.sw.md) · [मराठी](WORKFLOW.mr.md) · [తెలుగు](WORKFLOW.te.md) · [Türkçe](WORKFLOW.tr.md) · [தமிழ்](WORKFLOW.ta.md) · [Tiếng Việt](WORKFLOW.vi.md) · [한국어](WORKFLOW.ko.md)

---

我的母语是法语，但我大部分的软件工作——提交信息、代码注释、GitHub issue、与国际团队的聊天、文档——都是用英语完成的。流利地打出英语并不难；真正的瓶颈在于，当大脑其余部分都在用母语运转时，还要用英语*思考*。如果不需要自己动手翻译，口述就能彻底消除这个瓶颈。

## 配置

我使用两个独立的一键通话（push-to-talk）快捷键运行 opons-voxd：

- **`Ctrl+Alt+W` — 普通口述。** 按住，说法语，松开，法语文本就会在光标处输入。用于笔记、与法语同事聊天，任何需要保留法语的场合。
- **`Ctrl+Alt+K` — 口述并翻译。** 同样的操作，但我说法语，光标处出现的却是*英文*。用于任何面向英语读者的内容：提交信息、PR 描述、代码注释、GitHub issue、英语聊天频道。

不用复制粘贴到翻译工具，不用切换上下文，不用重新打字。我一开口，英文就出现在光标所在的位置。

## 为什么用两个快捷键，而不是一个

翻译只有在使用真正针对该任务训练过的模型时才能表现良好。我在日常法语快捷键上使用 `large-v3-turbo`，因为它速度快——但它的翻译质量很差，因为它是为转录速度而优化的，不是为翻译。翻译快捷键改为加载第二个模型（`medium`），才能正确翻译。两个模型会同时占用 GPU 显存；如果显卡有几 GB 以上的余量，这点额外开销完全可以忽略。

## 试一试

这是 opons-voxd 的标准配置——无需 fork，无需打补丁。关于 `OPONS_VOXD_PTT_HOTKEY`、`OPONS_VOXD_PTT_TRANSLATE_HOTKEY`、`OPONS_VOXD_TRANSLATE_MODEL` 和 `OPONS_VOXD_LANGUAGE`，请参见主 README 中的 [Configuration](../README.md#configuration) 和 [Push-to-talk-and-translate hotkey](../README.md#push-to-talk-and-translate-hotkey)。

如果你日常说的是英语以外的语言，但工作需要用英语，这大概是本项目中最实用的功能。

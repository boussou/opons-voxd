# 내 워크플로 — 모국어로 말하면, 영어로 입력된다

🌍 **언어:**
[English](../WORKFLOW.md) · [Français](WORKFLOW.fr.md) · [Deutsch](WORKFLOW.de.md) · [中文](WORKFLOW.zh.md) · [日本語](WORKFLOW.ja.md) · [Español](WORKFLOW.es.md) · [Italiano](WORKFLOW.it.md) · [हिन्दी](WORKFLOW.hi.md) · [العربية](WORKFLOW.ar.md) · [বাংলা](WORKFLOW.bn.md) · [Português](WORKFLOW.pt.md) · [Русский](WORKFLOW.ru.md) · [اردو](WORKFLOW.ur.md) · [Bahasa Indonesia](WORKFLOW.id.md) · [Kiswahili](WORKFLOW.sw.md) · [मराठी](WORKFLOW.mr.md) · [తెలుగు](WORKFLOW.te.md) · [Türkçe](WORKFLOW.tr.md) · [தமிழ்](WORKFLOW.ta.md) · [Tiếng Việt](WORKFLOW.vi.md)

---

저는 프랑스어가 모국어이지만, 커밋 메시지, 코드 주석, GitHub 이슈, 국제 팀과의 채팅, 문서 작업 등 소프트웨어 작업 대부분은 영어로 이루어집니다. 유창한 영어를 타이핑하는 것 자체는 어렵지 않습니다. 진짜 병목은 나머지 뇌가 모국어로 돌아가는 동안 영어로 *생각*해야 한다는 점입니다. 직접 번역할 필요가 없다면 받아쓰기는 이 병목을 완전히 없애줍니다.

## 설정

저는 opons-voxd를 서로 독립적인 두 개의 푸시투토크 단축키로 사용합니다.

- **`Ctrl+Alt+W` — 일반 받아쓰기.** 누르고 있는 동안 프랑스어로 말하고, 놓으면 커서 위치에 프랑스어 텍스트가 입력됩니다. 메모, 프랑스어권 동료와의 채팅 등 프랑스어로 남아 있어야 하는 모든 것에 사용합니다.
- **`Ctrl+Alt+K` — 받아쓰기 및 번역.** 동일한 동작이지만, 프랑스어로 말하면 커서 위치에 *영어*가 나타납니다. 커밋 메시지, PR 설명, 코드 주석, GitHub 이슈, 영어 채팅 채널 등 영어권 독자를 대상으로 하는 모든 것에 사용합니다.

번역기에 복사-붙여넣기도, 컨텍스트 전환도, 다시 타이핑할 필요도 없습니다. 제가 말하면, 커서가 이미 있는 바로 그 자리에 영어가 나타납니다.

## 왜 단축키가 하나가 아니라 둘인가

번역은 실제로 그 작업을 위해 학습된 모델에서만 잘 작동합니다. 일상적인 프랑스어 단축키에는 속도가 빠른 `large-v3-turbo`를 사용하지만, 이 모델은 전사(transcription) 속도에 최적화되어 있고 번역용으로 최적화된 것이 아니라서 번역 품질이 좋지 않습니다. 대신 번역 단축키는 제대로 번역하는 두 번째 모델(`medium`)을 로드합니다. 두 모델은 동시에 GPU 메모리에 올라가 있습니다. 여유 공간이 몇 GB 이상인 그래픽카드라면 이 추가 비용은 문제가 되지 않습니다.

## 사용해 보기

이것은 opons-voxd의 표준 설정이며, 포크나 패치가 필요 없습니다. `OPONS_VOXD_PTT_HOTKEY`, `OPONS_VOXD_PTT_TRANSLATE_HOTKEY`, `OPONS_VOXD_TRANSLATE_MODEL`, `OPONS_VOXD_LANGUAGE`에 대해서는 메인 README의 [Configuration](../README.md#configuration)과 [Push-to-talk-and-translate hotkey](../README.md#push-to-talk-and-translate-hotkey)를 참고하세요.

평소에는 영어가 아닌 언어를 쓰지만 업무는 영어로 한다면, 이것이 아마 이 프로젝트에서 가장 유용한 기능일 것입니다.

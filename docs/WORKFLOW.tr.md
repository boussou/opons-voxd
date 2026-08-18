# Benim İş Akışım — Kendi Dilinde Konuş, İngilizce Yazılsın

🌍 **Diller:**
[English](../WORKFLOW.md) · [Français](WORKFLOW.fr.md) · [Deutsch](WORKFLOW.de.md) · [中文](WORKFLOW.zh.md) · [日本語](WORKFLOW.ja.md) · [Español](WORKFLOW.es.md) · [Italiano](WORKFLOW.it.md) · [हिन्दी](WORKFLOW.hi.md) · [العربية](WORKFLOW.ar.md) · [বাংলা](WORKFLOW.bn.md) · [Português](WORKFLOW.pt.md) · [Русский](WORKFLOW.ru.md) · [اردو](WORKFLOW.ur.md) · [Bahasa Indonesia](WORKFLOW.id.md) · [Kiswahili](WORKFLOW.sw.md) · [मराठी](WORKFLOW.mr.md) · [తెలుగు](WORKFLOW.te.md) · [தமிழ்](WORKFLOW.ta.md) · [Tiếng Việt](WORKFLOW.vi.md) · [한국어](WORKFLOW.ko.md)

---

Ana dilim Fransızca, ama yazılım işimin çoğu — commit mesajları, kod yorumları, GitHub issue'ları, uluslararası ekiplerle sohbet, dokümantasyon — İngilizce yapılıyor. Akıcı İngilizce yazmak zor değil; asıl darboğaz, beynin geri kalanı ana dilde çalışırken İngilizce *düşünmek*. Kendi çevirini yapmak zorunda kalmıyorsan dikte bu darboğazı tamamen ortadan kaldırıyor.

## Kurulum

opons-voxd'yi iki bağımsız push-to-talk kısayoluyla kullanıyorum:

- **`Ctrl+Alt+W` — normal dikte.** Basılı tutuyorum, Fransızca konuşuyorum, bırakıyorum, imleçte Fransızca metin yazılıyor. Notlar, Fransızca konuşan meslektaşlarla sohbet ve Fransızca kalması gereken her şey için.
- **`Ctrl+Alt+K` — dikte-ve-çeviri.** Aynı hareket, ama Fransızca konuşuyorum ve imleçte *İngilizce* çıkıyor. İngilizce konuşan bir kitleye yönelik her şey için: commit mesajları, PR açıklamaları, kod yorumları, GitHub issue'ları, İngilizce sohbet kanalları.

Bir çeviriciye kopyala-yapıştır yok, bağlam değiştirme yok, yeniden yazma yok. Konuşuyorum ve İngilizce, imlecimin zaten bulunduğu yerde beliriyor.

## Neden bir değil de iki kısayol

Çeviri, yalnızca o görev için gerçekten eğitilmiş bir modelle iyi çalışır. Günlük Fransızca kısayolu için `large-v3-turbo` kullanıyorum çünkü hızlı — ama çeviri kalitesi kötü, çünkü transkripsiyon hızı için optimize edilmiş, çeviri için değil. Çeviri kısayolu bunun yerine doğru çeviren ikinci bir model (`medium`) yüklüyor. İki model de aynı anda GPU belleğinde duruyor; birkaç GB'tan fazla boşluğu olan bir kartta bu ek maliyet önemsiz.

## Deneyin

Bu, opons-voxd'nin standart yapılandırmasıdır — fork veya yama gerekmez. `OPONS_VOXD_PTT_HOTKEY`, `OPONS_VOXD_PTT_TRANSLATE_HOTKEY`, `OPONS_VOXD_TRANSLATE_MODEL` ve `OPONS_VOXD_LANGUAGE` için ana README'deki [Configuration](../README.md#configuration) ve [Push-to-talk-and-translate hotkey](../README.md#push-to-talk-and-translate-hotkey) bölümlerine bakın.

Günlük hayatta İngilizce dışında bir dil konuşup İngilizce çalışıyorsanız, bu muhtemelen bu projedeki en kullanışlı özellik.

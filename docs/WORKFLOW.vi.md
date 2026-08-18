# Quy trình làm việc của tôi — Đọc bằng ngôn ngữ của bạn, gõ ra tiếng Anh

🌍 **Ngôn ngữ:**
[English](../WORKFLOW.md) · [Français](WORKFLOW.fr.md) · [Deutsch](WORKFLOW.de.md) · [中文](WORKFLOW.zh.md) · [日本語](WORKFLOW.ja.md) · [Español](WORKFLOW.es.md) · [Italiano](WORKFLOW.it.md) · [हिन्दी](WORKFLOW.hi.md) · [العربية](WORKFLOW.ar.md) · [বাংলা](WORKFLOW.bn.md) · [Português](WORKFLOW.pt.md) · [Русский](WORKFLOW.ru.md) · [اردو](WORKFLOW.ur.md) · [Bahasa Indonesia](WORKFLOW.id.md) · [Kiswahili](WORKFLOW.sw.md) · [मराठी](WORKFLOW.mr.md) · [తెలుగు](WORKFLOW.te.md) · [Türkçe](WORKFLOW.tr.md) · [தமிழ்](WORKFLOW.ta.md) · [한국어](WORKFLOW.ko.md)

---

Tiếng mẹ đẻ của tôi là tiếng Pháp, nhưng phần lớn công việc phần mềm của tôi — tin nhắn commit, chú thích mã nguồn, issue trên GitHub, trò chuyện với các nhóm quốc tế, tài liệu — đều bằng tiếng Anh. Gõ tiếng Anh trôi chảy không phải là điều khó; nút thắt cổ chai thực sự là *suy nghĩ* bằng tiếng Anh trong khi phần còn lại của não vẫn đang vận hành bằng tiếng mẹ đẻ. Đọc chính tả loại bỏ hoàn toàn nút thắt này nếu bạn không phải tự dịch.

## Thiết lập

Tôi chạy opons-voxd với hai phím tắt push-to-talk độc lập:

- **`Ctrl+Alt+W` — đọc chính tả bình thường.** Giữ phím, nói tiếng Pháp, thả ra, văn bản tiếng Pháp được gõ tại vị trí con trỏ. Dùng cho ghi chú, trò chuyện với đồng nghiệp nói tiếng Pháp, bất cứ điều gì cần giữ nguyên bằng tiếng Pháp.
- **`Ctrl+Alt+K` — đọc chính tả và dịch.** Cùng thao tác, nhưng tôi nói tiếng Pháp và *tiếng Anh* hiện ra ở con trỏ. Dùng cho bất cứ điều gì hướng tới người đọc nói tiếng Anh: tin nhắn commit, mô tả PR, chú thích mã nguồn, issue trên GitHub, kênh trò chuyện tiếng Anh.

Không cần sao chép-dán vào công cụ dịch, không cần chuyển ngữ cảnh, không cần gõ lại. Tôi nói, và tiếng Anh xuất hiện ngay tại vị trí con trỏ của tôi.

## Vì sao là hai phím tắt, không phải một

Việc dịch chỉ hoạt động tốt với một mô hình thực sự được huấn luyện cho tác vụ đó. Tôi dùng `large-v3-turbo` cho phím tắt tiếng Pháp hằng ngày vì nó nhanh — nhưng chất lượng dịch của nó kém, vì nó được tối ưu cho tốc độ phiên âm, không phải dịch thuật. Thay vào đó, phím tắt dịch sẽ tải một mô hình thứ hai (`medium`) dịch đúng cách. Cả hai mô hình cùng nằm trong bộ nhớ GPU một lúc; trên một card có dư hơn vài GB, chi phí thêm này không đáng kể.

## Hãy thử

Đây là cấu hình tiêu chuẩn của opons-voxd — không cần fork, không cần vá lỗi. Xem [Configuration](../README.md#configuration) và [Push-to-talk-and-translate hotkey](../README.md#push-to-talk-and-translate-hotkey) trong README chính để biết về `OPONS_VOXD_PTT_HOTKEY`, `OPONS_VOXD_PTT_TRANSLATE_HOTKEY`, `OPONS_VOXD_TRANSLATE_MODEL`, và `OPONS_VOXD_LANGUAGE`.

Nếu hằng ngày bạn nói một ngôn ngữ khác tiếng Anh nhưng làm việc bằng tiếng Anh, đây có lẽ là tính năng hữu ích nhất trong dự án này.

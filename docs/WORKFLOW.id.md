# Alur Kerja Saya — Berbicara dalam Bahasa Sendiri, Diketik dalam Bahasa Inggris

🌍 **Bahasa:**
[English](../WORKFLOW.md) · [Français](WORKFLOW.fr.md) · [Deutsch](WORKFLOW.de.md) · [中文](WORKFLOW.zh.md) · [日本語](WORKFLOW.ja.md) · [Español](WORKFLOW.es.md) · [Italiano](WORKFLOW.it.md) · [हिन्दी](WORKFLOW.hi.md) · [العربية](WORKFLOW.ar.md) · [বাংলা](WORKFLOW.bn.md) · [Português](WORKFLOW.pt.md) · [Русский](WORKFLOW.ru.md) · [اردو](WORKFLOW.ur.md) · [Kiswahili](WORKFLOW.sw.md) · [मराठी](WORKFLOW.mr.md) · [తెలుగు](WORKFLOW.te.md) · [Türkçe](WORKFLOW.tr.md) · [தமிழ்](WORKFLOW.ta.md) · [Tiếng Việt](WORKFLOW.vi.md) · [한국어](WORKFLOW.ko.md)

---

Bahasa ibu saya adalah bahasa Prancis, tetapi sebagian besar pekerjaan perangkat lunak saya — pesan commit, komentar kode, issue GitHub, obrolan dengan tim internasional, dokumentasi — dilakukan dalam bahasa Inggris. Mengetik bahasa Inggris yang lancar bukan masalahnya; hambatan sesungguhnya adalah *berpikir* dalam bahasa Inggris sementara bagian otak lainnya bekerja dalam bahasa ibu. Dikte menghilangkan hambatan ini sepenuhnya jika Anda tidak perlu menerjemahkan sendiri.

## Pengaturan

Saya menjalankan opons-voxd dengan dua hotkey push-to-talk yang independen:

- **`Ctrl+Alt+W` — dikte normal.** Tahan, bicara dalam bahasa Prancis, lepaskan, teks Prancis diketik di kursor. Untuk catatan, obrolan dengan rekan berbahasa Prancis, apa pun yang harus tetap dalam bahasa Prancis.
- **`Ctrl+Alt+K` — dikte-dan-terjemahkan.** Gerakan yang sama, tetapi saya berbicara bahasa Prancis dan yang muncul di kursor adalah *bahasa Inggris*. Untuk apa pun yang ditujukan bagi pembaca berbahasa Inggris: pesan commit, deskripsi PR, komentar kode, issue GitHub, kanal obrolan berbahasa Inggris.

Tidak perlu menyalin-tempel ke penerjemah, tidak perlu berpindah konteks, tidak perlu mengetik ulang. Saya berbicara, dan bahasa Inggris muncul tepat di posisi kursor saya.

## Mengapa dua hotkey, bukan satu

Terjemahan hanya bekerja dengan baik dengan model yang benar-benar dilatih untuk tugas tersebut. Saya menggunakan `large-v3-turbo` untuk hotkey bahasa Prancis sehari-hari karena cepat — tetapi kualitas terjemahannya buruk, karena model ini dioptimalkan untuk kecepatan transkripsi, bukan terjemahan. Hotkey terjemahan justru memuat model kedua (`medium`) yang menerjemahkan dengan benar. Kedua model berada di memori GPU secara bersamaan; pada kartu dengan ruang lebih dari beberapa GB, biaya tambahan ini tidak menjadi masalah.

## Coba sendiri

Ini adalah konfigurasi standar opons-voxd — tidak perlu fork atau patch. Lihat [Configuration](../README.md#configuration) dan [Push-to-talk-and-translate hotkey](../README.md#push-to-talk-and-translate-hotkey) di README utama untuk `OPONS_VOXD_PTT_HOTKEY`, `OPONS_VOXD_PTT_TRANSLATE_HOTKEY`, `OPONS_VOXD_TRANSLATE_MODEL`, dan `OPONS_VOXD_LANGUAGE`.

Jika sehari-hari Anda berbicara dalam bahasa selain bahasa Inggris tetapi bekerja dalam bahasa Inggris, ini mungkin fitur paling berguna di seluruh proyek ini.

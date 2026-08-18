# Mi flujo de trabajo — Dicta en tu idioma, escribe en inglés

🌍 **Idiomas:**
[English](../WORKFLOW.md) · [Français](WORKFLOW.fr.md) · [Deutsch](WORKFLOW.de.md) · [中文](WORKFLOW.zh.md) · [日本語](WORKFLOW.ja.md) · [Italiano](WORKFLOW.it.md) · [हिन्दी](WORKFLOW.hi.md) · [العربية](WORKFLOW.ar.md) · [বাংলা](WORKFLOW.bn.md) · [Português](WORKFLOW.pt.md) · [Русский](WORKFLOW.ru.md) · [اردو](WORKFLOW.ur.md) · [Bahasa Indonesia](WORKFLOW.id.md) · [Kiswahili](WORKFLOW.sw.md) · [मराठी](WORKFLOW.mr.md) · [తెలుగు](WORKFLOW.te.md) · [Türkçe](WORKFLOW.tr.md) · [தமிழ்](WORKFLOW.ta.md) · [Tiếng Việt](WORKFLOW.vi.md) · [한국어](WORKFLOW.ko.md)

---

Soy hablante nativo de francés, pero la mayor parte de mi trabajo de software —mensajes de commit, comentarios de código, issues de GitHub, chats con equipos internacionales, documentación— ocurre en inglés. Escribir inglés fluido no es lo difícil; el verdadero cuello de botella es *pensar* en inglés mientras el resto del cerebro funciona en tu idioma materno. El dictado elimina por completo ese cuello de botella si no tienes que traducir tú mismo.

## La configuración

Uso opons-voxd con dos atajos push-to-talk independientes:

- **`Ctrl+Alt+W` — dictado normal.** Lo mantengo pulsado, hablo en francés, lo suelto, el texto en francés se escribe en el cursor. Para notas, chats con colegas francófonos, cualquier cosa que deba quedarse en francés.
- **`Ctrl+Alt+K` — dictar y traducir.** El mismo gesto, pero hablo en francés y lo que sale en el cursor es *inglés*. Para todo lo orientado a un público angloparlante: mensajes de commit, descripciones de PR, comentarios de código, issues de GitHub, canales de chat en inglés.

Sin copiar y pegar en un traductor, sin cambiar de contexto, sin volver a escribir. Hablo, y el inglés aparece justo donde ya está mi cursor.

## Por qué dos atajos y no uno

La traducción solo funciona bien con un modelo realmente entrenado para esa tarea. Uso `large-v3-turbo` para el atajo cotidiano en francés porque es rápido, pero su calidad de traducción es mala, ya que está optimizado para la velocidad de transcripción, no para traducir. El atajo de traducción carga en su lugar un segundo modelo (`medium`) que traduce correctamente. Ambos modelos residen en la memoria de la GPU al mismo tiempo; en una tarjeta con más de unos pocos GB de margen, ese coste adicional es irrelevante.

## Pruébalo

Esto es configuración estándar de opons-voxd — no hace falta ningún fork ni parche. Consulta [Configuration](../README.md#configuration) y [Push-to-talk-and-translate hotkey](../README.md#push-to-talk-and-translate-hotkey) en el README principal para `OPONS_VOXD_PTT_HOTKEY`, `OPONS_VOXD_PTT_TRANSLATE_HOTKEY`, `OPONS_VOXD_TRANSLATE_MODEL` y `OPONS_VOXD_LANGUAGE`.

Si en tu día a día hablas un idioma distinto del inglés pero trabajas en inglés, esta es probablemente la función más útil de todo el proyecto.

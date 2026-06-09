# lvgl_kawaii_face (ESPHome)

ESPHome wrapper around the [lvgl_kawaii_face](../../lvgl_kawaii_face-main) C
component — an animated kawaii face for LVGL 9. It draws eyes, eyebrows, blush
and a mouth on LVGL canvases with per-emotion animations, and exposes the
emotion as an ESPHome **action** so the face can react to your automations,
most notably the **voice assistant** pipeline.

The C source (`lvgl_kawaii_face.c` / `lvgl_kawaii_face.h`) is vendored here so
the component is self-contained when fetched via `external_components`.

## Configuration

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/youkorr/lvgl_9.5
      ref: claude/jolly-lamport-ERZOC
    components: [lvgl, image, font, lvgl_kawaii_face]

# A plain LVGL object the face fills and scales to:
lvgl:
  pages:
    - id: page_face
      widgets:
        - obj:
            id: face_panel
            width: 240
            height: 240
            align: CENTER
            bg_opa: TRANSP
            border_width: 0
            pad_all: 0

lvgl_kawaii_face:
  id: face
  parent_id: face_panel       # optional; omit to fill the active screen
  initial_emotion: neutral
  animation_speed: 30ms       # timer interval (~33 FPS)
  blink_interval: 3000ms
  auto_blink: true
  smooth: true                # smooth transitions between expressions
```

| Option | Default | Description |
|---|---|---|
| `parent_id` | *(screen)* | id of an LVGL widget the face fills. Omit to use the active screen. |
| `animation_speed` | `30ms` | Animation update interval. |
| `blink_interval` | `3000ms` | Auto-blink period. |
| `auto_blink` | `true` | Enable automatic blinking. |
| `smooth` | `true` | Smoothly transition between expressions. |
| `initial_emotion` | `neutral` | Expression shown at boot. |

> The C component is a singleton — configure a single `lvgl_kawaii_face:` block.

## Action: `lvgl_kawaii_face.set_emotion`

```yaml
- lvgl_kawaii_face.set_emotion:
    id: face
    emotion: happy            # templatable (lambda allowed)
```

### Emotions

`neutral`, `happy`, `worried`, `sad`, `surprised`, `angry`, `sleepy`, `wink`,
`love`, `playful`, `silly`, `smirk`, `cry`, `working_hard`, `excited`,
`confused`, `cool`.

Voice-assistant friendly aliases: `idle`→neutral, `listening`→surprised,
`thinking`→working_hard, `speaking`/`talking`→happy, `error`→sad.

## Voice assistant

Wire the actions into the standard `voice_assistant:` triggers — see
[`example.yaml`](example.yaml) for a complete, ready-to-adapt configuration:

```yaml
voice_assistant:
  on_wake_word_detected:
    - lvgl_kawaii_face.set_emotion: { id: face, emotion: excited }
  on_listening:
    - lvgl_kawaii_face.set_emotion: { id: face, emotion: listening }
  on_stt_end:
    - lvgl_kawaii_face.set_emotion: { id: face, emotion: thinking }
  on_tts_start:
    - lvgl_kawaii_face.set_emotion: { id: face, emotion: speaking }
  on_error:
    - lvgl_kawaii_face.set_emotion: { id: face, emotion: error }
  on_end:
    - lvgl_kawaii_face.set_emotion: { id: face, emotion: neutral }
```

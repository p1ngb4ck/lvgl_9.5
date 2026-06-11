# LVGL 9.5 Widgets - Quick Reference

Quick reference guide for all 35 LVGL 9.5 widgets available in ESPHome.

---

## 🎯 Basic Widgets

| Widget | Usage | Minimal Example |
|--------|-------|-----------------|
| **Label** | Display text | `- label: { text: "Hello" }` |
| **Button** | Clickable button | `- button: { text: "Click" }` |
| **Image** | Display image/SVG | `- image: { src: "S:/icon.svg" }` |
| **Object** | Base container | `- obj: { width: 200, height: 100 }` |

---

## 📝 Input Widgets

| Widget | Usage | Key Properties |
|--------|-------|-----------------|
| **Slider** | Value slider | `min_value`, `max_value`, `value` |
| **Switch** | ON/OFF toggle | `state: true/false` |
| **Checkbox** | Checkbox | `checked: true/false`, `text` |
| **Dropdown** | Drop-down list | `options`, `selected_index` |
| **Roller** | iOS-style roller | `options`, `visible_row_count` |
| **Textarea** | Multiline text input | `text`, `placeholder_text`, `max_length` |
| **Keyboard** | Virtual keyboard | `mode: TEXT_LOWER/TEXT_UPPER/NUMBER` |
| **Spinbox** | Numeric +/- input | `min_value`, `max_value`, `step` |

---

## 📊 Display Widgets

| Widget | Usage | Key Properties |
|--------|-------|-----------------|
| **Arc** | Circular gauge | `start_angle`, `end_angle`, `value` |
| **Bar** | Progress bar | `min_value`, `max_value`, `value` |
| **LED** | LED indicator | `color`, `brightness` |
| **Spinner** | Loading indicator | `spin_time`, `arc_length` |
| **Line** | Line/polyline | `points: [{x,y}, ...]` |
| **Scale** ⚡ | Graduated scale | `mode: ROUND_OUTER`, `range`, `angle_range` |
| **Chart** | Chart | `type: LINE/BAR`, `series` |
| **QR Code** | QR code | `data`, `size` |

---

## 🎬 Advanced Widgets

| Widget | Usage | Key Properties |
|--------|-------|-----------------|
| **AnimImg** | Animated images | `images: [...]`, `duration` |
| **Lottie** ⚡ | JSON vector animation | `src`, `loop`, `autoplay` |
| **3D Texture** ⚡ | 3D model | `src`, `angle_x/y/z` |
| **Arc Label** ⚡ | Curved text | `text`, `angle`, `radius` |
| **Span** | Rich text | `spans: [{text, color, font}]` |

---

## 📦 Container Widgets

| Widget | Usage | Key Properties |
|--------|-------|-----------------|
| **TabView** | Tabbed interface | `tabs: [{name, widgets}]` |
| **TileView** | Scrolling views | `tiles: [{row, col, dir}]` |
| **Menu** ⚡ | Hierarchical menu | `pages: [{title, widgets}]` |
| **Window** | Window with title | `title`, `close_button` |
| **List** | List of buttons | `items: [{text, icon}]` |
| **Table** | Rows/columns table | `col_count`, `row_count`, `cells` |
| **Calendar** ⚡ | Monthly calendar | `year`, `month`, `day` |
| **ButtonMatrix** | Grid of buttons | `rows`, `buttons` |
| **MsgBox** | Dialog box | `title`, `text`, `buttons` |
| **Canvas** | Drawing canvas | `width`, `height` |
| **ImageButton** | Image button | `src`, `src_pressed` |

---

## 🎨 Common Style Properties

```yaml
# Position
x: 100          # pixels or %
y: 50
width: 200
height: 100
align: CENTER   # TOP_LEFT, CENTER, BOTTOM_RIGHT, etc.

# Colors
bg_color: 0x2196F3      # Background
text_color: 0xFFFFFF    # Text
border_color: 0x000000  # Border

# Opacity
bg_opa: COVER           # TRANSP, 0-255, COVER

# Border and Corners
border_width: 2
radius: 10              # Rounded corners

# Padding
pad_all: 10
pad_left: 5
pad_right: 5
pad_top: 5
pad_bottom: 5

# Shadow
shadow_width: 10
shadow_color: 0x000000
shadow_opa: 128
```

---

## 🎯 Main Events

```yaml
# Input events
on_press:           # Press begins
on_pressing:        # Continuous press
on_click:           # Single click
on_short_click:     # Short click
on_long_press:      # Long press
on_release:         # Release

# New in LVGL 9.5 ⚡
on_single_click:    # 1st click
on_double_click:    # 2nd click
on_triple_click:    # 3rd click
on_hover_over:      # Hover
on_hover_leave:     # Hover end

# Special events
on_change:          # Value change
on_ready:           # Ready
on_focus:           # Gained focus
on_defocus:         # Lost focus
on_scroll:          # Scroll
```

---

## 🚀 LVGL Actions

```yaml
# Navigation
- lvgl.page.show: page_id
- lvgl.page.next:
- lvgl.page.previous:

# Widget update
- lvgl.widget.update:
    id: widget_id
    text: "New text"
    value: 50

# Lottie control
- lvgl.lottie.start: anim_id
- lvgl.lottie.stop: anim_id
- lvgl.lottie.pause: anim_id

# Widget state
- lvgl.widget.enable: widget_id
- lvgl.widget.disable: widget_id
- lvgl.widget.show: widget_id
- lvgl.widget.hide: widget_id
```

---

## 📐 Layout - Flex

```yaml
layout:
  type: FLEX
  flex_flow: ROW        # ROW, COLUMN, ROW_WRAP, etc.
  flex_align_main: CENTER
  flex_align_cross: CENTER
  flex_align_track: CENTER
```

---

## 📐 Layout - Grid

```yaml
layout:
  type: GRID
  grid_columns: [100, 100, 100]  # Column widths
  grid_rows: [50, 50]            # Row heights
  grid_column_align: CENTER
  grid_row_align: CENTER
```

---

## 🎨 States

```yaml
styles:
  - state: DEFAULT      # Default state
  - state: CHECKED      # Checked
  - state: FOCUSED      # Has focus
  - state: PRESSED      # Pressed
  - state: DISABLED     # Disabled
  - state: HOVERED      # Hovered
  - state: SCROLLED     # Scrolling
  - state: EDITED       # Editing
```

---

## 🎯 Parts

```yaml
styles:
  - part: MAIN          # Main part
  - part: SCROLLBAR     # Scrollbar
  - part: INDICATOR     # Indicator (slider, bar)
  - part: KNOB          # Knob (slider)
  - part: SELECTED      # Selected element
  - part: ITEMS         # Multiple items
  - part: TICKS         # Tick marks (scale)
  - part: CURSOR        # Cursor (textarea)
  - part: HEADER        # Header (menu, win)
  - part: SIDEBAR       # Sidebar (menu)
```

---

## 📏 Units

```yaml
# Pixels
width: 200          # 200 pixels

# Percentage
width: 50%          # 50% of parent

# Content
width: SIZE_CONTENT # Fitted to content
```

---

## 🎯 Flags

```yaml
flags:
  hidden: false         # Hidden
  clickable: true       # Clickable
  scrollable: false     # Scrollable
  checkable: false      # Checkable
  scroll_elastic: true  # Elastic scrolling
  scroll_momentum: true # Scroll momentum
  snappable: false      # Automatic snapping
```

---

## 🖼️ Supported Image Formats

| Format | Extension | Scalable | Animation | Usage |
|--------|-----------|----------|-----------|-------|
| **SVG** ⚡ | `.svg` | ✅ | ❌ | Icons, logos |
| **Lottie** ⚡ | `.json` | ✅ | ✅ | Smooth animations |
| **PNG** | `.png` | ❌ | ❌ | Photos with transparency |
| **JPEG** | `.jpg` | ❌ | ❌ | Photos |
| **BMP** | `.bmp` | ❌ | ❌ | Simple images |
| **GIF** | `.gif` | ❌ | ✅ | Animations (heavy) |

---

## 💾 Loading Images

```yaml
# From SD card
src: "S:/icons/home.svg"

# From an ESPHome image component
image:
  - id: my_img
    file: "images/icon.png"

lvgl:
  widgets:
    - image:
        src: my_img
```

---

## 🎨 Colors

```yaml
# Hexadecimal RGB
color: 0xFF0000     # Red
color: 0x00FF00     # Green
color: 0x0000FF     # Blue
color: 0xFFFFFF     # White
color: 0x000000     # Black

# Material Design colors
color: 0x2196F3     # Blue
color: 0x4CAF50     # Green
color: 0xF44336     # Red
color: 0xFF9800     # Orange
color: 0x9C27B0     # Purple
```

---

## 📝 Fonts

```yaml
# Built-in LVGL fonts
text_font: montserrat_8
text_font: montserrat_10
text_font: montserrat_12
text_font: montserrat_14
text_font: montserrat_16
text_font: montserrat_18
text_font: montserrat_20
text_font: montserrat_24
text_font: montserrat_28
text_font: montserrat_32
text_font: montserrat_48

# Special fonts
text_font: dejavu_16_persian_hebrew
text_font: simsun_16_cjk
text_font: unscii_8
text_font: unscii_16
```

---

## 🎬 Animations

```yaml
animated: true
animation:
  duration: 500ms
  delay: 0ms
  path: LINEAR      # LINEAR, EASE_IN, EASE_OUT,
                   # EASE_IN_OUT, OVERSHOOT, BOUNCE
```

---

## 🔄 Common Modes

### Slider / Bar Mode
- `NORMAL`: Single value
- `SYMMETRICAL`: Centered on 0
- `RANGE`: Two values (min-max)

### Arc Mode
- `NORMAL`: Normal arc
- `REVERSE`: Reversed arc
- `SYMMETRICAL`: Symmetrical

### Roller Mode
- `NORMAL`: Finite list
- `INFINITE`: Infinite loop

### Keyboard Mode
- `TEXT_LOWER`: Lowercase
- `TEXT_UPPER`: Uppercase
- `SPECIAL`: Special characters
- `NUMBER`: Numeric keypad

---

## 📊 Complete Example

```yaml
lvgl:
  log_level: INFO
  color_depth: 16
  displays:
    - my_display
  touchscreens:
    - my_touch

  pages:
    - id: home_page
      widgets:
        # Title
        - label:
            id: title
            text: "Dashboard"
            x: 50%
            y: 20
            align: TOP_MID
            text_font: montserrat_24
            text_color: 0x2196F3

        # Temperature with SVG icon
        - image:
            src: "S:/icons/temp.svg"
            x: 50
            y: 80
            width: 48
            height: 48

        - label:
            id: temp_label
            text: "22.5°C"
            x: 110
            y: 90
            text_font: montserrat_20

        # Slider control
        - slider:
            id: brightness
            x: 50
            y: 150
            width: 300
            min_value: 0
            max_value: 100
            value: 75
            on_change:
              - logger.log:
                  format: "Brightness: %d"
                  args: ['x']

        # Action button
        - button:
            x: 50%
            y: 250
            align: TOP_MID
            width: 150
            height: 50
            text: "Apply"
            bg_color: 0x4CAF50
            on_click:
              - logger.log: "Settings applied"

        # Lottie animation
        - lottie:
            id: loading
            src: "S:/anim/loading.json"
            x: 50%
            y: 350
            align: TOP_MID
            width: 100
            height: 100
            loop: true
            autoplay: true

  # Automation
  on_boot:
    - lvgl.page.show: home_page
```

---

## 🆕 What's new in LVGL 9.5

### New Widgets ⚡
- **Scale**: Replaces Meter (linear/circular scales)
- **Arc Label**: Curved text
- **Lottie**: JSON vector animations at 60 FPS
- **3D Texture**: 3D models with ThorVG
- **Menu**: Hierarchical navigation
- **Calendar**: Interactive calendar

### New Features ⚡
- **ThorVG**: Built-in vector graphics engine
- **Native SVG**: SVG support without external library
- **Performance**: 2x faster rendering
- **New events**: 54 events added (70 total)
- **New states**: `default` state added

---

## 📚 Complete Documentation

- **Complete Guide**: `WIDGETS_GUIDE.md` (35 widgets in detail)
- **Main README**: `README.md`
- **Specific Widgets**:
  - `SCALE_WIDGET_README.md` - Scale widget
  - `SCALE_QUICK_REFERENCE.md` - Scale reference
  - `MENU_README.md` - Menu widget
  - `WIN_README.md` - Window widget
  - `TABLE_README.md` - Table widget
  - `CHART_README.md` - Chart widget
  - `LOTTIE_README.md` - Lottie widget
  - And more...

---

## 🔗 Useful Links

### Official Documentation
- [LVGL 9.5 Docs](https://docs.lvgl.io/9.5/)
- [Widget Catalog](https://docs.lvgl.io/9.5/details/widgets/index.html)
- [ESPHome LVGL](https://esphome.io/components/lvgl.html)

### Graphic Resources
- [Remix Icon](https://remixicon.com/) - 2800+ SVG icons
- [Weather Icons](https://github.com/basmilius/weather-icons) - Weather animations
- [LottieFiles](https://lottiefiles.com/) - Lottie animations
- [Ionicons](https://ionic.io/ionicons) - Premium icons

---

**LVGL 9.5 for ESPHome - Complete Implementation**

✅ 35/35 widgets
✅ 70/70 events
✅ 13/13 states
✅ 11/11 parts
✅ ThorVG + SVG + Lottie

Made with ❤️ for the ESPHome community

## License

This project is a fork of the ESPHome `lvgl` component and follows the same dual-license arrangement:

- **Python** code (`.py`) — MIT License
- **C++ / runtime** code (`.c`, `.cpp`, `.h`, `.hpp`, `.tcc`, `.ino`) — GPLv3 License

See the [`LICENSE`](../../LICENSE) file for the full MIT and GPLv3 texts, and [`NOTICE`](../../NOTICE) for third-party attributions (ESPHome, LVGL, Espressif, ThorVG).

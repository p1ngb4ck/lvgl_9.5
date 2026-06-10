# Complete Guide to LVGL 9.5 Widgets for ESPHome

This guide documents **all 35 widgets** available in the LVGL 9.5 implementation for ESPHome.

## Table of Contents

- [Basic Widgets](#basic-widgets)
- [Input Widgets](#input-widgets)
- [Display Widgets](#display-widgets)
- [Container Widgets](#container-widgets)
- [Advanced Widgets](#advanced-widgets)

---

## Basic Widgets

### 1. Label (Text)

Displays static or dynamic text.

```yaml
lvgl:
  pages:
    - id: home
      widgets:
        - label:
            id: my_label
            text: "Hello World!"
            x: 50
            y: 50
            text_font: montserrat_20
            text_color: 0xFFFFFF
            text_align: CENTER
            # Long text mode
            long_mode: WRAP  # WRAP, DOT, SCROLL, SCROLL_CIRCULAR, CLIP
            width: 200
```

**Main properties**:
- `text`: Text to display
- `text_font`: Font
- `text_color`: Text color
- `text_align`: Alignment (LEFT, CENTER, RIGHT)
- `long_mode`: Behavior for long text
- `recolor`: Enables inline color codes

**Documentation**: [Label - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/label.html)

---

### 2. Button (Button)

Clickable button with text or icon.

```yaml
lvgl:
  widgets:
    - button:
        id: my_button
        text: "Click Me"
        x: 100
        y: 100
        width: 120
        height: 50
        checkable: false  # Toggle button if true
        on_click:
          - logger.log: "Button clicked!"
        on_long_press:
          - logger.log: "Long press!"
```

**Available actions**:
- `on_click`: Single click
- `on_long_press`: Long press
- `on_press`: Press start
- `on_release`: Release

**Documentation**: [Button - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/button.html)

---

### 3. Image (Image)

Displays PNG, BMP, JPEG, GIF or SVG images.

```yaml
lvgl:
  widgets:
    - image:
        id: my_image
        src: "S:/icons/home.svg"  # SVG file on SD card
        # or
        src: my_image_id  # Image defined in esphome
        x: 50
        y: 50
        width: 64   # Resizes (SVG only)
        height: 64
        angle: 45   # Rotation in degrees (0-360)
        zoom: 256   # Zoom (256 = 100%, 512 = 200%)
        pivot_x: 50%  # Rotation pivot X
        pivot_y: 50%  # Rotation pivot Y
        antialias: true  # Anti-aliasing
```

**Supported formats**:
- **SVG**: Vector, scalable (ThorVG)
- **PNG**: Bitmap with transparency
- **BMP**: Simple bitmap
- **JPEG**: Compressed photos
- **GIF**: Animations

**Documentation**: [Image - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/image.html)

---

### 4. Object (Base Container)

Base container for grouping widgets.

```yaml
lvgl:
  widgets:
    - obj:
        id: my_container
        x: 0
        y: 0
        width: 200
        height: 150
        bg_color: 0x2196F3
        border_width: 2
        border_color: 0xFFFFFF
        radius: 10  # Rounded corners
        pad_all: 10  # Internal padding
        widgets:
          - label:
              text: "Inside container"
```

**Documentation**: [Object - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/obj.html)

---

## Input Widgets

### 5. Slider (Slider)

Slider for selecting a value.

```yaml
lvgl:
  widgets:
    - slider:
        id: brightness_slider
        x: 50
        y: 100
        width: 300
        min_value: 0
        max_value: 100
        value: 50
        mode: NORMAL  # NORMAL, SYMMETRICAL, RANGE
        on_change:
          - lambda: |-
              ESP_LOGI("slider", "Value: %d", (int)x);
```

**Modes**:
- `NORMAL`: Single value from min to max
- `SYMMETRICAL`: Centered value (0 in the middle)
- `RANGE`: Two values (start and end)

**Documentation**: [Slider - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/slider.html)

---

### 6. Switch (Switch)

ON/OFF switch.

```yaml
lvgl:
  widgets:
    - switch:
        id: wifi_switch
        x: 100
        y: 150
        state: true  # ON at startup
        on_change:
          - if:
              condition:
                lambda: return x;
              then:
                - logger.log: "Switch ON"
              else:
                - logger.log: "Switch OFF"
```

**Documentation**: [Switch - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/switch.html)

---

### 7. Checkbox (Checkbox)

Checkbox with label.

```yaml
lvgl:
  widgets:
    - checkbox:
        id: agree_checkbox
        text: "I agree to terms"
        x: 50
        y: 200
        checked: false
        on_change:
          - logger.log:
              format: "Checkbox: %s"
              args: [ 'x ? "checked" : "unchecked"' ]
```

**Documentation**: [Checkbox - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/checkbox.html)

---

### 8. Dropdown (Dropdown List)

Dropdown selection list.

```yaml
lvgl:
  widgets:
    - dropdown:
        id: city_selector
        x: 50
        y: 100
        width: 150
        options: "Paris\nLyon\nMarseille\nToulouse"
        # or with a list
        options:
          - "Paris"
          - "Lyon"
          - "Marseille"
        selected_index: 0
        on_select:
          - lambda: |-
              ESP_LOGI("dropdown", "Selected: %d", (int)x);
```

**Documentation**: [Dropdown - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/dropdown.html)

---

### 9. Roller (Selection Roller)

Vertical selection roller (iOS style).

```yaml
lvgl:
  widgets:
    - roller:
        id: time_roller
        x: 100
        y: 100
        width: 100
        height: 150
        options: "00\n01\n02\n03\n04\n05"
        selected_index: 0
        visible_row_count: 5  # Number of visible rows
        mode: NORMAL  # NORMAL or INFINITE (loop)
        on_select:
          - logger.log:
              format: "Hour: %d"
              args: [ 'x' ]
```

**Documentation**: [Roller - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/roller.html)

---

### 10. Textarea (Text Area)

Multiline text input area.

```yaml
lvgl:
  widgets:
    - textarea:
        id: message_input
        x: 50
        y: 100
        width: 300
        height: 150
        text: "Enter message..."
        placeholder_text: "Type here..."
        password_mode: false
        one_line: false  # true = single-line input
        max_length: 100
        accepted_chars: "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 "
        on_change:
          - logger.log:
              format: "Text: %s"
              args: [ 'x.c_str()' ]
```

**Documentation**: [Textarea - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/textarea.html)

---

### 11. Keyboard (Virtual Keyboard)

Virtual keyboard for text entry.

```yaml
lvgl:
  widgets:
    - textarea:
        id: input_field
        x: 50
        y: 50
        width: 300

    - keyboard:
        id: my_keyboard
        x: 0
        y: 250
        width: 100%
        height: 200
        mode: TEXT_LOWER  # TEXT_LOWER, TEXT_UPPER, SPECIAL, NUMBER
        textarea: input_field  # Links to the textarea
```

**Modes**:
- `TEXT_LOWER`: Lowercase letters
- `TEXT_UPPER`: Uppercase letters
- `SPECIAL`: Special characters
- `NUMBER`: Numeric keypad

**Documentation**: [Keyboard - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/keyboard.html)

---

### 12. Spinbox (Numeric Input)

Numeric input with +/- buttons.

```yaml
lvgl:
  widgets:
    - spinbox:
        id: temperature_spinbox
        x: 100
        y: 100
        width: 150
        height: 50
        value: 20
        min_value: 0
        max_value: 100
        step: 1
        digits: 3  # Number of digits
        decimal_places: 1  # Number of decimal places
        rollover: true  # Loops at the end
```

**Documentation**: [Spinbox - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/spinbox.html)

---

## Display Widgets

### 13. Arc (Circular Arc)

Arc/circle for displaying a value (gauge).

```yaml
lvgl:
  widgets:
    - arc:
        id: volume_arc
        x: 100
        y: 100
        width: 150
        height: 150
        start_angle: 135  # Start angle (0-360)
        end_angle: 45     # End angle
        value: 50
        min_value: 0
        max_value: 100
        mode: NORMAL  # NORMAL, REVERSE, SYMMETRICAL
        rotation: 0   # Global rotation
        adjustable: true  # Adjustable by the user
```

**Documentation**: [Arc - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/arc.html)

---

### 14. Bar (Progress Bar)

Horizontal or vertical progress bar.

```yaml
lvgl:
  widgets:
    - bar:
        id: battery_bar
        x: 50
        y: 100
        width: 200
        height: 20
        min_value: 0
        max_value: 100
        value: 75
        mode: NORMAL  # NORMAL, SYMMETRICAL, RANGE
        # Animation
        animated: true
        animation:
          duration: 500ms
```

**Documentation**: [Bar - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/bar.html)

---

### 15. LED (LED)

LED indicator with color and brightness.

```yaml
lvgl:
  widgets:
    - led:
        id: status_led
        x: 100
        y: 100
        width: 30
        height: 30
        color: 0x00FF00  # Green
        brightness: 255  # 0-255
```

**Documentation**: [LED - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/led.html)

---

### 16. Spinner (Loading Indicator)

Animated loading indicator.

```yaml
lvgl:
  widgets:
    - spinner:
        id: loading_spinner
        x: 150
        y: 150
        width: 50
        height: 50
        spin_time: 1000ms  # Duration of one rotation
        arc_length: 60  # Arc length (0-360)
```

**Documentation**: [Spinner - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/spinner.html)

---

### 17. Line (Line)

Line or polyline.

```yaml
lvgl:
  widgets:
    - line:
        id: my_line
        x: 50
        y: 50
        points:
          - x: 0
            y: 0
          - x: 100
            y: 50
          - x: 200
            y: 0
        line_width: 3
        line_color: 0xFF0000
        line_rounded: true  # Rounded ends
```

**Documentation**: [Line - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/line.html)

---

### 18. Scale (Scale/Gauge) ⚡ New in LVGL 9

Linear or circular graduated scale (replaces Meter).

```yaml
lvgl:
  widgets:
    - scale:
        id: speedometer
        x: 50
        y: 50
        width: 300
        height: 300
        mode: ROUND_OUTER  # HORIZONTAL_TOP, HORIZONTAL_BOTTOM,
                          # VERTICAL_LEFT, VERTICAL_RIGHT,
                          # ROUND_INNER, ROUND_OUTER
        range:
          min: 0
          max: 200
        angle_range: 270  # Total angle for circular mode
        rotation: 0
        # Ticks
        total_tick_count: 21
        major_tick_every: 5
        label_count: 11
        # Tick style
        tick_length: 10
        tick_width: 2
```

**Documentation**: [Scale - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/scale.html)
**See also**: `SCALE_WIDGET_README.md` and `SCALE_QUICK_REFERENCE.md`

---

### 19. Chart (Chart)

Chart for displaying data.

```yaml
lvgl:
  widgets:
    - chart:
        id: temperature_chart
        x: 50
        y: 50
        width: 300
        height: 200
        type: LINE  # LINE, BAR, SCATTER
        point_count: 20
        y_range:
          min: 0
          max: 40
        series:
          - id: temp_series
            color: 0xFF0000
            width: 2
```

**Chart types**:
- `LINE`: Line curve
- `BAR`: Histogram
- `SCATTER`: Scatter plot

**Documentation**: [Chart - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/chart.html)
**See also**: `CHART_README.md`

---

### 20. QR Code

Generates and displays a QR code.

```yaml
lvgl:
  widgets:
    - qrcode:
        id: wifi_qrcode
        x: 100
        y: 100
        size: 150
        data: "WIFI:T:WPA;S:MyNetwork;P:password123;;"
        dark_color: 0x000000
        light_color: 0xFFFFFF
```

**Documentation**: [QR Code - LVGL 9.5](https://docs.lvgl.io/9.5/details/libs/qrcode.html)

---

## Advanced Widgets

### 21. AnimImg (Animated Image)

Displays a sequence of animated images.

```yaml
lvgl:
  widgets:
    - animimg:
        id: my_animation
        x: 100
        y: 100
        images:
          - frame1
          - frame2
          - frame3
        duration: 100ms  # Duration per frame
        repeat_count: -1  # -1 = infinite
        auto_start: true
```

**Documentation**: [AnimImg - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/animimg.html)

---

### 22. Lottie (Vector Animation) ⚡ New in LVGL 9

JSON vector animations (ultra smooth).

```yaml
lvgl:
  widgets:
    - lottie:
        id: weather_animation
        src: "S:/animations/weather.json"
        x: 100
        y: 100
        width: 200
        height: 200
        loop: true
        autoplay: true
```

**Advantages**:
- Ultra-smooth 60 FPS animations
- 90% lighter than GIFs
- Resizable without loss of quality

**Resources**:
- [Weather Icons by Basmilius](https://github.com/basmilius/weather-icons)
- [LottieFiles](https://lottiefiles.com/)

**See also**: `LOTTIE_README.md`

---

### 23. 3D Texture ⚡ New in LVGL 9

Displays 3D models with ThorVG.

```yaml
lvgl:
  widgets:
    - tex3d:
        id: my_3d_model
        src: "S:/models/cube.svg"
        x: 100
        y: 100
        width: 200
        height: 200
        angle_x: 0
        angle_y: 0
        angle_z: 0
```

**See also**: `TEX3D_README.md`

---

### 24. Arc Label ⚡ New in LVGL 9

Text curved along an arc.

```yaml
lvgl:
  widgets:
    - arclabel:
        id: curved_text
        x: 100
        y: 100
        width: 200
        height: 200
        text: "Curved Text Example"
        angle: 270  # Arc angle
        radius: 100
        rotation: 0
```

**See also**: `ARCLABEL_README.md`

---

### 25. Span (Rich Text)

Text with mixed styles (bold, colors, different sizes).

```yaml
lvgl:
  widgets:
    - span:
        id: rich_text
        x: 50
        y: 50
        width: 300
        mode: BREAK  # EXPAND, BREAK, DOTS, CLIP
        spans:
          - text: "Bold text"
            text_font: montserrat_20
            text_color: 0xFF0000
          - text: " normal "
          - text: "italic"
            text_decor: UNDERLINE
```

**Documentation**: [Spangroup - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/spangroup.html)
**See also**: `SPAN_README.md`

---

## Container Widgets

### 26. TabView (Tabs)

Tabbed interface.

```yaml
lvgl:
  widgets:
    - tabview:
        id: my_tabs
        x: 0
        y: 0
        width: 100%
        height: 100%
        tab_position: TOP  # TOP, BOTTOM, LEFT, RIGHT
        tabs:
          - id: tab_home
            name: "Home"
            widgets:
              - label:
                  text: "Home content"

          - id: tab_settings
            name: "Settings"
            widgets:
              - label:
                  text: "Settings content"
```

**Documentation**: [TabView - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/tabview.html)

---

### 27. TileView (Scrolling Views)

Grid views with scrolling.

```yaml
lvgl:
  widgets:
    - tileview:
        id: my_tileview
        x: 0
        y: 0
        width: 100%
        height: 100%
        tiles:
          - id: tile1
            row: 0
            col: 0
            dir: HOR  # HOR, VER, ALL
            widgets:
              - label:
                  text: "Tile 1"

          - id: tile2
            row: 0
            col: 1
            dir: HOR
            widgets:
              - label:
                  text: "Tile 2"
```

**Documentation**: [TileView - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/tileview.html)

---

### 28. Menu (Hierarchical Menu) ⚡ New in LVGL 9

Hierarchical navigation menu with sidebar.

```yaml
lvgl:
  widgets:
    - menu:
        id: settings_menu
        x: 0
        y: 0
        width: 100%
        height: 100%
        root_back_button: false
        pages:
          - id: main_page
            title: "Main Menu"
            widgets:
              - button:
                  text: "Settings"
              - button:
                  text: "About"

          - id: settings_page
            title: "Settings"
            widgets:
              - switch:
                  text: "WiFi"
              - switch:
                  text: "Bluetooth"
```

**Documentation**: [Menu - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/menu.html)
**See also**: `MENU_README.md`

---

### 29. Window (Window)

Window with title bar and buttons.

```yaml
lvgl:
  widgets:
    - win:
        id: my_window
        x: 50
        y: 50
        width: 300
        height: 400
        title: "Window Title"
        header_height: 40
        close_button: true
        widgets:
          - label:
              text: "Window content"
```

**Documentation**: [Window - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/win.html)
**See also**: `WIN_README.md`

---

### 30. List (List)

List of buttons with text and icons.

```yaml
lvgl:
  widgets:
    - list:
        id: my_list
        x: 50
        y: 50
        width: 250
        height: 300
        items:
          - text: "Item 1"
            icon: "\xEF\x80\x81"  # Font Awesome icon
          - text: "Item 2"
            icon: "\xEF\x80\x82"
          - text: "Item 3"
```

**Documentation**: [List - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/list.html)

---

### 31. Table (Table)

Table with rows and columns.

```yaml
lvgl:
  widgets:
    - table:
        id: data_table
        x: 50
        y: 50
        width: 300
        height: 200
        col_count: 3
        row_count: 4
        cells:
          - row: 0
            col: 0
            text: "Name"
          - row: 0
            col: 1
            text: "Age"
          - row: 0
            col: 2
            text: "City"
          - row: 1
            col: 0
            text: "Alice"
          - row: 1
            col: 1
            text: "25"
          - row: 1
            col: 2
            text: "Paris"
```

**Documentation**: [Table - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/table.html)
**See also**: `TABLE_README.md` and `TABLE_IMPLEMENTATION_SUMMARY.md`

---

### 32. Calendar (Calendar) ⚡ New in LVGL 9

Interactive monthly calendar.

```yaml
lvgl:
  widgets:
    - calendar:
        id: my_calendar
        x: 50
        y: 50
        width: 300
        height: 300
        year: 2024
        month: 1  # 1-12
        day: 15
        show_month_names: true
        show_day_names: true
        on_change:
          - lambda: |-
              ESP_LOGI("calendar", "Selected: %d/%d/%d", day, month, year);
```

**Documentation**: [Calendar - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/calendar.html)
**See also**: `CALENDAR_README.md`

---

### 33. ButtonMatrix (Button Matrix)

Grid of configurable buttons.

```yaml
lvgl:
  widgets:
    - buttonmatrix:
        id: keypad
        x: 50
        y: 250
        width: 300
        height: 200
        rows: 4
        buttons:
          - row: 0
            buttons:
              - "1"
              - "2"
              - "3"
          - row: 1
            buttons:
              - "4"
              - "5"
              - "6"
          - row: 2
            buttons:
              - "7"
              - "8"
              - "9"
          - row: 3
            buttons:
              - ""
              - "0"
              - ""
        on_click:
          - lambda: |-
              ESP_LOGI("btnmatrix", "Button %d clicked", button_id);
```

**Documentation**: [ButtonMatrix - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/buttonmatrix.html)

---

### 34. MsgBox (Message Box)

Modal dialog box.

```yaml
lvgl:
  widgets:
    - msgbox:
        id: confirm_dialog
        title: "Confirmation"
        text: "Are you sure?"
        close_button: true
        buttons:
          - "Yes"
          - "No"
        on_click:
          - lambda: |-
              if (button_id == 0) {
                ESP_LOGI("msgbox", "Yes clicked");
              } else {
                ESP_LOGI("msgbox", "No clicked");
              }
```

**Documentation**: [MsgBox - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/msgbox.html)

---

### 35. Canvas (Drawing Canvas)

Drawing surface for custom graphics.

```yaml
lvgl:
  widgets:
    - canvas:
        id: drawing_canvas
        x: 50
        y: 50
        width: 300
        height: 200
        bg_color: 0xFFFFFF
```

**Drawing functions** (via lambda):
- Lines, rectangles, circles
- Text
- Images
- Individual pixels

**Documentation**: [Canvas - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/canvas.html)

---

### 36. ImageButton (Image Button)

Button with images for each state.

```yaml
lvgl:
  widgets:
    - imgbtn:
        id: power_button
        x: 100
        y: 100
        width: 64
        height: 64
        src: power_icon  # Normal image
        src_pressed: power_icon_pressed  # Pressed image
        src_checked: power_icon_on  # Checked image
        on_click:
          - logger.log: "Power button clicked"
```

**Documentation**: [ImageButton - LVGL 9.5](https://docs.lvgl.io/9.5/details/widgets/imagebutton.html)
**See also**: `IMGBTN_README.md`

---

## Properties Common to All Widgets

### Position and Size

```yaml
x: 100          # X position in pixels or %
y: 50           # Y position
width: 200      # Width
height: 100     # Height
```

### Alignment

```yaml
align: CENTER   # TOP_LEFT, TOP_MID, TOP_RIGHT, LEFT_MID,
               # CENTER, RIGHT_MID, BOTTOM_LEFT,
               # BOTTOM_MID, BOTTOM_RIGHT
align_to: other_widget_id
```

### Style

```yaml
bg_color: 0x2196F3      # Background color
bg_opa: COVER           # Opacity (0-255 or TRANSP/COVER)
border_width: 2         # Border thickness
border_color: 0xFFFFFF  # Border color
radius: 10              # Rounded corner radius
pad_all: 10             # Uniform padding
pad_left: 5             # Left padding
pad_right: 5            # Right padding
pad_top: 5              # Top padding
pad_bottom: 5           # Bottom padding
shadow_width: 10        # Shadow width
shadow_color: 0x000000  # Shadow color
```

### States and Parts

```yaml
styles:
  - state: DEFAULT      # default, checked, focused, pressed, etc.
    part: MAIN          # main, scrollbar, indicator, knob, etc.
    bg_color: 0x2196F3

  - state: PRESSED
    part: MAIN
    bg_color: 0x1976D2
```

### Flags

```yaml
flags:
  hidden: false         # Hidden
  clickable: true       # Clickable
  scrollable: true      # Scrollable
  checkable: false      # Can be checked
```

---

## Available Events

All widgets support these events:

### Input Events
- `on_press`: Press start
- `on_pressing`: Continuous pressing
- `on_click`: Single click
- `on_short_click`: Short click
- `on_long_press`: Long press
- `on_release`: Release
- `on_scroll`: Scroll
- `on_focus`: Focus gained
- `on_defocus`: Focus lost

### Special Events
- `on_change`: Value change
- `on_ready`: Widget ready
- `on_cancel`: Cancellation

### New LVGL 9.5 Events ⚡

```yaml
on_single_click: ...    # 1st click
on_double_click: ...    # 2nd click
on_triple_click: ...    # 3rd click
on_hover_over: ...      # Hover
on_hover_leave: ...     # Hover end
on_screen_loaded: ...   # Screen loaded
on_screen_unloaded: ... # Screen unloaded
```

---

## LVGL Actions

Actions available to control widgets:

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

# Animation
- lvgl.lottie.start: lottie_id
- lvgl.lottie.stop: lottie_id
- lvgl.lottie.pause: lottie_id

# State
- lvgl.widget.enable: widget_id
- lvgl.widget.disable: widget_id
- lvgl.widget.show: widget_id
- lvgl.widget.hide: widget_id
```

---

## Resources

### Official Documentation
- [LVGL 9.5 Documentation](https://docs.lvgl.io/9.5/)
- [Widget Catalog](https://docs.lvgl.io/9.5/details/widgets/index.html)
- [Events Reference](https://docs.lvgl.io/9.5/details/common-widget-features/events.html)
- [Styles Guide](https://docs.lvgl.io/9.5/details/common-widget-features/styles.html)

### Graphic Resources
- **SVG Icons**:
  - [Remix Icon](https://remixicon.com/) - 2,800+ icons
  - [Ionicons](https://ionic.io/ionicons)
  - [Heroicons](https://heroicons.com/)

- **Lottie Animations**:
  - [Weather Icons](https://github.com/basmilius/weather-icons)
  - [LottieFiles](https://lottiefiles.com/)
  - [Lordicon](https://lordicon.com/)

### Examples
- [LVGL Examples](https://docs.lvgl.io/9.5/examples.html)
- [ESPHome LVGL Examples](https://esphome.io/components/lvgl.html)

---

## Support and Contribution

### Issues and Questions
- [GitHub Issues](https://github.com/youkorr/test2_esp_video_esphome/issues)
- [ESPHome Discord](https://discord.gg/KhAMKrd)

### Widget Documentation
Check the specific READMEs for more details:
- `ARCLABEL_README.md`
- `CALENDAR_README.md`
- `CHART_README.md`
- `IMGBTN_README.md`
- `LOTTIE_README.md`
- `MENU_README.md`
- `SCALE_WIDGET_README.md`
- `SCALE_QUICK_REFERENCE.md`
- `SPAN_README.md`
- `TABLE_README.md`
- `TABLE_IMPLEMENTATION_SUMMARY.md`
- `TEX3D_README.md`
- `WIN_README.md`

---

**Complete LVGL 9.5 implementation for ESPHome**
✅ 35/35 widgets documented
✅ 70 events supported
✅ ThorVG/SVG/Lottie enabled

**Made with ❤️ for the ESPHome community**

## License

This project is a fork of the ESPHome `lvgl` component and follows the same dual-license arrangement:

- **Python** code (`.py`) — MIT License
- **C++ / runtime** code (`.c`, `.cpp`, `.h`, `.hpp`, `.tcc`, `.ino`) — GPLv3 License

See the [`LICENSE`](../../LICENSE) file for the full MIT and GPLv3 texts, and [`NOTICE`](../../NOTICE) for third-party attributions (ESPHome, LVGL, Espressif, ThorVG).

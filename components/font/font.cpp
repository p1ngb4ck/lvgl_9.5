#include "font.h"

#include "esphome/core/color.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace font {

static const char *const TAG = "font";

#ifdef USE_LVGL_FONT

static const uint8_t OPA4_TABLE[16] = {
    0, 17, 34, 51, 68, 85, 102, 119,
    136, 153, 170, 187, 204, 221, 238, 255};

static const uint8_t OPA2_TABLE[4] = {0, 85, 170, 255};

//==================== BASIC BITMAP ====================
const uint8_t *Font::get_glyph_bitmap(const lv_font_t *font, uint32_t unicode_letter) {
  auto *fe = (Font *) font->dsc;
  const auto *gd = fe->get_glyph_data_(unicode_letter);
  if (!gd) return nullptr;
  return gd->data;
}

//==================== LVGL DRAW ====================
const void *Font::get_glyph_bitmap(lv_font_glyph_dsc_t *dsc, lv_draw_buf_t *draw_buf) {
  const auto *font = dsc->resolved_font;
  auto *fe = (Font *) font->dsc;
  const auto *gd = fe->get_glyph_data_(dsc->gid.index);
  if (!gd) return nullptr;

  const uint8_t *bitmap_in = gd->data;
  uint8_t *bitmap_out = draw_buf->data;

  int32_t x, y, i = 0;
  uint32_t stride = lv_draw_buf_width_to_stride(gd->width, LV_COLOR_FORMAT_A8);

  switch (fe->get_bpp()) {

    case 1: {
      uint8_t mask = 0, byte = 0;
      for (y = 0; y < gd->height; y++) {
        for (x = 0; x < gd->width; x++) {
          if (mask == 0) {
            mask = 0x80;
            byte = *bitmap_in++;
          }
          bitmap_out[x] = (byte & mask) ? 255 : 0;
          mask >>= 1;
        }
        bitmap_out += stride;
      }
    } break;

    case 2:
      for (y = 0; y < gd->height; y++) {
        for (x = 0; x < gd->width; x++, i++) {
          switch (i & 3) {
            case 0: bitmap_out[x] = OPA2_TABLE[(*bitmap_in) >> 6]; break;
            case 1: bitmap_out[x] = OPA2_TABLE[((*bitmap_in) >> 4) & 3]; break;
            case 2: bitmap_out[x] = OPA2_TABLE[((*bitmap_in) >> 2) & 3]; break;
            case 3:
              bitmap_out[x] = OPA2_TABLE[(*bitmap_in) & 3];
              bitmap_in++;
              break;
          }
        }
        bitmap_out += stride;
      }
      break;

    case 4:
      for (y = 0; y < gd->height; y++) {
        for (x = 0; x < gd->width; x++, i++) {
          if ((i & 1) == 0) {
            bitmap_out[x] = OPA4_TABLE[(*bitmap_in) >> 4];
          } else {
            bitmap_out[x] = OPA4_TABLE[(*bitmap_in) & 0xF];
            bitmap_in++;
          }
        }
        bitmap_out += stride;
      }
      break;

    case 8:
      memcpy(bitmap_out, bitmap_in, gd->width * gd->height);
      break;

    default:
      ESP_LOGW(TAG, "Unsupported bpp: %d", fe->get_bpp());
      break;
  }

  return draw_buf;
}

//==================== GLYPH DESC ====================
bool Font::get_glyph_dsc_cb(const lv_font_t *font, lv_font_glyph_dsc_t *dsc,
                            uint32_t unicode_letter, uint32_t next) {
  auto *fe = (Font *) font->dsc;
  const auto *gd = fe->get_glyph_data_(unicode_letter);

  if (!gd) return false;

  dsc->box_w = gd->width;
  dsc->box_h = gd->height;
  dsc->ofs_x = gd->offset_x;
  dsc->ofs_y = gd->offset_y;

  // LVGL 8+
  dsc->format = LV_FONT_GLYPH_FORMAT_A8;

  dsc->gid.index = unicode_letter;

  return true;
}

//==================== CACHE ====================
const Glyph *Font::get_glyph_data_(uint32_t unicode_letter) {
  if (unicode_letter == this->last_letter_ && this->last_letter_ != 0)
    return this->last_data_;

  auto *glyph = this->find_glyph(unicode_letter);
  if (!glyph) return nullptr;

  this->last_letter_ = unicode_letter;
  this->last_data_ = glyph;
  return glyph;
}

#endif  // USE_LVGL_FONT

//==================== UTF-8 ====================
static uint32_t extract_unicode_codepoint(const char *utf8_str, size_t *length) {
  const uint8_t *current = reinterpret_cast<const uint8_t *>(utf8_str);
  uint8_t c1 = *current++;

  if (c1 == 0) {
    *length = 0;
    return 0;
  }

  uint32_t code_point = 0;

  if (c1 < 0x80) {
    code_point = c1;
  } else if ((c1 & 0xE0) == 0xC0) {
    uint8_t c2 = *current++;
    if ((c2 & 0xC0) != 0x80) return 0;
    code_point = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
  } else if ((c1 & 0xF0) == 0xE0) {
    uint8_t c2 = *current++;
    uint8_t c3 = *current++;
    if (((c2 & 0xC0) != 0x80) || ((c3 & 0xC0) != 0x80)) return 0;
    code_point = ((c1 & 0x0F) << 12) |
                 ((c2 & 0x3F) << 6) |
                 (c3 & 0x3F);
  } else if ((c1 & 0xF8) == 0xF0) {
    uint8_t c2 = *current++;
    uint8_t c3 = *current++;
    uint8_t c4 = *current++;
    if (((c2 & 0xC0) != 0x80) ||
        ((c3 & 0xC0) != 0x80) ||
        ((c4 & 0xC0) != 0x80)) return 0;

    code_point = ((c1 & 7) << 18) |
                 ((c2 & 0x3F) << 12) |
                 ((c3 & 0x3F) << 6) |
                 (c4 & 0x3F);
  } else {
    *length = 0;
    return 0;
  }

  *length = current - reinterpret_cast<const uint8_t *>(utf8_str);
  return code_point;
}

//==================== CONSTRUCTOR ====================
Font::Font(const Glyph *data, int data_nr, int baseline, int height,
           int descender, int xheight, int capheight, uint8_t bpp)
    : glyphs_(ConstVector(data, data_nr)),
      baseline_(baseline),
      height_(height),
      descender_(descender),
      linegap_(height - baseline - descender),
      xheight_(xheight),
      capheight_(capheight),
      bpp_(bpp) {

#ifdef USE_LVGL_FONT
  this->lv_font_.dsc = this;
  this->lv_font_.line_height = this->height_;
  this->lv_font_.base_line = this->height_ - this->baseline_;
  this->lv_font_.get_glyph_dsc = get_glyph_dsc_cb;
  this->lv_font_.get_glyph_bitmap = get_glyph_bitmap;
#endif
}

//==================== FIND ====================
const Glyph *Font::find_glyph(uint32_t codepoint) const {
  int lo = 0;
  int hi = this->glyphs_.size() - 1;

  while (lo != hi) {
    int mid = (lo + hi + 1) / 2;
    if (this->glyphs_[mid].is_less_or_equal(codepoint))
      lo = mid;
    else
      hi = mid - 1;
  }

  auto *result = &this->glyphs_[lo];
  if (result->code_point == codepoint)
    return result;

  return nullptr;
}

#ifdef USE_DISPLAY

//==================== MEASURE ====================
void Font::measure(const char *str, int *width, int *x_offset, int *baseline, int *height) {
  *baseline = this->baseline_;
  *height = this->height_;

  int min_x = 0;
  bool has_char = false;
  int x = 0;

  for (;;) {
    size_t length;
    auto code_point = extract_unicode_codepoint(str, &length);
    if (length == 0) break;

    str += length;
    auto *glyph = this->find_glyph(code_point);

    if (!glyph) {
      if (!this->glyphs_.empty())
        x += this->glyphs_[0].advance;
      continue;
    }

    if (!has_char)
      min_x = glyph->offset_x;
    else
      min_x = std::min(min_x, x + glyph->offset_x);

    x += glyph->advance;
    has_char = true;
  }

  *x_offset = min_x;
  *width = x - min_x;
}

//==================== PRINT ====================
void Font::print(int x_start, int y_start, display::Display *display,
                 Color color, const char *text, Color background) {

  int x_at = x_start;

  for (;;) {
    size_t length;
    auto code_point = extract_unicode_codepoint(text, &length);
    if (length == 0) break;

    text += length;
    auto *glyph = this->find_glyph(code_point);

    if (!glyph) {
      if (!this->glyphs_.empty())
        x_at += this->glyphs_[0].advance;
      continue;
    }

    const uint8_t *data = glyph->data;
    uint8_t bitmask = 0;
    uint8_t pixel_data = 0;

    for (int y = 0; y < glyph->height; y++) {
      for (int x = 0; x < glyph->width; x++) {

        uint8_t pixel = 0;

        for (uint8_t b = 0; b < this->bpp_; b++) {
          if (bitmask == 0) {
            pixel_data = progmem_read_byte(data++);
            bitmask = 0x80;
          }

          pixel <<= 1;
          if (pixel_data & bitmask)
            pixel |= 1;

          bitmask >>= 1;
        }

        if (pixel) {
          display->draw_pixel_at(
            x_at + glyph->offset_x + x,
            y_start + glyph->offset_y + y,
            color
          );
        }
      }
    }

    x_at += glyph->advance;
  }
}

#endif  // USE_DISPLAY

}  // namespace font
}  // namespace esphome

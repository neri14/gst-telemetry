#ifndef COMMON_H
#define COMMON_H

#include <string>

extern "C" {
    #include <pango/pango.h>
}

namespace telemetry {
namespace overlay {

enum class ETextAlign {
    Left,
    Center,
    Right
};

inline ETextAlign text_align_from_string(const std::string& align) {
    if (align == "left") {
        return ETextAlign::Left;
    } else if (align == "center") {
        return ETextAlign::Center;
    } else if (align == "right") {
        return ETextAlign::Right;
    } else {
        return ETextAlign::Left; // Default alignment
    }
}

inline PangoAlignment to_pango_align(ETextAlign align) {
    if (align == ETextAlign::Left) {
        return PANGO_ALIGN_LEFT;
    } else if (align == ETextAlign::Center) {
        return PANGO_ALIGN_CENTER;
    } else if (align == ETextAlign::Right) {
        return PANGO_ALIGN_RIGHT;
    } else {
        return PANGO_ALIGN_LEFT; // Default alignment
    }
}

} // namespace overlay
} // namespace telemetry

#endif // COMMON_H
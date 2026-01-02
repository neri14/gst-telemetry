#ifndef COMMON_H
#define COMMON_H

#include <string>

extern "C" {
    #include <pango/pango.h>
}

namespace telemetry {

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
    switch (align) {
        case ETextAlign::Right:
            return PANGO_ALIGN_RIGHT;
        case ETextAlign::Center:
            return PANGO_ALIGN_CENTER;
        case ETextAlign::Left:
        default:
            return PANGO_ALIGN_LEFT;
    }
}

} // namespace telemetry

#endif // COMMON_H
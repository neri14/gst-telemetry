
#ifndef ANCHOR_POINT_H
#define ANCHOR_POINT_H

#include <string>

namespace telemetry {

enum class EAnchorPoint {
    TopLeft,
    // TopRight, //unsupported
    // BottomLeft, //unsupported
    // BottomRight, //unsupported
    Center
};

namespace defaults {
    const EAnchorPoint anchor_point = EAnchorPoint::TopLeft;
}

inline EAnchorPoint anchor_point_from_string(const std::string& anchor) {
    if (anchor == "default") {
        return defaults::anchor_point;
    } else if (anchor == "topleft") {
        return EAnchorPoint::TopLeft;
    } else if (anchor == "center") {
        return EAnchorPoint::Center;
    } else {
        return defaults::anchor_point; 
    }
}

} // namespace telemetry

#endif // ANCHOR_POINT_H

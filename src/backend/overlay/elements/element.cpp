#include "element.h"

namespace telemetry {
namespace overlay {

Element::Element(std::shared_ptr<track::Track> track, int x, int y)
    : x(x), y(y), track_(track) {
}

void Element::draw(time::microseconds_t timestamp, cairo_t* cr) {
    for (auto& child : children) {
        child->draw(timestamp, cr);
    }
}

} // namespace overlay
} // namespace telemetry

#ifndef ELEMENT_H
#define ELEMENT_H

#include <cairo.h>
#include <vector>
#include "backend/utils/time.h"
#include "backend/track/track.h"

namespace telemetry {
namespace overlay {

struct Element {
    Element(std::shared_ptr<track::Track> track, int x = 0, int y = 0);
    virtual ~Element() = default;

    virtual void draw(time::microseconds_t timestamp, cairo_t* cr);

    std::vector<std::shared_ptr<Element>> children;
    int x;
    int y;

protected:
    std::shared_ptr<track::Track> track_;
};

} // namespace overlay
} // namespace telemetry

#endif // ELEMENT_H
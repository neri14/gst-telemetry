#ifndef LAYOUT_H
#define LAYOUT_H

#include <cairo.h>
#include <pugixml.hpp>
#include <string>

//TODO the widgets
#include "widgets/widget.h"

#include "backend/utils/logging/logger.h"
#include "backend/track/track.h"

namespace telemetry {
namespace overlay {

class Layout {
public:
    Layout(std::shared_ptr<track::Track> track);
    ~Layout() = default;

    void draw(time::microseconds_t timestamp, cairo_t* cr);

    bool load(const std::string& path);

private:
    mutable utils::logging::Logger log{"layout"};

    std::shared_ptr<Widget> parse_node(pugi::xml_node node);
    
    //TODO helpers to create widgets

    std::shared_ptr<track::Track> track_;
    std::shared_ptr<Widget> root_;
};

} // namespace overlay
} // namespace telemetry

#endif // LAYOUT_H
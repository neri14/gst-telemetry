#ifndef LAYOUT_H
#define LAYOUT_H

#include <cairo.h>
#include <pugixml.hpp>
#include <string>

#include "widgets/widget.h"

#include "backend/utils/logging/logger.h"
#include "backend/track/track.h"

namespace telemetry {
namespace overlay {

using surface_list_t = std::vector<std::tuple<int, int, cairo_surface_t*>>;

class Layout {
public:
    Layout(std::shared_ptr<track::Track> track);
    ~Layout();

    void draw(time::microseconds_t timestamp, surface_list_t& surface_list);

    bool load(const std::string& path);

private:
    mutable utils::logging::Logger log{"layout"};

    std::shared_ptr<Widget> parse_node(pugi::xml_node node);

    template<typename T>
    std::shared_ptr<T> parse_widget(pugi::xml_node node) {
        auto params = parse_parameters(node, T::parameter_types);
        return T::create(params);
    }

    parameter_map_ptr parse_parameters(
        pugi::xml_node node,
        const parameter_type_map_t& param_types);

    std::shared_ptr<track::Track> track_;
    std::shared_ptr<Widget> root_;

    unsigned int surface_count_ = 0;
    unsigned long total_drawing_time_ms_ = 0;
    unsigned int drawing_count_ = 0;
};

} // namespace overlay
} // namespace telemetry

#endif // LAYOUT_H
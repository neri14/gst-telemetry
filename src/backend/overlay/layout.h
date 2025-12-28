#ifndef LAYOUT_H
#define LAYOUT_H

#include <cairo.h>
#include <pugixml.hpp>
#include <string>

#include "elements/element.h"
#include "elements/conditional_element.h"
#include "elements/text_widget.h"
#include "elements/value_widget.h"
#include "elements/datetime_widget.h"
#include "elements/chart_widget.h"

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

    bool parse_node(std::shared_ptr<Element> element, pugi::xml_node node);
    std::shared_ptr<ConditionalElement> make_conditional_element(std::shared_ptr<Element> parent,
        std::shared_ptr<track::Track> track, pugi::xml_node node);
    std::shared_ptr<TextWidget> make_text_widget(std::shared_ptr<Element> parent,
        std::shared_ptr<track::Track> track, pugi::xml_node node);
    std::shared_ptr<ValueWidget> make_value_widget(std::shared_ptr<Element> parent,
        std::shared_ptr<track::Track> track, pugi::xml_node node);
    std::shared_ptr<DatetimeWidget> make_datetime_widget(std::shared_ptr<Element> parent,
        std::shared_ptr<track::Track> track, pugi::xml_node node);
    std::shared_ptr<ChartWidget> make_chart_widget(std::shared_ptr<Element> parent,
        std::shared_ptr<track::Track> track, pugi::xml_node node);

    std::shared_ptr<track::Track> track_;
    std::shared_ptr<Element> root_;
};

} // namespace overlay
} // namespace telemetry

#endif // LAYOUT_H
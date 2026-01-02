#include "layout.h"

#include <chrono>

#include "backend/utils/text_align.h"
#include "backend/utils/color.h"

#include "widgets/container_widget.h"
#include "widgets/conditional_widget.h"
#include "widgets/text_widget.h"
#include "widgets/circle_widget.h"

#include "widgets/params/color_parameter.h"
#include "widgets/params/alignment_parameter.h"
#include "widgets/params/numeric_parameter.h"
#include "widgets/params/string_parameter.h"
#include "widgets/params/boolean_parameter.h"

namespace telemetry {
namespace overlay {

namespace defaults {
    const char* font = "Arial 12";
    const char* color = "white";
    const char* border_color = "black";
    const int border_width = 0;
    const char* align = "left";
    const char* key = "";
    const char* format = "";
    const double scale = 1.0;
    const char* datetime_format = "%Y-%m-%d %H:%M:%S";
    const char* timezone = "UTC";
    const int chart_width = 200;
    const int chart_height = 100;
    const int chart_line_width = 2;
    const int chart_point_size = 4;
} // namespace defaults

Layout::Layout(std::shared_ptr<track::Track> track)
    : track_(track) {
}

void Layout::draw(time::microseconds_t timestamp, cairo_t* cr) {
    if (cr == nullptr) {
        log.error("draw: no cairo canvas provided");
        return;
    }
    if (root_ == nullptr) {
        log.warning("draw: no root widget defined");
        return;
    }
    //TODO future optimization idea (probably first tracing needs to be ported to verify gains):
    //  - traverse widget tree and collect "redrawing tasks" that would return e.g. function, x, y, z-index
    //  - sort tasks by z-index (from back to front, tbd for equal z-index)
    //  - draw in parallel onto internal caches (passing to worker threads in order)
    //  - on separate thread compose image in sorted order as soon as each surface is ready

    log.debug("Drawing layout");

    auto t1 = std::chrono::high_resolution_clock::now();
    root_->draw(timestamp, cr);
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = t2 - t1;
    log.debug("Layout drawing time: {} ms", elapsed.count());
}

bool Layout::load(const std::string& path) {
    log.info("Loading layout from path: {}", path);
    
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(path.c_str());

    if (!result) {
        log.error("Failed to load layout file: {}. Error description: {}", path, result.description());
        return false;
    }

    if (doc.children().empty()) {
        log.error("Empty layout file: {}", path);
        return false;
    }

    if (std::distance(doc.children().begin(), doc.children().end()) > 1) {
        log.error("More than one root node in layout file");
        return false;
    }

    pugi::xml_node node = doc.child("layout");
    if (!node) {
        log.error("No <layout> root node found in layout file");
        return false;
    }

    root_ = parse_node(node);

    return root_ != nullptr;
}

std::shared_ptr<Widget> Layout::parse_node(pugi::xml_node node) {
    log.debug("Parsing layout node: {}", node.name());

    std::string name = node.name();
    std::shared_ptr<Widget> widget = nullptr;

    if (name == "layout") {
        widget = parse_widget<Widget>(node);
        log.debug("Created base widget");
    } else if (name == "container") {
        widget = parse_widget<ContainerWidget>(node);
        log.debug("Created Container widget");
    } else if (name == "if") {
        widget = parse_widget<ConditionalWidget>(node);
        log.debug("Created Conditional widget");
    } else if (name == "text") {
        widget = parse_widget<TextWidget>(node);
        log.debug("Created Text widget");
    } else if (name == "circle") {
        widget = parse_widget<CircleWidget>(node);
        log.debug("Created Circle widget");
    } else {
        log.warning("Unknown widget defined: {}", name);
    }

    if (widget) {
        for (auto child : node.children()) {
            auto child_widget = parse_node(child);

            if (child_widget) {
                widget->add_child(child_widget);
            } else {
                log.warning("Failed to create child widget {} under widget {}", child.name(), name);
            }
        }
    } else {
        log.warning("Failed to create widget for node: {}", name);
    }

    return widget;
}

parameter_map_ptr Layout::parse_parameters(
        pugi::xml_node node,
        const parameter_type_map_t& param_types) {
    auto params = std::make_shared<parameter_map_t>();

    for (auto attr : node.attributes()) {
        std::string attr_name = attr.name();
        std::string attr_value = attr.as_string();

        auto it = param_types.find(attr_name);
        if (it != param_types.end()) {
            switch (it->second) {
                case ParameterType::Numeric:
                    (*params)[attr_name] = NumericParameter::create(attr_value, track_);
                    break;
                case ParameterType::Color:
                    (*params)[attr_name] = ColorParameter::create(attr_value, track_);
                    break;
                case ParameterType::Alignment:
                    (*params)[attr_name] = AlignmentParameter::create(attr_value, track_);
                    break;
                case ParameterType::String:
                    (*params)[attr_name] = StringParameter::create(attr_value, track_);
                    break;
                case ParameterType::Boolean:
                    (*params)[attr_name] = BooleanParameter::create(attr_value, track_);
                    break;
                // case ParameterType::FormattedValue:
                //     (*params)[attr_name] = std::make_shared<FormattedValueParameter>(attr_value, track_);
                //     break;
                default:
                    log.warning("Unknown parameter type for attribute: {}", attr_name);
                    break;
            }
        } else {
            log.warning("Unknown parameter name: {}", attr_name);
        }
    }

    return params;
}

} // namespace overlay
} // namespace telemetry
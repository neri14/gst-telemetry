#include "layout.h"
#include "backend/utils/text_align.h"
#include "backend/utils/color.h"
#include <chrono>

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
        log.warning("draw: no root element defined");
        return;
    }

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
    std::shared_ptr<Widget> element = nullptr;

    if (name == "layout") {
        element = std::make_shared<Widget>(track_);
        log.debug("Created base widget");
    // } else if (name == "container") {
    //     element = std::make_shared<Element>(track_,
    //         node.attribute("x").as_int(0) + (parent ? parent->x : 0),
    //         node.attribute("y").as_int(0) + (parent ? parent->y : 0));
    //     log.debug("Created container element");
    // } else if (name == "if") {
    //     element = make_conditional_element(parent, track_, node);
    //     log.debug("Created ConditionalElement element");
    // } else if (name == "widget") {
    //     std::string type = node.attribute("type").as_string();

    //     if (type == "text") {
    //         element = make_text_widget(parent, track_, node);
    //         log.debug("Created TextWidget element");
    //     } else {
    //         log.warning("Unknown widget type: {}", type);
    //     }
    } else {
        log.warning("Unknown layout element: {}", name);
    }

    if (element) {
        for (auto child : node.children()) {
            auto child_element = parse_node(child);

            if (child_element) {
                element->add_child(child_element);
            } else {
                log.warning("Failed to create child element {} under element {}", child.name(), name);
            }
        }
    } else {
        log.warning("Failed to create element for node: {}", name);
    }

    return element;
}

} // namespace overlay
} // namespace telemetry
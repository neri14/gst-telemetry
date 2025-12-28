#include "layout.h"
#include "elements/common/text_align.h"
#include "elements/common/color.h"
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
    log.debug("Layout draw time: {} ms", elapsed.count());
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

    bool ok = parse_node(nullptr, node);

    return ok;
}

bool Layout::parse_node(std::shared_ptr<Element> parent, pugi::xml_node node) {
    log.debug("Parsing layout node: {}", node.name());

    bool ok = true;

    std::string name = node.name();
    std::shared_ptr<Element> element = nullptr;

    if (name == "layout") {
        root_ = std::make_shared<Element>(track_);
        element = root_;
        log.debug("Created root layout element");
    } else if (name == "container") {
        element = std::make_shared<Element>(track_,
            node.attribute("x").as_int(0) + (parent ? parent->x : 0),
            node.attribute("y").as_int(0) + (parent ? parent->y : 0));
        log.debug("Created container element");
    } else if (name == "widget") {
        std::string type = node.attribute("type").as_string();

        if (type == "text") {
            element = make_text_widget(parent, track_, node);
            log.debug("Created TextWidget element");
        } else if (type == "value") {
            element = make_value_widget(parent, track_, node);
            log.debug("Created ValueWidget element");
        } else if (type == "datetime") {
            element = make_datetime_widget(parent, track_, node);
            log.debug("Created DatetimeWidget element");
        } else {
            log.warning("Unknown widget type: {}", type);
        }
    } else if (name == "if") {
        element = make_conditional_element(parent, track_, node);
        log.debug("Created ConditionalElement element");
    }else {
        log.warning("Unknown layout element: {}", name);
    }

    if (element) {
        if (parent) {
            parent->children.push_back(element);
        }

        for (auto child : node.children()) {
            ok = parse_node(element, child) && ok;
        }
    } else {
        log.warning("Failed to create element for node: {}", name);
        ok = false;
    }

    return ok;
}

std::shared_ptr<TextWidget> Layout::make_text_widget(
        std::shared_ptr<Element> parent,
        std::shared_ptr<track::Track> track,
        pugi::xml_node node) {
    int x = node.attribute("x").as_int(0) + parent->x;
    int y = node.attribute("y").as_int(0) + parent->y;
    ETextAlign align = text_align_from_string(node.attribute("align").as_string(defaults::align));
    std::string font = node.attribute("font").as_string("Arial 12");
    rgba color = color_from_string(node.attribute("color").as_string(defaults::color));
    rgba border_color = color_from_string(node.attribute("border_color").as_string(defaults::border_color));
    int border_width = node.attribute("border_width").as_int(defaults::border_width);
    std::string text = node.attribute("text").as_string("");

    return std::make_shared<TextWidget>(
        track, x, y, align, font, color, border_color, border_width, text);
}

std::shared_ptr<ValueWidget> Layout::make_value_widget(
        std::shared_ptr<Element> parent,
        std::shared_ptr<track::Track> track,
        pugi::xml_node node) {
    int x = node.attribute("x").as_int(0) + parent->x;
    int y = node.attribute("y").as_int(0) + parent->y;
    ETextAlign align = text_align_from_string(node.attribute("align").as_string(defaults::align));
    std::string font = node.attribute("font").as_string(defaults::font);
    rgba color = color_from_string(node.attribute("color").as_string(defaults::color));
    rgba border_color = color_from_string(node.attribute("border_color").as_string(defaults::border_color));
    int border_width = node.attribute("border_width").as_int(defaults::border_width);
    std::string key = node.attribute("key").as_string(defaults::key);
    std::string format = node.attribute("format").as_string(defaults::format);
    double scale = node.attribute("scale").as_double(defaults::scale);

    return std::make_shared<ValueWidget>(
        track, x, y, align, font, color, border_color, border_width, key, format, scale);
}

std::shared_ptr<DatetimeWidget> Layout::make_datetime_widget(
        std::shared_ptr<Element> parent,
        std::shared_ptr<track::Track> track,
        pugi::xml_node node) {
    int x = node.attribute("x").as_int(0) + parent->x;
    int y = node.attribute("y").as_int(0) + parent->y;
    ETextAlign align = text_align_from_string(node.attribute("align").as_string(defaults::align));
    std::string font = node.attribute("font").as_string(defaults::font);
    rgba color = color_from_string(node.attribute("color").as_string(defaults::color));
    rgba border_color = color_from_string(node.attribute("border_color").as_string(defaults::border_color));
    int border_width = node.attribute("border_width").as_int(defaults::border_width);
    std::string key = node.attribute("key").as_string(defaults::key);
    std::string format = node.attribute("format").as_string(defaults::datetime_format);
    std::string timezone = node.attribute("timezone").as_string(defaults::timezone);

    return std::make_shared<DatetimeWidget>(
        track, x, y, align, font, color, border_color, border_width, key, format, timezone);
}

std::shared_ptr<ConditionalElement> Layout::make_conditional_element(
        std::shared_ptr<Element> parent,
        std::shared_ptr<track::Track> track,
        pugi::xml_node node) {
    int x = node.attribute("x").as_int(0) + parent->x;
    int y = node.attribute("y").as_int(0) + parent->y;

    std::string key = node.attribute("key").as_string(defaults::key);
    std::optional<EOperator> oper = operator_from_string(node.attribute("operator").as_string(""));
    std::optional<double> value = std::nullopt;
    if (node.attribute("value")) {
        value = node.attribute("value").as_double();
    }

    return std::make_shared<ConditionalElement>(track, x, y, key, oper, value);
}

} // namespace overlay
} // namespace telemetry
#include <exprtk.hpp>
#include "widget.h"

namespace telemetry {
namespace overlay {

std::shared_ptr<Widget> Widget::create(parameter_map_ptr parameters) {
    utils::logging::Logger log{"Widget::create"};
    log.info("Creating base Widget");

    if (parameters->size() > 0) {
        log.warning("Base Widget does not support parameters, ignoring {} parameters",
                    parameters->size());
    }

    return std::make_shared<Widget>();
}

Widget::Widget() : log("Widget") {
}

Widget::Widget(const std::string& name) : log(name) {
}

void Widget::add_child(std::shared_ptr<Widget> child) {
    children_.push_back(child);
}

void Widget::draw(time::microseconds_t timestamp,
                  schedule_drawing_cb_t schedule_drawing_cb,
                  double x_offset, double y_offset) {
    for (auto& child : children_) {
        child->draw(timestamp, schedule_drawing_cb, x_offset, y_offset);
    }
}

} // namespace overlay
} // namespace telemetry

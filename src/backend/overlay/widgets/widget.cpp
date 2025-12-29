#include "widget.h"

namespace telemetry {
namespace overlay {

Widget::Widget(std::shared_ptr<track::Track> track)
        : log("widget"), track_(track) {
}

Widget::Widget(std::string name, std::shared_ptr<track::Track> track)
        : log(name), track_(track) {
}

void Widget::add_child(std::shared_ptr<Widget> child) {
    children_.push_back(child);
}

void Widget::draw(time::microseconds_t timestamp, cairo_t* cr, int x_offset, int y_offset) {
    for (auto& child : children_) {
        child->draw(timestamp, cr, x_offset, y_offset);
    }
}

} // namespace overlay
} // namespace telemetry

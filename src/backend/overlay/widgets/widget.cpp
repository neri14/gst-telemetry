#include <exprtk.hpp>
#include "widget.h"

namespace telemetry {
namespace overlay {

std::shared_ptr<Widget> Widget::create(parameter_map_ptr parameters) {
    utils::logging::Logger log{"Widget::create"};
    log.info("Creating base Widget");

    auto widget = std::make_shared<Widget>();

    if (parameters->size() > 0) {
        widget->log.warning("Base Widget does not support parameters, ignoring {} parameters",
                    parameters->size());
    }

    return widget;
}

Widget::Widget() : log("widget") {
}

Widget::Widget(std::string name) : log(name) {
}

void Widget::add_child(std::shared_ptr<Widget> child) {
    children_.push_back(child);
}

void Widget::draw(time::microseconds_t timestamp, cairo_t* cr, double x_offset, double y_offset) {
    //FIXME remove
    // /* TEST */
    // typedef exprtk::symbol_table<double> symbol_table_t;
    // typedef exprtk::expression<double>   expression_t;
    // typedef exprtk::parser<double>       parser_t;

    // // std::string expression_string = "3840 * (video_time % 15) / 15";
    // std::string expression_string = "video_time";
    
    // double video_time = time::us_to_s(timestamp);
    
    // symbol_table_t symbol_table;
    // symbol_table.add_variable("video_time", video_time);
    
    // expression_t expression;
    // expression.register_symbol_table(symbol_table);
    
    // parser_t parser;
    // parser.compile(expression_string, expression);
    
    // double result = expression.value();
    // log.error("Result: {}", result);
    // /* TEST */

    for (auto& child : children_) {
        child->draw(timestamp, cr, x_offset, y_offset);
    }
}

} // namespace overlay
} // namespace telemetry

#include "chart_widget.h"
#include "backend/utils/color.h"
#include "text_render.h"
#include "trace/trace.h"
#include <algorithm>
#include <cmath>
#include <deque>
#include <format>
#include <iostream>

extern "C" {
    #include <cairo.h>
}

//FIXME major refactoring required to cleanup the code

namespace telemetry {
namespace overlay {
namespace defaults {
    const rgb line_color = color::white;
    const int line_width = 2;

    const std::string marker_format = "{:.1f}";
    const std::string marker_font_name = "Arial";
    const int marker_font_size = 12;
    const int marker_label_offset = 4;
    const int marker_label_border_width = 0;
    const rgb marker_label_border_color = color::black;

    const rgb max_marker_color = color::transparent;
    const int max_marker_radius = 8;
    const int max_marker_border_width = 2;
    const rgb max_marker_border_color = color::red;
    const std::string max_marker_label_position = "top";

    const rgb min_marker_color = color::transparent;
    const int min_marker_radius = 8;
    const int min_marker_border_width = 2;
    const rgb min_marker_border_color = color::blue;
    const std::string min_marker_label_position = "bottom";
} // namespace defaults

namespace {
    ELabelPosition label_position_from_string(const std::string& s, ELabelPosition def) {
        if (s == "top") return ELabelPosition::Top;
        if (s == "bottom") return ELabelPosition::Bottom;
        if (s == "left") return ELabelPosition::Left;
        if (s == "right") return ELabelPosition::Right;
        if (s == "top-left") return ELabelPosition::TopLeft;
        if (s == "top-right") return ELabelPosition::TopRight;
        if (s == "bottom-left") return ELabelPosition::BottomLeft;
        if (s == "bottom-right") return ELabelPosition::BottomRight;

        utils::logging::Logger log{"ChartWidget"};
        log.warning("Unknown marker label position '{}', defaulting", s);
        return def;
    }
} // namespace


bool compare(std::shared_ptr<NumericParameter::sections_t> a, std::shared_ptr<NumericParameter::sections_t> b) {
    bool changed = false;

    if (a && !b) {
        changed = true;
    } else if (!a && b) {
        changed = true;
    } else if (a && b) {
        if (a->size() != b->size()) {
            changed = true;
        } else {
            for (size_t i = 0; i < a->size(); i++) {
                if (a->at(i).size() != b->at(i).size()) {
                    changed = true;
                    break;
                }
                auto it1 = a->at(i).begin();
                auto it2 = b->at(i).begin();
                while (it1 != a->at(i).end() && it2 != b->at(i).end()) {
                    if (it1->first != it2->first || it1->second != it2->second) {
                        changed = true;
                        break;
                    }
                    ++it1;
                    ++it2;
                }
                if (changed) {
                    break;
                }
            }
        }
    }

    return changed;
}


std::shared_ptr<ChartWidget> ChartWidget::create(parameter_map_ptr parameters) {
    utils::logging::Logger log{"ChartWidget::create"};
    log.info("Creating ChartWidget");

    auto widget = std::make_shared<ChartWidget>();

    for (const auto& [name, param] : *parameters) {
        if (name == "x") {
            widget->x_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "y") {
            widget->y_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "width") {
            widget->width_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "height") {
            widget->height_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "line-color") {
            widget->line_color_ = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "line-width") {
            widget->line_width_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "point-color") {
            widget->point_color_ = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "point-size") {
            widget->point_size_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "point-border-color") {
            widget->point_border_color_ = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "point-border-width") {
            widget->point_border_width_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "background-below") {
            widget->background_below_ = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "x-value") {
            widget->x_value_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "y-value") {
            widget->y_value_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "value-time-step") {
            widget->value_time_step_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "stretch-to-fill") {
            widget->stretch_to_fill_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else if (name == "min-x") {
            widget->min_x_param_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "max-x") {
            widget->max_x_param_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "min-y") {
            widget->min_y_param_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "max-y") {
            widget->max_y_param_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "visible") {
            widget->visible_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else if (name == "filter-value") {
            widget->filter_value_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "filter-max") {
            widget->filter_max_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "filter-min") {
            widget->filter_min_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "zoom-to-filter-x") {
            widget->zoom_to_filter_x_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else if (name == "zoom-to-filter-y") {
            widget->zoom_to_filter_y_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else if (name == "marker-window") {
            widget->marker_window_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "marker-format") {
            widget->marker_format_ = std::dynamic_pointer_cast<StringParameter>(param);
        } else if (name == "marker-font-name") {
            widget->marker_font_name_ = std::dynamic_pointer_cast<StringParameter>(param);
        } else if (name == "marker-font-size") {
            widget->marker_font_size_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "marker-label-offset") {
            widget->marker_label_offset_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "marker-label-border-width") {
            widget->marker_label_border_width_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "marker-label-border-color") {
            widget->marker_label_border_color_ = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "max-marker") {
            widget->max_marker_style_.enabled = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else if (name == "max-marker-color") {
            widget->max_marker_style_.color = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "max-marker-radius") {
            widget->max_marker_style_.radius = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "max-marker-border-width") {
            widget->max_marker_style_.border_width = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "max-marker-border-color") {
            widget->max_marker_style_.border_color = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "max-marker-label-color") {
            widget->max_marker_style_.label_color = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "max-marker-label-position") {
            widget->max_marker_style_.label_position = std::dynamic_pointer_cast<StringParameter>(param);
        } else if (name == "min-marker") {
            widget->min_marker_style_.enabled = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else if (name == "min-marker-color") {
            widget->min_marker_style_.color = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "min-marker-radius") {
            widget->min_marker_style_.radius = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "min-marker-border-width") {
            widget->min_marker_style_.border_width = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "min-marker-border-color") {
            widget->min_marker_style_.border_color = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "min-marker-label-color") {
            widget->min_marker_style_.label_color = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "min-marker-label-position") {
            widget->min_marker_style_.label_position = std::dynamic_pointer_cast<StringParameter>(param);
        } else {
            log.warning("Unknown parameter '{}' for ChartWidget", name);
        }
    }

    if (!widget->x_ || !widget->y_ || !widget->width_ || !widget->height_ || !widget->x_value_ || !widget->y_value_) {
        log.error("Missing required parameters (x, y, width, height, x-value, y-value)");
        return nullptr;
    }

    if ((widget->max_marker_style_.enabled || widget->min_marker_style_.enabled) && !widget->marker_window_) {
        log.error("marker-window is required when max-marker or min-marker is set");
        return nullptr;
    }

    if (!widget->line_color_) {
        log.debug("Line color parameter not set, using default value");
        widget->line_color_ = std::make_shared<ColorParameter>(defaults::line_color);
    }
    if (!widget->line_width_) {
        log.debug("Line width parameter not set, using default value");
        widget->line_width_ = std::make_shared<NumericParameter>(defaults::line_width);
    }
    if (!widget->stretch_to_fill_) {
        log.debug("Stretch-to-fit parameter not set, defaulting to true");
        widget->stretch_to_fill_ = std::make_shared<BooleanParameter>(true);
    }
    if (!widget->visible_) {
        log.debug("Visible parameter not set, defaulting to true");
        widget->visible_ = std::make_shared<BooleanParameter>(true);
    }

    if (!widget->zoom_to_filter_x_) {
        log.debug("Zoom-to-filter-x parameter not set, defaulting to false");
        widget->zoom_to_filter_x_ = std::make_shared<BooleanParameter>(false);
    }
    if (!widget->zoom_to_filter_y_) {
        log.debug("Zoom-to-filter-y parameter not set, defaulting to false");
        widget->zoom_to_filter_y_ = std::make_shared<BooleanParameter>(false);
    }

    if (!widget->marker_format_) {
        widget->marker_format_ = std::make_shared<StringParameter>(defaults::marker_format);
    }
    if (!widget->marker_font_name_) {
        widget->marker_font_name_ = std::make_shared<StringParameter>(defaults::marker_font_name);
    }
    if (!widget->marker_font_size_) {
        widget->marker_font_size_ = std::make_shared<NumericParameter>(defaults::marker_font_size);
    }
    if (!widget->marker_label_offset_) {
        widget->marker_label_offset_ = std::make_shared<NumericParameter>(defaults::marker_label_offset);
    }
    if (!widget->marker_label_border_width_) {
        widget->marker_label_border_width_ = std::make_shared<NumericParameter>(defaults::marker_label_border_width);
    }
    if (!widget->marker_label_border_color_) {
        widget->marker_label_border_color_ = std::make_shared<ColorParameter>(defaults::marker_label_border_color);
    }

    if (!widget->max_marker_style_.enabled) {
        widget->max_marker_style_.enabled = std::make_shared<BooleanParameter>(false);
    }
    if (!widget->max_marker_style_.color) {
        widget->max_marker_style_.color = std::make_shared<ColorParameter>(defaults::max_marker_color);
    }
    if (!widget->max_marker_style_.radius) {
        widget->max_marker_style_.radius = std::make_shared<NumericParameter>(defaults::max_marker_radius);
    }
    if (!widget->max_marker_style_.border_width) {
        widget->max_marker_style_.border_width = std::make_shared<NumericParameter>(defaults::max_marker_border_width);
    }
    if (!widget->max_marker_style_.border_color) {
        widget->max_marker_style_.border_color = std::make_shared<ColorParameter>(defaults::max_marker_border_color);
    }
    if (!widget->max_marker_style_.label_color) {
        // defaults to the (possibly dynamic) border color - reuse the same Parameter instance
        widget->max_marker_style_.label_color = widget->max_marker_style_.border_color;
    }
    if (!widget->max_marker_style_.label_position) {
        widget->max_marker_style_.label_position = std::make_shared<StringParameter>(defaults::max_marker_label_position);
    }

    if (!widget->min_marker_style_.enabled) {
        widget->min_marker_style_.enabled = std::make_shared<BooleanParameter>(false);
    }
    if (!widget->min_marker_style_.color) {
        widget->min_marker_style_.color = std::make_shared<ColorParameter>(defaults::min_marker_color);
    }
    if (!widget->min_marker_style_.radius) {
        widget->min_marker_style_.radius = std::make_shared<NumericParameter>(defaults::min_marker_radius);
    }
    if (!widget->min_marker_style_.border_width) {
        widget->min_marker_style_.border_width = std::make_shared<NumericParameter>(defaults::min_marker_border_width);
    }
    if (!widget->min_marker_style_.border_color) {
        widget->min_marker_style_.border_color = std::make_shared<ColorParameter>(defaults::min_marker_border_color);
    }
    if (!widget->min_marker_style_.label_color) {
        // defaults to the (possibly dynamic) border color - reuse the same Parameter instance
        widget->min_marker_style_.label_color = widget->min_marker_style_.border_color;
    }
    if (!widget->min_marker_style_.label_position) {
        widget->min_marker_style_.label_position = std::make_shared<StringParameter>(defaults::min_marker_label_position);
    }

    return widget;
}

ChartWidget::ChartWidget()
        : Widget("ChartWidget") {
}

void ChartWidget::draw(time::microseconds_t timestamp,
                        schedule_drawing_cb_t schedule_drawing_cb,
                        double x_offset, double y_offset) {
    // calculate visibility
    visible_->update(timestamp);

    if (visible_->get_value(timestamp)) {
        x_->update(timestamp);
        y_->update(timestamp);

        double x = x_offset + x_->get_value(timestamp);
        double y = y_offset + y_->get_value(timestamp);

        schedule_drawing_cb([this, timestamp, x, y](Surface& surface) {
            this->draw_impl(surface, timestamp, x, y);
        });

        // draw childern relative to chart top-left
        Widget::draw(timestamp, schedule_drawing_cb, x, y);
    } else {
        log.debug("Visibility is false, skipping drawing");
    }
    
}

void ChartWidget::draw_impl(Surface& surface, time::microseconds_t timestamp, double x, double y) {
    TRACE_EVENT_BEGIN(EV_CHART_WIDGET_DRAW);

    // if cache is not drawn, mark for redraw
    bool invalidate_line_cache = !line_cache_drawn_;
    bool invalidate_point_cache = !point_cache_drawn_;
    bool invalidate_extremes_cache = !extremes_cache_drawn_;

    // update parameters that can affect cache validity
    if (width_ && width_->update(timestamp)) {
        invalidate_line_cache = true;
        invalidate_point_cache = true;
        invalidate_extremes_cache = true;
    }
    if (height_ && height_->update(timestamp)) {
        invalidate_line_cache = true;
        invalidate_point_cache = true;
        invalidate_extremes_cache = true;
    }
    if (value_time_step_ && value_time_step_->update(timestamp)) {
        invalidate_line_cache = true;
        invalidate_point_cache = true;
        invalidate_extremes_cache = true;
    }
    if (stretch_to_fill_ && stretch_to_fill_->update(timestamp)) {
        invalidate_line_cache = true;
        invalidate_point_cache = true;
        invalidate_extremes_cache = true;
    }
    if (min_x_param_ && min_x_param_->update(timestamp)) {
        invalidate_line_cache = true;
        invalidate_point_cache = true;
        invalidate_extremes_cache = true;
    }
    if (max_x_param_ && max_x_param_->update(timestamp)) {
        invalidate_line_cache = true;
        invalidate_point_cache = true;
        invalidate_extremes_cache = true;
    }
    if (min_y_param_ && min_y_param_->update(timestamp)) {
        invalidate_line_cache = true;
        invalidate_point_cache = true;
        invalidate_extremes_cache = true;
    }
    if (max_y_param_ && max_y_param_->update(timestamp)) {
        invalidate_line_cache = true;
        invalidate_point_cache = true;
        invalidate_extremes_cache = true;
    }
    if (line_width_ && line_width_->update(timestamp)) {
        invalidate_line_cache = true;
    }
    if (point_color_ && point_color_->update(timestamp)) {
        invalidate_point_cache = true;
    }
    if (point_size_ && point_size_->update(timestamp)) {
        invalidate_point_cache = true;
    }
    if (point_border_color_ && point_border_color_->update(timestamp)) {
        invalidate_point_cache = true;
    }
    if (point_border_width_ && point_border_width_->update(timestamp)) {
        invalidate_point_cache = true;
    }
    // update filtering parameters
    if (filter_value_ && filter_value_->update(timestamp)) {
        // invalidate_line_cache = true;
        // invalidate_point_cache = true;
    }
    if (filter_max_ && filter_max_->update(timestamp)) {
        // invalidate_line_cache = true;
        // invalidate_point_cache = true;
    }
    if (filter_min_ && filter_min_->update(timestamp)) {
        // invalidate_line_cache = true;
        // invalidate_point_cache = true;
    }
    if (zoom_to_filter_x_ && zoom_to_filter_x_->update(timestamp)) {
        invalidate_line_cache = true;
        invalidate_point_cache = true;
        invalidate_extremes_cache = true;
    }
    if (zoom_to_filter_y_ && zoom_to_filter_y_->update(timestamp)) {
        invalidate_line_cache = true;
        invalidate_point_cache = true;
        invalidate_extremes_cache = true;
    }

    // update marker parameters - any change here requires both a fresh
    // x_values/y_values fetch (to re-run local-extrema detection) and a
    // repaint of the extremes cache, so these also force invalidate_line_cache
    for (auto& param : std::vector<parameter_ptr_t>{
            marker_window_, marker_format_, marker_font_name_, marker_font_size_,
            marker_label_offset_, marker_label_border_width_, marker_label_border_color_,
            max_marker_style_.enabled, max_marker_style_.color, max_marker_style_.radius,
            max_marker_style_.border_width, max_marker_style_.border_color,
            max_marker_style_.label_color, max_marker_style_.label_position,
            min_marker_style_.enabled, min_marker_style_.color, min_marker_style_.radius,
            min_marker_style_.border_width, min_marker_style_.border_color,
            min_marker_style_.label_color, min_marker_style_.label_position}) {
        if (param && param->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_extremes_cache = true;
        }
    }

    // read values needed for cache size calculation
    double width = width_->get_value(timestamp);
    double height = height_->get_value(timestamp);
    double line_width = line_width_->get_value(timestamp);
    double point_size = point_size_ ? point_size_->get_value(timestamp) : 0.0;
    stretch_chart_ = stretch_to_fill_->get_value(timestamp);

    // extra margin needed to fit local-extreme marker rings + their labels
    // without clipping. Exact label size depends on the formatted value/font
    // metrics (unknown without a full Pango pass), so this is a conservative,
    // per-frame-cheap estimate - not an exact fit - mirroring the same
    // "naive overestimation" approach StringWidget uses for its own cache sizing.
    double marker_extra = 0.0;
    bool markers_enabled = max_marker_style_.enabled->get_value(timestamp) ||
                            min_marker_style_.enabled->get_value(timestamp);
    if (markers_enabled) {
        double marker_radius = std::max(max_marker_style_.radius->get_value(timestamp),
                                        min_marker_style_.radius->get_value(timestamp));
        double marker_border = std::max(max_marker_style_.border_width->get_value(timestamp),
                                        min_marker_style_.border_width->get_value(timestamp));
        double label_offset = marker_label_offset_->get_value(timestamp);
        double label_border = marker_label_border_width_->get_value(timestamp);
        double font_size = marker_font_size_->get_value(timestamp);

        double label_w_estimate = font_size * 0.6 * 10.0; // ~10 average-width characters
        double label_h_estimate = font_size * 1.6;

        marker_extra = marker_radius + marker_border + label_offset + label_border
                       + std::max(label_w_estimate, label_h_estimate);
    }

    // calculate margin and cache size
    margin_ = 2*static_cast<int>(std::ceil(std::max({line_width, point_size, marker_extra})));
    int min_cache_width = static_cast<int>(std::ceil(width)) + 2*margin_;
    int min_cache_height = static_cast<int>(std::ceil(height)) + 2*margin_;

    if (min_cache_width > cache_width_ || min_cache_height > cache_height_) {
        // cache size change requires full redraw
        cache_width_ = min_cache_width;
        cache_height_ = min_cache_height;
        invalidate_line_cache = true;
        invalidate_point_cache = true;
        invalidate_extremes_cache = true;
    }

    time::microseconds_t value_step = time::INVALID_TIME;
    if (value_time_step_) {
        double step_val = value_time_step_->get_value(timestamp);
        if (step_val > 0.0) {
            value_step = time::s_to_us(step_val);;
        }
    }

    // read filter values
    bool filter_active = !!filter_value_;
    double filter_min = filter_min_ ? filter_min_->get_value(timestamp) : std::numeric_limits<double>::min();
    double filter_max = filter_max_ ? filter_max_->get_value(timestamp) : std::numeric_limits<double>::max();
    auto filter_values = filter_value_ ? filter_value_->get_values(value_step, filter_min, filter_max) : nullptr;

    if (filter_active && (value_step != time::INVALID_TIME || compare(last_filter_values_, filter_values))) {
        //if value_step is not invalid - there is almost 100% chance that filter values have changed
        //  as they are calculated in intervals from timestamp that is in video time domain (typical use case is for often changing graphs)
        invalidate_line_cache = true;
        invalidate_point_cache = true;
        invalidate_extremes_cache = true;
        last_filter_values_ = filter_values;
    }

    // redraw line cache if needed
    if (invalidate_line_cache) {
        TRACE_EVENT_BEGIN(EV_CHART_WIDGET_UPDATE_LINE_CACHE);

        bool filter_zoom_x = filter_active && zoom_to_filter_x_ && zoom_to_filter_x_->get_value(timestamp);
        bool filter_zoom_y = filter_active && zoom_to_filter_y_ && zoom_to_filter_y_->get_value(timestamp);

        std::shared_ptr<NumericParameter::sections_t> x_values = nullptr;
        std::shared_ptr<NumericParameter::sections_t> y_values = nullptr;

        if (filter_active && filter_values){
            std::vector<std::vector<time::microseconds_t>> timestamps = {};
            for (const auto& section : *filter_values) {
                std::vector<time::microseconds_t> ts_section = {};
                for (const auto& [ts, _] : section) {
                    ts_section.push_back(ts);
                }
                timestamps.push_back(ts_section);
            }
            x_values = x_value_->get_values(timestamps);
            y_values = y_value_->get_values(timestamps);
        } else {
            x_values = x_value_->get_values(value_step);
            y_values = y_value_->get_values(value_step);
        }

        lock_x_minmax_ = false;
        if (min_x_param_ && max_x_param_) {
            min_x_ = min_x_param_->get_value(timestamp);
            max_x_ = max_x_param_->get_value(timestamp);

            if (min_x_ < max_x_) {
                lock_x_minmax_ = true;
            }
        }
        lock_y_minmax_ = false;
        if (min_y_param_ && max_y_param_) {
            min_y_ = min_y_param_->get_value(timestamp);
            max_y_ = max_y_param_->get_value(timestamp);
            if (min_y_ < max_y_) {
                lock_y_minmax_ = true;
            }
        }

        recalculate_extremes(filter_zoom_x ? x_values : x_value_->get_values(),
                                filter_zoom_y ? y_values : y_value_->get_values());

        if (invalid_) {
            log.error("ChartWidget is in invalid state, aborting drawing");
            TRACE_EVENT_END(EV_CHART_WIDGET_UPDATE_LINE_CACHE);
            TRACE_EVENT_END(EV_CHART_WIDGET_DRAW);
            return;
        }

        redraw_line_cache(width, height, line_width, x_values, y_values);
        line_cache_drawn_ = true;

        TRACE_EVENT_END(EV_CHART_WIDGET_UPDATE_LINE_CACHE);

        if (invalidate_extremes_cache) {
            TRACE_EVENT_BEGIN(EV_CHART_WIDGET_UPDATE_EXTREMES_CACHE);
            redraw_extremes_cache(width, height, timestamp, x_values, y_values);
            extremes_cache_drawn_ = true;
            TRACE_EVENT_END(EV_CHART_WIDGET_UPDATE_EXTREMES_CACHE);
        }
    }

    // recalutate x and y values after possible track cache update
    // (since extremes recalculation invalidates cached values)
    if (x_value_ && x_value_->update(timestamp)) {
        invalidate_point_cache = true;
    }
    if (y_value_ && y_value_->update(timestamp)) {
        invalidate_point_cache = true;
    }

    double x_value = x_value_->get_value(timestamp, true);
    double y_value = y_value_->get_value(timestamp, true);

    // redraw point cache if needed
    if (invalidate_point_cache) {
        TRACE_EVENT_BEGIN(EV_CHART_WIDGET_UPDATE_POINT_CACHE);
        redraw_point_cache(width, height,
            point_color_ ? point_color_->get_value(timestamp) : color::transparent, point_size,
            point_border_color_ ? point_border_color_->get_value(timestamp) : color::transparent,
            point_border_width_ ? point_border_width_->get_value(timestamp) : 0,
            x_value, y_value);
        point_cache_drawn_ = true;
        TRACE_EVENT_END(EV_CHART_WIDGET_UPDATE_POINT_CACHE);
    }

    if (invalidate_line_cache || invalidate_point_cache || invalidate_extremes_cache) {
        TRACE_EVENT_BEGIN(EV_CHART_WIDGET_DRAW_COMBINED_CACHE);
        int surface_width = 0;
        int surface_height = 0;

        if (combined_cache_) {
            surface_width = cairo_image_surface_get_width(combined_cache_);
            surface_height = cairo_image_surface_get_height(combined_cache_);
        }

        if (!combined_cache_ || surface_width < cache_width_ || surface_height < cache_height_) {
            // bigger widget size require allocating bigger cache
            if (combined_cache_) {
                cairo_surface_destroy(combined_cache_);
                combined_cache_ = nullptr;
            }
            combined_cache_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, cache_width_, cache_height_);
            combined_cache_drawn_ = false;
            log.info("Allocated new combined cache surface: {}x{}", cache_width_, cache_height_);
        }

        cairo_t* cache_cr = cairo_create(combined_cache_);
        if (combined_cache_drawn_) {
            // clear cache
            cairo_save(cache_cr);
            cairo_set_operator(cache_cr, CAIRO_OPERATOR_CLEAR);
            cairo_paint(cache_cr);
            cairo_restore(cache_cr);
            combined_cache_drawn_ = false;
        }

        // draw caches onto combined cache
        if (line_cache_) {
            TRACE_EVENT_BEGIN(EV_CHART_WIDGET_DRAW_LINE_CACHE);
            cairo_set_source_surface(cache_cr, line_cache_, 0, 0);
            cairo_paint(cache_cr);
            TRACE_EVENT_END(EV_CHART_WIDGET_DRAW_LINE_CACHE);
        }
        if (point_cache_) {
            TRACE_EVENT_BEGIN(EV_CHART_WIDGET_DRAW_POINT_CACHE);
            cairo_set_source_surface(cache_cr, point_cache_, 0, 0);
            cairo_paint(cache_cr);
            TRACE_EVENT_END(EV_CHART_WIDGET_DRAW_POINT_CACHE);
        }
        if (extremes_cache_) {
            TRACE_EVENT_BEGIN(EV_CHART_WIDGET_DRAW_EXTREMES_CACHE);
            cairo_set_source_surface(cache_cr, extremes_cache_, 0, 0);
            cairo_paint(cache_cr);
            TRACE_EVENT_END(EV_CHART_WIDGET_DRAW_EXTREMES_CACHE);
        }

        cairo_destroy(cache_cr);
        combined_cache_drawn_ = true;
        TRACE_EVENT_END(EV_CHART_WIDGET_DRAW_COMBINED_CACHE);
    }

    surface.x =  x - margin_;
    surface.y =  y - margin_;
    surface.surface = combined_cache_;

    TRACE_EVENT_END(EV_CHART_WIDGET_DRAW);
}


void ChartWidget::redraw_line_cache(double width, double height, double line_width,
                                    std::shared_ptr<NumericParameter::sections_t> x_values,
                                    std::shared_ptr<NumericParameter::sections_t> y_values) {
    int surface_width = 0;
    int surface_height = 0;

    if (line_cache_) {
        surface_width = cairo_image_surface_get_width(line_cache_);
        surface_height = cairo_image_surface_get_height(line_cache_);
    }

    if (!line_cache_ || surface_width < cache_width_ || surface_height < cache_height_) {
        // bigger widget size require allocating bigger cache
        if (line_cache_) {
            cairo_surface_destroy(line_cache_);
            line_cache_ = nullptr;
        }
        line_cache_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, cache_width_, cache_height_);
        line_cache_drawn_ = false;
        log.info("Allocated new line cache surface: {}x{}", cache_width_, cache_height_);
    }

    cairo_t* cache_cr = cairo_create(line_cache_);
    if (line_cache_drawn_) {
        // clear cache
        cairo_save(cache_cr);
        cairo_set_operator(cache_cr, CAIRO_OPERATOR_CLEAR);
        cairo_paint(cache_cr);
        cairo_restore(cache_cr);
        line_cache_drawn_ = false;
    }

    // draw background below line
    if (background_below_) {
        draw_background(cache_cr, width, height, line_width, x_values, y_values);
    }

    if (line_width > 0) {
        draw_line(cache_cr, width, height, line_width, x_values, y_values);
    }

    cairo_destroy(cache_cr);
    line_cache_drawn_ = true;
}

void ChartWidget::draw_background(cairo_t* cache_cr, double width, double height, double line_width,
                                 std::shared_ptr<NumericParameter::sections_t> x_values,
                                 std::shared_ptr<NumericParameter::sections_t> y_values) {
    bool static_color = background_below_->is_static();
    if (static_color) {
        background_below_->update(time::INVALID_TIME);
        rgb static_color = background_below_->get_value(time::INVALID_TIME);
        cairo_set_source_rgba(cache_cr, static_color.r, static_color.g, static_color.b, static_color.a);
    }

    double y_base = height + margin_ + line_width;

    double last_x_pos = std::numeric_limits<double>::quiet_NaN();
    double first_x_pos = std::numeric_limits<double>::quiet_NaN();

    auto find_y_value = [&](time::microseconds_t ts) -> double {
        for (const auto& section : *y_values) {
            auto it = section.find(ts);
            if (it != section.end()) {
                return it->second;
            }
        }
        return std::numeric_limits<double>::quiet_NaN();
    };

    auto fill = [&](double x_pos) {
        cairo_line_to(cache_cr, x_pos, y_base);
        cairo_line_to(cache_cr, first_x_pos, y_base);
        cairo_close_path(cache_cr);
        cairo_fill(cache_cr);
    };

    for (const auto& section : *x_values) {
        bool last_point_valid = false;
        for (const auto& [ts, x_val] : section) {
            double y_val = find_y_value(ts);
            if (std::isnan(x_val) || std::isnan(y_val)) {
                fill(last_x_pos);
                last_point_valid = false;
                continue; // skip NaN values
            }

            auto [x_pos, y_pos] = translate(x_val, y_val, width, height);
            x_pos += margin_;
            y_pos += margin_;

            if (std::isnan(first_x_pos)) {
                first_x_pos = x_pos;
            }

            if (last_point_valid && cairo_has_current_point(cache_cr)) {
                cairo_line_to(cache_cr, x_pos, y_pos);

                if (!static_color) {
                    background_below_->update(ts);
                    rgb dynamic_color = background_below_->get_value(ts);
                    cairo_set_source_rgba(cache_cr, dynamic_color.r, dynamic_color.g, dynamic_color.b, dynamic_color.a);
                    fill(x_pos);
                    cairo_move_to(cache_cr, x_pos, y_pos);
                    first_x_pos = x_pos;
                }
            } else {
                cairo_move_to(cache_cr, x_pos, y_pos);
            }

            last_x_pos = x_pos;
            last_point_valid = true;
        }
    }
    
    fill(last_x_pos);
}

void ChartWidget::draw_line(cairo_t* cache_cr, double width, double height, double line_width,
                            std::shared_ptr<NumericParameter::sections_t> x_values,
                            std::shared_ptr<NumericParameter::sections_t> y_values) {
    cairo_set_line_cap(cache_cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_line_join(cache_cr, CAIRO_LINE_JOIN_BEVEL);

    cairo_set_line_width(cache_cr, line_width);

    bool static_color = line_color_->is_static();
    if (static_color) {
        line_color_->update(time::INVALID_TIME);
        rgb static_color = line_color_->get_value(time::INVALID_TIME);
        cairo_set_source_rgba(cache_cr, static_color.r, static_color.g, static_color.b, static_color.a);
    }

    auto find_y_value = [&](time::microseconds_t ts) -> double {
        for (const auto& section : *y_values) {
            auto it = section.find(ts);
            if (it != section.end()) {
                return it->second;
            }
        }
        return std::numeric_limits<double>::quiet_NaN();
    };

    for (const auto& section : *x_values) {
        bool last_point_valid = false;
        for (const auto& [ts, x_val] : section) {
            double y_val = find_y_value(ts);

            if (std::isnan(x_val) || std::isnan(y_val)) {
                cairo_stroke(cache_cr);
                last_point_valid = false;
                continue; // skip NaN values
            }

            auto [x_pos, y_pos] = translate(x_val, y_val, width, height);
            x_pos += margin_;
            y_pos += margin_;

            if (last_point_valid && cairo_has_current_point(cache_cr)) {
                cairo_line_to(cache_cr, x_pos, y_pos);
                if (!static_color) {
                    line_color_->update(ts);
                    rgb dynamic_color = line_color_->get_value(ts);
                    cairo_set_source_rgba(cache_cr, dynamic_color.r, dynamic_color.g, dynamic_color.b, dynamic_color.a);
                    cairo_stroke(cache_cr);
                }
            } else {
                cairo_move_to(cache_cr, x_pos, y_pos);
            }
            last_point_valid = true;
        }
    }
    cairo_stroke(cache_cr);
}

void ChartWidget::redraw_point_cache(double width, double height,
                                     rgb point_color, double point_size,
                                     rgb point_border_color, double point_border_width,
                                     double x_value, double y_value) {
    int surface_width = 0;
    int surface_height = 0;

    if (point_cache_) {
        surface_width = cairo_image_surface_get_width(point_cache_);
        surface_height = cairo_image_surface_get_height(point_cache_);
    }

    if (!point_cache_ || surface_width < cache_width_ || surface_height < cache_height_) {
        // bigger widget size require allocating bigger cache
        if (point_cache_) {
            cairo_surface_destroy(point_cache_);
            point_cache_ = nullptr;
        }
        point_cache_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, cache_width_, cache_height_);
        point_cache_drawn_ = false;
        log.info("Allocated new point cache surface: {}x{}", cache_width_, cache_height_);
    }

    cairo_t* cache_cr = cairo_create(point_cache_);
    if (point_cache_drawn_) {
        // clear cache
        cairo_save(cache_cr);
        cairo_set_operator(cache_cr, CAIRO_OPERATOR_CLEAR);
        cairo_paint(cache_cr);
        cairo_restore(cache_cr);
        point_cache_drawn_ = false;
    }

    //draw point
    if (point_size > 0 && !std::isnan(x_value) && !std::isnan(y_value)) {
        cairo_set_line_width(cache_cr, 1.0);
        auto [x_pos, y_pos] = translate(x_value, y_value, width, height);
        x_pos += margin_;
        y_pos += margin_;

        cairo_arc(cache_cr, x_pos, y_pos, point_size / 2.0, 0, 2 * M_PI);
        if (point_border_width > 0) {
            cairo_set_source_rgba(cache_cr, point_border_color.r, point_border_color.g, point_border_color.b, point_border_color.a);
            cairo_set_line_width(cache_cr, point_border_width);
            cairo_stroke_preserve(cache_cr);
        }

        cairo_set_source_rgba(cache_cr, point_color.r, point_color.g, point_color.b, point_color.a);
        cairo_fill(cache_cr);
        point_cache_drawn_ = true;
    }

    cairo_destroy(cache_cr);
}

void ChartWidget::redraw_extremes_cache(double width, double height, time::microseconds_t timestamp,
                                        std::shared_ptr<NumericParameter::sections_t> x_values,
                                        std::shared_ptr<NumericParameter::sections_t> y_values) {
    int surface_width = 0;
    int surface_height = 0;

    if (extremes_cache_) {
        surface_width = cairo_image_surface_get_width(extremes_cache_);
        surface_height = cairo_image_surface_get_height(extremes_cache_);
    }

    if (!extremes_cache_ || surface_width < cache_width_ || surface_height < cache_height_) {
        // bigger widget size require allocating bigger cache
        if (extremes_cache_) {
            cairo_surface_destroy(extremes_cache_);
            extremes_cache_ = nullptr;
        }
        extremes_cache_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, cache_width_, cache_height_);
        extremes_cache_drawn_ = false;
        log.info("Allocated new extremes cache surface: {}x{}", cache_width_, cache_height_);
    }

    cairo_t* cache_cr = cairo_create(extremes_cache_);
    if (extremes_cache_drawn_) {
        // clear cache
        cairo_save(cache_cr);
        cairo_set_operator(cache_cr, CAIRO_OPERATOR_CLEAR);
        cairo_paint(cache_cr);
        cairo_restore(cache_cr);
        extremes_cache_drawn_ = false;
    }

    bool draw_max = max_marker_style_.enabled->get_value(timestamp);
    bool draw_min = min_marker_style_.enabled->get_value(timestamp);

    if ((draw_max || draw_min) && x_values && y_values && !x_values->empty() && !y_values->empty()) {
        if (draw_max) {
            draw_direction_markers(cache_cr, width, height, timestamp, *x_values, *y_values, true, max_marker_style_);
        }
        if (draw_min) {
            draw_direction_markers(cache_cr, width, height, timestamp, *x_values, *y_values, false, min_marker_style_);
        }
    }

    cairo_destroy(cache_cr);
}

void ChartWidget::draw_direction_markers(cairo_t* cache_cr, double width, double height, time::microseconds_t timestamp,
                                         const NumericParameter::sections_t& x_values,
                                         const NumericParameter::sections_t& y_values,
                                         bool find_max, const MarkerStyle& style) {
    double window = marker_window_ ? marker_window_->get_value(timestamp) : 0.0;
    auto extrema = find_local_extrema(x_values, y_values, window, find_max);
    if (extrema.empty()) {
        return;
    }

    rgb marker_color = style.color->get_value(timestamp);
    double radius = style.radius->get_value(timestamp);
    double border_width = style.border_width->get_value(timestamp);
    rgb border_color = style.border_color->get_value(timestamp);
    rgb label_color = style.label_color->get_value(timestamp);
    ELabelPosition position = label_position_from_string(
        style.label_position->get_value(timestamp),
        find_max ? ELabelPosition::Top : ELabelPosition::Bottom);

    std::string format = marker_format_->get_value(timestamp);
    std::string font = std::format("{} {}", marker_font_name_->get_value(timestamp),
                                   static_cast<int>(std::round(marker_font_size_->get_value(timestamp))));
    double label_offset = marker_label_offset_->get_value(timestamp);
    double label_border_width = marker_label_border_width_->get_value(timestamp);
    rgb label_border_color = marker_label_border_color_->get_value(timestamp);

    for (const auto& pt : extrema) {
        auto [x_pos, y_pos] = translate(pt.x_val, pt.y_val, width, height);
        x_pos += margin_;
        y_pos += margin_;

        std::string label = std::vformat(format, std::make_format_args(pt.y_val));

        draw_marker(cache_cr, x_pos, y_pos, radius, marker_color, border_width, border_color,
                    label, font, label_color, label_border_width, label_border_color,
                    position, label_offset);
    }
}

void ChartWidget::draw_marker(cairo_t* cache_cr, double x_pos, double y_pos,
                              double radius, rgb color, double border_width, rgb border_color,
                              const std::string& label, const std::string& font,
                              rgb label_color, double label_border_width, rgb label_border_color,
                              ELabelPosition position, double label_offset) const {
    // clear any leftover path/current-point from a previously drawn marker in
    // this same cache - cairo_arc() implicitly draws a connecting line from a
    // stale current point (e.g. left behind by the previous label's
    // pango_cairo_show_layout(), which cairo_save/cairo_restore does not clear)
    cairo_new_path(cache_cr);

    if (radius > 0) {
        cairo_arc(cache_cr, x_pos, y_pos, radius, 0, 2 * M_PI);
        if (border_width > 0) {
            cairo_set_source_rgba(cache_cr, border_color.r, border_color.g, border_color.b, border_color.a);
            cairo_set_line_width(cache_cr, border_width);
            cairo_stroke_preserve(cache_cr);
        }
        // transparent fill by default - the marker is meant to ring the data
        // point, not hide it
        cairo_set_source_rgba(cache_cr, color.r, color.g, color.b, color.a);
        cairo_fill(cache_cr);
    }

    if (label.empty()) {
        return;
    }

    TextSize size = measure_text(cache_cr, label, font);
    double edge = radius + border_width + label_offset;

    double box_x = x_pos - size.width / 2.0;
    double box_y = y_pos - edge - size.height;
    ETextAlign align = ETextAlign::Center;

    switch (position) {
        case ELabelPosition::Top:
            box_x = x_pos - size.width / 2.0;
            box_y = y_pos - edge - size.height;
            align = ETextAlign::Center;
            break;
        case ELabelPosition::Bottom:
            box_x = x_pos - size.width / 2.0;
            box_y = y_pos + edge;
            align = ETextAlign::Center;
            break;
        case ELabelPosition::Left:
            box_x = x_pos - edge - size.width;
            box_y = y_pos - size.height / 2.0;
            align = ETextAlign::Right;
            break;
        case ELabelPosition::Right:
            box_x = x_pos + edge;
            box_y = y_pos - size.height / 2.0;
            align = ETextAlign::Left;
            break;
        case ELabelPosition::TopLeft:
            box_x = x_pos - edge - size.width;
            box_y = y_pos - edge - size.height;
            align = ETextAlign::Right;
            break;
        case ELabelPosition::TopRight:
            box_x = x_pos + edge;
            box_y = y_pos - edge - size.height;
            align = ETextAlign::Left;
            break;
        case ELabelPosition::BottomLeft:
            box_x = x_pos - edge - size.width;
            box_y = y_pos + edge;
            align = ETextAlign::Right;
            break;
        case ELabelPosition::BottomRight:
            box_x = x_pos + edge;
            box_y = y_pos + edge;
            align = ETextAlign::Left;
            break;
    }

    // draw_text() lays text out within a fixed-size box inset by `margin` from
    // its origin; translate so (box_x, box_y) becomes that box's top-left corner.
    cairo_save(cache_cr);
    cairo_translate(cache_cr, box_x, box_y);
    draw_text(cache_cr, size.width, size.height, 0, label, font, align, label_color,
              label_border_width, label_border_color);
    cairo_restore(cache_cr);
}

std::vector<ChartWidget::ExtremePoint> ChartWidget::find_local_extrema(
        const NumericParameter::sections_t& x_values,
        const NumericParameter::sections_t& y_values,
        double window, bool find_max) const {
    std::vector<ExtremePoint> result;
    if (window <= 0.0) {
        return result;
    }

    auto find_y_value = [&](time::microseconds_t ts) -> double {
        for (const auto& section : y_values) {
            auto it = section.find(ts);
            if (it != section.end()) {
                return it->second;
            }
        }
        return std::numeric_limits<double>::quiet_NaN();
    };

    auto better = [&](double a, double b) { return find_max ? (a >= b) : (a <= b); };

    // processes one gap-free, x-ascending run: flags each point that is the
    // extreme value within +/- window/2 of it (a "candidate"), but only once
    // that neighborhood is fully contained within the run's own x-range -
    // otherwise a point still climbing at the trailing edge of a live/moving
    // chart would be flagged as a fake local max just because nothing higher
    // has arrived *yet*. Confirmed candidates within `window` of each other are
    // then merged into a single marker (their true extreme) to avoid multiple
    // circles on one noisy/flat-top peak.
    auto process_run = [&](const std::vector<ExtremePoint>& run) {
        size_t n = run.size();
        if (n < 2) {
            return;
        }

        std::vector<bool> is_candidate(n, false);
        std::deque<size_t> dq; // indices, front = current window's extreme
        size_t lo = 0, hi = 0;

        for (size_t c = 0; c < n; c++) {
            // absorb every point within window/2 to the right of c
            while (hi < n && run[hi].x_val <= run[c].x_val + window / 2.0) {
                while (!dq.empty() && better(run[hi].y_val, run[dq.back()].y_val)) {
                    dq.pop_back();
                }
                dq.push_back(hi);
                hi++;
            }
            // drop points that fell out of window/2 to the left of c
            while (lo < c && run[lo].x_val < run[c].x_val - window / 2.0) {
                if (!dq.empty() && dq.front() == lo) {
                    dq.pop_front();
                }
                lo++;
            }

            bool full_left = (run[c].x_val - window / 2.0) >= run.front().x_val;
            bool full_right = (run[c].x_val + window / 2.0) <= run.back().x_val;
            if (full_left && full_right && !dq.empty() && dq.front() == c) {
                is_candidate[c] = true;
            }
        }

        std::vector<size_t> candidates;
        for (size_t i = 0; i < n; i++) {
            if (is_candidate[i]) {
                candidates.push_back(i);
            }
        }

        size_t i = 0;
        while (i < candidates.size()) {
            size_t j = i;
            size_t best = candidates[i];
            while (j + 1 < candidates.size() &&
                   (run[candidates[j + 1]].x_val - run[candidates[i]].x_val) <= window) {
                j++;
                if (better(run[candidates[j]].y_val, run[best].y_val)) {
                    best = candidates[j];
                }
            }
            result.push_back(run[best]);
            i = j + 1;
        }
    };

    std::vector<ExtremePoint> run;
    for (const auto& section : x_values) {
        for (const auto& [ts, x_val] : section) {
            double y_val = find_y_value(ts);
            if (std::isnan(x_val) || std::isnan(y_val)) {
                process_run(run);
                run.clear();
                continue;
            }
            run.push_back(ExtremePoint{ts, x_val, y_val});
        }
        process_run(run);
        run.clear();
    }

    return result;
}

void ChartWidget::recalculate_extremes(std::shared_ptr<NumericParameter::sections_t> x_values,
                                       std::shared_ptr<NumericParameter::sections_t> y_values) {
    invalid_ = false;// reset invalid state
    if (lock_x_minmax_ && lock_y_minmax_) {
        // both min and max are locked, no need to recalculate
        return;
    }

    if (!x_values || x_values->empty() || !y_values || y_values->empty()) {
        log.error("Extremes recalculation failure - no x or y values available");
        invalid_ = true;
        return;
    }

    if (!lock_x_minmax_) {  
        min_x_ = std::numeric_limits<double>::max();
        max_x_ = std::numeric_limits<double>::min();
        for (const auto& section: *x_values) {
            for (const auto& [ts, x_val] : section) {
                if (std::isnan(x_val)) {
                    continue; // skip NaN values
                }
                if (x_val < min_x_) {
                    min_x_ = x_val;
                }
                if (x_val > max_x_) {
                    max_x_ = x_val;
                }
            }
        }
        if (min_x_ >= max_x_) {
            log.error("Extremes recalculation failure - min_x ({}) >= max_x ({})", min_x_, max_x_);
            invalid_ = true;
        }
    }

    if (!lock_y_minmax_) {
        min_y_ = std::numeric_limits<double>::max();
        max_y_ = std::numeric_limits<double>::min();
        for (const auto& section: *y_values) {
            for (const auto& [ts, y_val] : section) {
                if (std::isnan(y_val)) {
                    continue; // skip NaN values
                }

                if (y_val < min_y_) {
                    min_y_ = y_val;
                }
                if (y_val > max_y_) {
                    max_y_ = y_val;
                }
            }
        }
        if (min_y_ >= max_y_) {
            log.error("Extremes recalculation failure - min_y ({}) >= max_y ({})", min_y_, max_y_);
            invalid_ = true;
        }
    }
}

std::pair<double, double> ChartWidget::translate(double x_value, double y_value, double width, double height) const {
    double x_range = max_x_ - min_x_;
    double y_range = max_y_ - min_y_;

    double x_scale = width / x_range;
    double y_scale = height / y_range;

    double x_offset = 0;
    double y_offset = 0;

    if (!stretch_chart_) {
        if (x_scale < y_scale) {
            double used_height = y_range * x_scale;
            y_offset = (height - used_height) / 2.0;

            y_scale = x_scale;
        } else if (y_scale < x_scale) {
            double used_width = x_range * y_scale;
            x_offset = (width - used_width) / 2.0;

            x_scale = y_scale;
        }
    }

    double x_pos = (x_value - min_x_) * x_scale + x_offset;
    double y_pos = height - y_offset - (y_value - min_y_) * y_scale; // invert y axis and apply centering offset
    return {x_pos, y_pos};
}

} // namespace overlay
} // namespace telemetry
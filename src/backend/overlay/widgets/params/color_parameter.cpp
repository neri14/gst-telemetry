#include "color_parameter.h"

#include "backend/utils/string_utils.h"

namespace telemetry {
namespace overlay {

std::shared_ptr<ColorParameter> ColorParameter::create(
        const std::string& definition, std::shared_ptr<track::Track> track) {
    std::string def{definition};
    trim(def);

    utils::logging::Logger log{"ColorParameter::create"};
    log.debug("Creating ColorParameter with definition: {}", def);

    bool rgb_def = get_function_name(def) == "rgb";
    bool rgba_def = get_function_name(def) == "rgba";
    if (rgb_def || rgba_def) {

        std::string r_def, g_def, b_def, a_def;

        def = get_function_argstr(def);

        size_t pos1 = def.find(',');
        size_t pos2 = def.find(',', pos1 + 1);
        size_t pos3 = std::string::npos;
        if (rgba_def) {
            pos3 = def.find(',', pos2 + 1);
        }

        r_def = def.substr(0, pos1);
        g_def = def.substr(pos1 + 1, pos2 - pos1 - 1);
        b_def = def.substr(pos2 + 1, (rgba_def ? (pos3 - pos2 - 1) : std::string::npos));
        if (rgba_def) {
            a_def = def.substr(pos3 + 1);
        } else {
            a_def = "1.0"; // default alpha to 1.0
        }

        auto r_param = NumericParameter::create(r_def, track);
        auto g_param = NumericParameter::create(g_def, track);
        auto b_param = NumericParameter::create(b_def, track);
        auto a_param = NumericParameter::create(a_def, track);

        if (!r_param || !g_param || !b_param || !a_param) {
            log.warning("Failed to create sub-parameters for color definition '{}'", def);
            return nullptr;
        }

        log.debug("Created sub-parameter-based color parameter, r:'{}' g:'{}' b:'{}' a:'{}'",
                  r_def, g_def, b_def, a_def);
        return std::make_shared<ColorParameter>(r_param, g_param, b_param, a_param);
    }

    // if begins with "key(" and ends with ")" - track key
    if (get_function_name(def) == "key") {
        std::string key = get_function_argstr(def);
        log.debug("Created track-key-based color parameter with key '{}'", key);
        return std::make_shared<ColorParameter>(key, track);
    }

    // otherwise try to parse as static color value
    rgb static_value = color_from_string(def);
    log.debug("Created static color parameter with value r:{} g:{} b:{} a:{}", 
              static_value.r, static_value.g, static_value.b, static_value.a);
    return std::make_shared<ColorParameter>(static_value);
}

ColorParameter::ColorParameter(std::shared_ptr<NumericParameter> r_param,
                   std::shared_ptr<NumericParameter> g_param,
                   std::shared_ptr<NumericParameter> b_param,
                   std::shared_ptr<NumericParameter> a_param)
        : update_strategy_(UpdateStrategy::SubParameter),
          r_param_(r_param), g_param_(g_param), b_param_(b_param), a_param_(a_param) {
}

ColorParameter::ColorParameter(const std::string& key, std::shared_ptr<track::Track> track)
        : update_strategy_(UpdateStrategy::TrackKey),
          track_(track) {
    field_id = track_->get_field_id(key);
}

ColorParameter::ColorParameter(rgb static_value)
        : update_strategy_(UpdateStrategy::Static),
          value_(static_value) {
}

bool ColorParameter::update(time::microseconds_t timestamp) {
    bool valid = (value_ != color::invalid);

    switch (update_strategy_) {
        case UpdateStrategy::Static:
            return false; // static value does not change
        case UpdateStrategy::TrackKey:
            if (track_) {
                track::Value v = track_->get(field_id, timestamp);
                rgb new_value = color_from_string(v.as_string());
                if (new_value != value_) {
                    value_ = new_value;
                    return true;
                }
            }
            return false;
        case UpdateStrategy::SubParameter: {
            bool changed = false;
            rgb new_value = value_;

            if (r_param_ && (!valid || r_param_->update(timestamp))) {
                new_value.r = std::clamp(r_param_->get_value(timestamp), 0.0, 1.0);
                changed = true;
            }
            if (g_param_ && (!valid || g_param_->update(timestamp))) {
                new_value.g =  std::clamp(g_param_->get_value(timestamp), 0.0, 1.0);
                changed = true;
            }
            if (b_param_ && (!valid || b_param_->update(timestamp))) {
                new_value.b =  std::clamp(b_param_->get_value(timestamp), 0.0, 1.0);
                changed = true;
            }
            if (a_param_ && (!valid || a_param_->update(timestamp))) {
                new_value.a =  std::clamp(a_param_->get_value(timestamp), 0.0, 1.0);
                changed = true;
            }

            if (changed) {
                value_ = new_value;
            }
            return changed;
        }
        default:
            log.warning("Unknown update strategy in ColorParameter");
            return false;
    }
}

rgb ColorParameter::get_value(time::microseconds_t timestamp) const {
    if (value_ == color::invalid) {
        log.warning("ColorParameter has invalid value at timestamp {}, defaulting to white", timestamp);
        return color::white;
    }
    return value_;
}

bool ColorParameter::is_static() const {
    if (update_strategy_ == UpdateStrategy::SubParameter) {
        return (!r_param_ || r_param_->is_static()) &&
               (!g_param_ || g_param_->is_static()) &&
               (!b_param_ || b_param_->is_static()) &&
               (!a_param_ || a_param_->is_static());
    }
    return update_strategy_ == UpdateStrategy::Static;
}

} // namespace telemetry
} // namespace overlay

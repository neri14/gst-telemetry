#ifndef TRACK_H
#define TRACK_H

#include <chrono>
#include <format>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <stdint.h>

#include <pugixml.hpp>

#include "backend/utils/logging/logger.h"
#include "backend/utils/time.h"
#include "value.h"

namespace telemetry {
namespace track {

using field_id_t = uint32_t;
static constexpr field_id_t INVALID_FIELD = UINT32_MAX;

class Track {
public:
    Track(time::microseconds_t offset = 0);
    ~Track() = default;

    bool load(const std::string& path);

    field_id_t get_field_id(const std::string& field_name) const;

    //TODO add argument to getters to specify retrieve LAST, NEXT or INTERPOLATED value
    Value get(const std::string& key, time::microseconds_t timestamp = time::INVALID_TIME) const;
    Value get(field_id_t field_id, time::microseconds_t timestamp = time::INVALID_TIME) const;

    Value get_metadata(const std::string& key) const;
    Value get_metadata(field_id_t field_id) const;

    Value get_trackpoint_data(const std::string& key, time::microseconds_t timestamp) const;
    Value get_trackpoint_data(field_id_t field_id, time::microseconds_t timestamp) const;

    Value get_virtual_data(const std::string& key, time::microseconds_t timestamp) const;
    Value get_virtual_data(field_id_t field_id, time::microseconds_t timestamp) const;

private:
    mutable utils::logging::Logger log{"track"};

    bool parse_gpx(pugi::xml_node node);
    bool parse_metadata(pugi::xml_node node);

    bool parse_trk(pugi::xml_node node);
    time::time_point_t find_start_time(
        pugi::xml_object_range<pugi::xml_named_node_iterator> trksegs);

    bool parse_trk_ext(pugi::xml_node node);
    bool parse_trk_ext_atx(pugi::xml_node node);
    bool parse_trk_ext_asx(pugi::xml_node node);

    bool parse_trkseg(pugi::xml_node node);
    bool parse_trkpt(pugi::xml_node node);

    bool store_metadata(const std::string& key, const Value& value);
    bool store_trackpoint_data(
        time::microseconds_t timestamp,
        const std::string& key, const Value& value);
    void create_virtual_fields();

    field_id_t register_metadata_field(const std::string& key);
    field_id_t register_trackpoint_field(const std::string& key);
    field_id_t register_segment_field(const std::string& key);
    field_id_t register_virtual_field(const std::string& key);

    time::microseconds_t to_relative_time_domain(time::time_point_t timestamp) const;

    std::map<std::string, field_id_t> field_ids_;
    field_id_t next_field_id_ = 0;

    std::map<field_id_t, Value> metadata_;
    std::map<time::microseconds_t, std::shared_ptr<std::map<field_id_t, Value>>> trackpoints_;
    std::map<field_id_t, std::function<Value(time::microseconds_t)>> virtual_data_mapping_;

    time::microseconds_t min_timestamp_ = time::INVALID_TIME;
    time::microseconds_t max_timestamp_ = 0;

    time::time_point_t start_time_ = time::INVALID_TIME_POINT;
    time::microseconds_t start_offset_ = 0;
};

} // namespace track
} // namespace telemetry

#endif // TRACK_H

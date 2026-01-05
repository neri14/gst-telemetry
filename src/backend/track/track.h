#ifndef TRACK_H
#define TRACK_H

#include <chrono>
#include <format>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
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
    using fields_map_t = std::map<field_id_t, Value>;
    using trackpoint_data_map_t = std::map<time::microseconds_t, std::shared_ptr<fields_map_t>>;
    using segments_metadata_map_t = std::map<field_id_t, std::shared_ptr<fields_map_t>>;
    using trackpoint_ts_view_t = std::ranges::keys_view<std::ranges::ref_view<const trackpoint_data_map_t>>;

    Track(time::microseconds_t offset = 0);
    ~Track() = default;

    bool load(const std::string& path);

    field_id_t get_field_id(const std::string& field_name) const;

    trackpoint_ts_view_t get_trackpoint_timestamps() const;

    Value get(const std::string& key, time::microseconds_t timestamp = time::INVALID_TIME) const;
    Value get(field_id_t field_id, time::microseconds_t timestamp = time::INVALID_TIME) const;

    Value get_metadata(field_id_t field_id) const;
    
    Value get_trackpoint_data(field_id_t field_id, time::microseconds_t timestamp) const;
    Value get_lerp_trackpoint_data(field_id_t field_id, time::microseconds_t timestamp) const;
    Value get_pchip_trackpoint_data(field_id_t field_id, time::microseconds_t timestamp) const;
    
    Value get_virtual_data(field_id_t field_id, time::microseconds_t timestamp) const;
    
    Value get_segment_data(field_id_t field_id, time::microseconds_t timestamp) const;
    Value get_segment_metadata(field_id_t field_id) const;

private:
    mutable utils::logging::Logger log{"track"};

    using segments_lut_t = std::map<field_id_t, std::map<field_id_t, std::pair<time::time_point_t, time::time_point_t>>>;
    /* segments[segment type id][instance index] = {start time, end time} */

    bool parse_gpx(pugi::xml_node node);
    bool parse_metadata(pugi::xml_node node);

    bool parse_trk(pugi::xml_node node);
    time::time_point_t find_start_time(
        pugi::xml_object_range<pugi::xml_named_node_iterator> trksegs);

    bool parse_trk_ext(pugi::xml_node node);
    bool parse_trk_ext_atx(pugi::xml_node node);
    bool parse_trk_ext_asx(pugi::xml_node node);

    bool parse_trk_ext_asx_segment(pugi::xml_node node);
    void generate_sorted_segment_lists();
    void generate_segment_metadata_field_aliases();
    void generate_segment_virtual_metadata_fields();

    bool parse_trkseg(pugi::xml_node node);
    bool parse_trkpt(pugi::xml_node node);

    bool store_metadata(const std::string& key, const Value& value);
    bool store_trackpoint_data(
        time::microseconds_t timestamp,
        const std::string& key, const Value& value);
    void create_virtual_fields();

    std::vector<field_id_t> get_active_segments_ordered(field_id_t segment_type, time::microseconds_t timestamp) const;
    std::vector<field_id_t> get_prev_segments_ordered(field_id_t segment_type, time::microseconds_t timestamp) const;
    std::vector<field_id_t> get_next_segments_ordered(field_id_t segment_type, time::microseconds_t timestamp) const;

    field_id_t register_metadata_field(const std::string& key);
    field_id_t register_trackpoint_field(const std::string& key);
    field_id_t register_virtual_field(const std::string& key);

    field_id_t register_field(const std::string& key, field_id_t mask);

    time::microseconds_t to_relative_time_domain(time::time_point_t timestamp) const;

    std::map<std::string, field_id_t> field_ids_;
    field_id_t next_field_id_ = 0;

    fields_map_t metadata_;
    trackpoint_data_map_t trackpoints_;
    std::map<field_id_t, std::function<Value(time::microseconds_t)>> virtual_data_mapping_;

    std::map<std::string, field_id_t> segment_types_;
    segments_lut_t segments_lut_;
    std::map<field_id_t, std::vector<field_id_t>> segments_ordered_by_start_time_;
    std::map<field_id_t, std::vector<field_id_t>> segments_ordered_by_end_time_;

    std::map<std::string, field_id_t> segment_metadata_partial_field_ids_;
    field_id_t next_segment_metadata_field_id_ = 0;

    time::microseconds_t min_timestamp_ = time::INVALID_TIME;
    time::microseconds_t max_timestamp_ = 0;

    time::time_point_t start_time_ = time::INVALID_TIME_POINT;
    time::microseconds_t start_offset_ = 0;
};

} // namespace track
} // namespace telemetry

#endif // TRACK_H


// NOTES ON SEGMENTS:

// ### SEGMENT DATA ###
// # put segment summary data (as is in the gpx)
// # into (...)_meta_(...)
// # and where applicable create virtual fields
// # like the ones for point but relative to segment start/end
// # AND DOCUMENT EVERYTHING IN MARKDOWN TABLE IN REPO
// # which fields from gpx are visible how etc...

// # e.g.
// s_climb_next_meta_ascent # next climb total ascent
// s_climb_dist # current climb distance since climb start
// s_segment_timer # current strava segment time since start


// ### SEGMENT PREFIXES ###

// # prefixes (tbd prev or last?)

// s_TYPE_        # currently active
// s_TYPE_prev_   # next active
// s_TYPE_next_   # last active

// # lists

// # all active segments in order of start
// #(making a list where last finished stays for few seconds before dissapears will be tricky)
// #(but should be doable)

// s_TYPE_N_         # all currently active segments of a type ordered by start time ascending
// s_TYPE_prev_N_    # ordered by finish time descending (so last, second to last to finish etc)
// s_TYPE_next_N_    # ordered by start time aascending

// # where N:=<1, ...)
// # and s_TYPE_1_ == s_TYPE_
// # and s_TYPE_prev_1_ == s_TYPE_prev_
// # and s_TYPE_next_1_ == s_TYPE_next_



// # plus all segments list

// s_TYPE_lst_N_   # all segments of a type (e.g. for a summary screen)


// # examples

// s_climb_next_meta_ascent         # total ascent of next climb
// s_segment_prev_meta_elapsedtime  # total time of last strava segment




// NOTES ON SEGMENT GRAPHS

// # graphs (for segments, but actually same design could be reused between segments and "moving graphs" just with different start-end points(?)

// graph widget needs to take parameters (naming tbd):
// start_time = ..
// end_time = ..

// (a new type of widget parameter?)

// use cases:
// - segments: keys from which to take start and end timestamp of graph (wont change as long as segment is visible)
// - elevation graph: whole activity - either no params or keywords "begin" "end" or virtual(?) keys providing begin and end times?
// - moving graph: either - end_time="time" key, start_time??? or if numeric value provided - treat it as relative to current time and then start_time=-30 end_time=0 ?


// also need to take care of how to treat continous graphs?
// e.g. if the interpolated value is provided use x-step parameter to define distance between points (with some reasonable default), if non-interpolated key used no x-step needed?

// (also need to consider how to treat mix of x-interpolated y-nonointerpolated and other way around)
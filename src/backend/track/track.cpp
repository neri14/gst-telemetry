#include "track.h"

#include "backend/utils/time.h"


std::string drop_ns(const std::string& name) {
    auto pos = name.rfind(':');
    return (pos == std::string::npos) ? name : name.substr(pos + 1);
}

namespace telemetry {
namespace track {
namespace consts {
    const field_id_t max_id = 0x00FFFFFF;

    const field_id_t metadata_id_mask = 0x08000000;
    const field_id_t trackpoint_id_mask = 0x04000000;
    const field_id_t segment_id_mask = 0x02000000;
    const field_id_t virtual_id_mask = 0x01000000;

    const std::string metadata_prefix_ = "metadata.";
    const std::string trackpoint_prefix_ = "point.";
    const std::string segment_prefix_ = "segment.";
}

Track::Track(time::microseconds_t offset): start_offset_(offset) {
    log.info("Track created with offset: {} us", offset);

    create_virtual_fields();
}

bool Track::load(const std::string& path) {
    log.info("Loading track from path: {}", path);

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(path.c_str());

    if (!result) {
        log.error("Failed to load track file: {}. Error description: {}", path, result.description());
        return false;
    }

    if (doc.children().empty()) {
        log.error("Empty track file: {}", path);
        return false;
    }

    if (std::distance(doc.children().begin(), doc.children().end()) > 1) {
        log.error("More than one root node in track file");
        return false;
    }

    pugi::xml_node root = doc.child("gpx");
    if (!root) {
        log.error("No <gpx> root node found in track file");
        return false;
    }

    bool ok = parse_gpx(root);

    return ok;
}

field_id_t Track::get_field_id(const std::string& field_name) const {
    auto it = field_ids_.find(field_name);
    if (it != field_ids_.end()) {
        return it->second;
    }
    return INVALID_FIELD;
}

Track::trackpoint_ts_view_t Track::get_trackpoint_timestamps() const {
    return std::views::keys(trackpoints_);
}

Value Track::get(const std::string& key, time::microseconds_t timestamp) const {
    field_id_t field_id = get_field_id(key);
    return get(field_id, timestamp);
}

Value Track::get(field_id_t field_id, time::microseconds_t timestamp) const {
    if (field_id & consts::metadata_id_mask) {
        return get_metadata(field_id);
    } else if (field_id & consts::trackpoint_id_mask) {
        return get_trackpoint_data(field_id, timestamp);
    } else if (field_id & consts::segment_id_mask) {
        // TODO segment data retrieval - if possible without additional segment identifier?
        //      there can be multiple segments active at same time
        log.warning("Segment field data retrieval not implemented yet");
        return Value();
    } else if (field_id & consts::virtual_id_mask) {
        return get_virtual_data(field_id, timestamp);
    } else {
        log.warning("Unknown field id: {}", field_id);
        return Value();
    }
}

Value Track::get_metadata(const std::string& key) const {
    field_id_t field_id = get_field_id(key);
    return get_metadata(field_id);
}

Value Track::get_metadata(field_id_t field_id) const {
    auto it = metadata_.find(field_id);
    if (it != metadata_.end()) {
        return it->second;
    }
    return Value();
}

Value Track::get_trackpoint_data(const std::string& key, time::microseconds_t timestamp) const {
    field_id_t field_id = get_field_id(key);
    return get_trackpoint_data(field_id, timestamp);
}

Value Track::get_trackpoint_data(field_id_t field_id, time::microseconds_t timestamp) const {
    //TODO add caching of last accessed trackpoint (or similar)
    //TODO need to handle case where the data is stale?

    auto tp_it = trackpoints_.upper_bound(timestamp);

    if (tp_it != trackpoints_.begin()) {
        --tp_it;

        auto& tp_data = *(tp_it->second);
        auto field_it = tp_data.find(field_id);
        if (field_it != tp_data.end()) {
            return field_it->second;
        }
    }
    return Value();
}

Value Track::get_virtual_data(const std::string& key, time::microseconds_t timestamp) const {
    field_id_t field_id = get_field_id(key);
    return get_virtual_data(field_id, timestamp);
}

Value Track::get_virtual_data(field_id_t field_id, time::microseconds_t timestamp) const {
    auto it = virtual_data_mapping_.find(field_id);
    if (it != virtual_data_mapping_.end()) {
        return it->second(timestamp);
    }

    log.warning("Unknown virtual field id: {}", field_id);
    return Value();
}

bool Track::parse_gpx(pugi::xml_node node) {
    log.info("Parsing GPX root node");

    bool ok = true;

    pugi::xml_node metadata = node.child("metadata");
    if (metadata) {
        ok = parse_metadata(metadata) && ok;
    }

    pugi::xml_node trk = node.child("trk");
    if (trk) {
        ok = parse_trk(trk) && ok;
    }

    return ok;
}


bool Track::parse_metadata(pugi::xml_node node) {
    log.info("Parsing GPX metadata");

    auto bounds = node.child("bounds");
    if (bounds) {
        auto minlat = bounds.attribute("minlat");
        if (minlat) {
            double value = minlat.as_double();
            log.debug("Metadata minlat: {}", value);
            store_metadata("minlat", value);
        }
        auto maxlat = bounds.attribute("maxlat");
        if (maxlat) {
            double value = maxlat.as_double();
            log.debug("Metadata maxlat: {}", value);
            store_metadata("maxlat", value);
        }
        auto minlon = bounds.attribute("minlon");
        if (minlon) {
            double value = minlon.as_double();
            log.debug("Metadata minlon: {}", value);
            store_metadata("minlon", value);
        }
        auto maxlon = bounds.attribute("maxlon");
        if (maxlon) {
            double value = maxlon.as_double();
            log.debug("Metadata maxlon: {}", value);
            store_metadata("maxlon", value);
        }
    }

    auto name = node.child("name");
    if (name) {
        log.debug("Metadata name: {}", name.text().as_string());
        store_metadata("name", std::string(name.text().as_string()));
    }

    return true;
}

bool Track::parse_trk(pugi::xml_node node) {
    log.info("Parsing GPX track");

    auto name = node.child("name");
    if (name) {
        log.debug("Track name: {}", name.text().as_string());
        store_metadata("name", std::string(name.text().as_string()));
    }
    auto src = node.child("src");
    if (src) {
        log.debug("Track src: {}", src.text().as_string());
        store_metadata("src", std::string(src.text().as_string()));
    }
    auto type = node.child("type");
    if (type) {
        log.debug("Track type: {}", type.text().as_string());
        store_metadata("type", std::string(type.text().as_string()));
    }

    start_time_ = find_start_time(node.children("trkseg"));
    if (start_time_ == time::INVALID_TIME_POINT) {
        log.error("Failed to find valid start timestamp in track");
        return false;
    }
    log.info("Track start time: {}", std::format("{}", start_time_));

    bool ok = true;

    auto extensions = node.child("extensions");
    if (extensions) {
        ok = parse_trk_ext(extensions) && ok;
    }

    for (auto trkseg: node.children("trkseg")) {
        ok = parse_trkseg(trkseg) && ok;
    }

    return ok;
}

time::time_point_t Track::find_start_time(
                pugi::xml_object_range<pugi::xml_named_node_iterator> trksegs) {
    time::time_point_t st = time::INVALID_TIME_POINT;
    for (auto trkseg: trksegs) {
        for (auto trkpt : trkseg.children("trkpt")) {
            if (trkpt.child("time")) {
                std::string time_str = trkpt.child("time").text().as_string();
                auto timestamp = time::parse_iso8601(time_str);
                if (timestamp != time::INVALID_TIME_POINT) {
                    if (st == time::INVALID_TIME_POINT || timestamp < st) {
                        st = timestamp;
                    }
                }
            }
        }
    }
    return st;
}

bool Track::parse_trk_ext(pugi::xml_node node) {
    log.info("Parsing GPX track extensions");
    bool ok = true;
    
    for (pugi::xml_node child : node.children()) {
        if (std::string(child.name()).ends_with("ActivityTrackExtension")) {
            ok = parse_trk_ext_atx(child) && ok;
        } else if (std::string(child.name()).ends_with("ActivitySegmentsExtension")) {
            ok = parse_trk_ext_asx(child) && ok;
        } else {
            log.info("Ignoring unknown track extension: {}", child.name());
        }
    }

    return ok;
}

bool Track::parse_trk_ext_atx(pugi::xml_node node) {
    log.info("Parsing GPX ActivityTrackExtension");
    bool ok = true;

    int cnt = 0;
    for (pugi::xml_node child : node.children()) {
        std::string key =drop_ns(child.name());
        double value = child.text().as_double();

        log.debug("Metadata: {} = {}", key, value);
        store_metadata(key, value);
        cnt++;
    }
    log.info("Parsed {} ActivityTrackExtension fields", cnt);

    return ok;
}

bool Track::parse_trk_ext_asx(pugi::xml_node node) {
    log.info("Parsing GPX ActivitySegmentsExtension");
    // TODO implement parsing of ActivitySegmentsExtension
    return true;
}

bool Track::parse_trkseg(pugi::xml_node node) {
    log.info("Parsing GPX track segment");
    bool ok = true;

    for (pugi::xml_node trkpt : node.children("trkpt")) {
        ok = parse_trkpt(trkpt) && ok;
    }

    return ok;
}

bool Track::parse_trkpt(pugi::xml_node node) {
    log.debug("Parsing GPX track point");

    if (!node.attribute("lat") || !node.attribute("lon")) {
        log.warning("Track point missing lat or lon attribute");
        return false;
    }
    if (!node.child("time")) {
        log.warning("Track point missing time value");
        return false;
    }

    std::string time_str = node.child("time").text().as_string();
    auto timestamp = time::parse_iso8601(time_str);
    if (timestamp == time::INVALID_TIME_POINT) {
        log.warning("Failed to parse track point time: {}", time_str);
        return false;
    }
    log.debug("Track point time: {}", timestamp);

    time::microseconds_t ts = to_relative_time_domain(timestamp);

    store_trackpoint_data(ts, "time", Value(timestamp));

    double lat = node.attribute("lat").as_double();
    double lon = node.attribute("lon").as_double();
    store_trackpoint_data(ts, "lat", Value(lat));
    store_trackpoint_data(ts, "lon", Value(lon));
    log.debug("Track point lat: {}, lon: {}", lat, lon);

    for (pugi::xml_node child : node.children()) {
        std::string key = drop_ns(child.name());
        if (key == "time") {
            continue; // processed separately
        }
        if (key == "extensions") {
            for (pugi::xml_node ext_child : child.children()) {
                std::string ext_key = drop_ns(ext_child.name());
                if (ext_key == "TrackPointExtension" || ext_key == "ActivityTrackPointExtension") {
                    for (pugi::xml_node ext_data : ext_child.children()) {
                        std::string data_key = drop_ns(ext_data.name());
                        if (ext_data.text().empty()) {
                            log.debug("Track point extension data: {} has no data", data_key);
                            continue; // no data
                        }
                        double value = ext_data.text().as_double();
                        log.debug("Track point extension data: {} = {}", data_key, value);
                        store_trackpoint_data(ts, data_key, Value(value));
                    }
                } else {
                    log.debug("Ignoring unknown track point extension: {}", ext_key);
                }
            }
            continue; // processed
        }
        if (child.text().empty()) {
            log.debug("Track point data: {} has no data", key);
            continue; // no data
        }
        double value = child.text().as_double();
        log.debug("Track point data: {} = {}", key, value);
        store_trackpoint_data(ts, key, Value(value));
    }

    return true;
}

bool Track::store_metadata(const std::string& key, const Value& value) {
    field_id_t field_id = register_metadata_field(key);
    metadata_[field_id] = value;
    return true;
}

bool Track::store_trackpoint_data(time::microseconds_t timestamp,
                                  const std::string& key, const Value& value) {
    std::shared_ptr<std::map<field_id_t, Value>> tp_data;
    
    auto it = trackpoints_.find(timestamp);
    if (it != trackpoints_.end()) {
        tp_data = it->second;
    } else {
        tp_data = std::make_shared<std::map<field_id_t, Value>>();
        trackpoints_[timestamp] = tp_data;

        if (timestamp < min_timestamp_) {
            min_timestamp_ = timestamp;
        }
        if (timestamp > max_timestamp_) {
            max_timestamp_ = timestamp;
        }
    }

    field_id_t field_id = register_trackpoint_field(key);
    (*tp_data)[field_id] = value;
    return true;
}

void Track::create_virtual_fields() {
    virtual_data_mapping_[register_virtual_field("time_elapsed")] = [this](time::microseconds_t timestamp) -> Value {
        if (min_timestamp_ != time::INVALID_TIME) {
            return Value(time::us_to_s(timestamp - min_timestamp_));
        } else {
            return Value();
        }
    };

    virtual_data_mapping_[register_virtual_field("time_remaining")] = [this](time::microseconds_t timestamp) -> Value {
        if (max_timestamp_ != 0) {
            return Value(time::us_to_s(max_timestamp_ - timestamp));
        } else {
            return Value();
        }
    };

    virtual_data_mapping_[register_virtual_field("active")] = [this](time::microseconds_t timestamp) -> Value {
        if (min_timestamp_ != time::INVALID_TIME && max_timestamp_ != 0) {
            return Value(timestamp >= min_timestamp_ && timestamp <= max_timestamp_);
        } else {
            return Value(false);
        }
    };

    virtual_data_mapping_[register_virtual_field("countdown")] = [this](time::microseconds_t timestamp) -> Value {
        if (min_timestamp_ != time::INVALID_TIME) {
            time::microseconds_t diff = min_timestamp_ - timestamp;
            if (diff >= 0) {
                return Value(time::us_to_s(diff));
            }
        }
        return Value();
    };

    virtual_data_mapping_[register_virtual_field("overtime")] = [this](time::microseconds_t timestamp) -> Value {
        if (max_timestamp_ != 0) {
            time::microseconds_t diff = timestamp - max_timestamp_;
            if (diff >= 0) {
                return Value(time::us_to_s(diff));
            }
        }
        return Value();
    };
}

field_id_t Track::register_metadata_field(const std::string& key) {
    std::string metakey = consts::metadata_prefix_ + key;
    auto id = get_field_id(metakey);
    if (id != INVALID_FIELD) {
        return id;
    }

    id = next_field_id_++;
    id |= consts::metadata_id_mask;
    field_ids_[metakey] = id;

    log.debug("Registered new metadata field: {} with id {}", metakey, id);
    return id;
}

field_id_t Track::register_trackpoint_field(const std::string& key) {
    std::string tpkey = consts::trackpoint_prefix_ + key;
    auto id = get_field_id(tpkey);
    if (id != INVALID_FIELD) {
        return id;
    }

    id = next_field_id_++;
    id |= consts::trackpoint_id_mask;
    field_ids_[tpkey] = id;

    log.debug("Registered new trackpoint field: {} with id {}", tpkey, id);
    return id;
}

field_id_t Track::register_segment_field(const std::string& key) {
    std::string segkey = consts::segment_prefix_ + key;
    auto id = get_field_id(segkey);
    if (id != INVALID_FIELD) {
        return id;
    }

    id = next_field_id_++;
    id |= consts::segment_id_mask;
    field_ids_[segkey] = id;

    log.debug("Registered new segment field: {} with id {}", segkey, id);
    return id;
}

field_id_t Track::register_virtual_field(const std::string& key) {
    auto id = get_field_id(key);
    if (id != INVALID_FIELD) {
        return id;
    }

    id = next_field_id_++;
    id |= consts::virtual_id_mask;
    field_ids_[key] = id;

    log.debug("Registered new virtual field: {} with id {}", key, id);
    return id;
}

time::microseconds_t Track::to_relative_time_domain(time::time_point_t timestamp) const {
    auto us = static_cast<time::microseconds_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(timestamp - start_time_).count());
    return us + start_offset_;
}

} // namespace track
} // namespace telemetry

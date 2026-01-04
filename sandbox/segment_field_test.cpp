#include <algorithm>
#include <cstdint>
#include <iostream>
#include <set>

using field_id_t = uint32_t;

namespace consts {
    namespace mask {
        namespace segment {
            const field_id_t type         = 0x00F80000; // up to 32 segment types
            const field_id_t list         = 0x00070000; // up to 8 segment lists
            const field_id_t segment      = 0x0000FF00; // up to 256 segment instances (within type-list context)
            const field_id_t field        = 0x000000FF; // up to 256 fields (within segment point/segment metadata)
        }
    }
} // namespace consts


struct segment_field_id {
    field_id_t type;
    field_id_t list;
    field_id_t segment;
    field_id_t field;

    segment_field_id(field_id_t type, field_id_t list, field_id_t segment, field_id_t field)
        : type(type), list(list), segment(segment), field(field) {}

    segment_field_id(field_id_t fid) {
        type = (fid & consts::mask::segment::type) >> 19;
        list = (fid & consts::mask::segment::list) >> 16;
        segment = (fid & consts::mask::segment::segment) >> 8;
        field = fid & consts::mask::segment::field;
    }

    explicit operator field_id_t() const {
        return ((type << 19) & consts::mask::segment::type) |
               ((list << 16) & consts::mask::segment::list) |
               ((segment << 8) & consts::mask::segment::segment) |
               (field & consts::mask::segment::field);
    }
};

int main() {
    int checked = 0;
    int ok = 0;

    field_id_t last_id = 0xFFFFFFFF;

    for (uint32_t type = 0x0; type <= 0x1F; ++type) {
        for (uint32_t list = 0x0; list <= 0x7; ++list) {
            for (uint32_t segment = 0x0; segment <= 0xFF; ++segment) {
                for (uint32_t field = 0x0; field <= 0xFF; ++field) {
                    segment_field_id sfid = segment_field_id(type, list, segment, field);
                    ++checked;

                    field_id_t id = static_cast<field_id_t>(sfid);

                    if (sfid.type != type || sfid.list != list || sfid.segment != segment || sfid.field != field) {
                        std::cout << "Error - packed: type: " << type
                                  << " list: " << list
                                  << " segment: " << segment
                                  << " field: " << field << std::endl;
                        std::cout << "      unpacked: type: " << sfid.type
                                  << " list: " << sfid.list
                                  << " segment: " << sfid.segment
                                  << " field: " << sfid.field << std::endl;
                    } else if (last_id != 0xFFFFFFFF && last_id >= id) {
                        std::cout << "Error - not increasing id: " << id << " after " << last_id << std::endl;
                    } else {
                        ++ok;
                    }
                    last_id = id;
                }
            }
        }
    }

    std::cout << "packed-unpacked correctly: " << ok << " / " << checked << std::endl;

    return checked-ok;
}
#ifndef BOOLEAN_PARAMETER_H
#define BOOLEAN_PARAMETER_H

#include "parameter.h"
#include "backend/track/track.h"
#include "backend/utils/time.h"
#include <string>

namespace telemetry {
namespace overlay {

class BooleanParameter : public Parameter {
public:
    BooleanParameter(const std::string& definition, std::shared_ptr<track::Track> track);
    BooleanParameter(bool static_value);
    ~BooleanParameter() override = default;

    bool update(time::microseconds_t timestamp) override;
    bool get_value(time::microseconds_t timestamp) const;

private:
    std::string definition_;
    std::shared_ptr<track::Track> track_;

    bool static_value_ = false;
};

} // namespace telemetry
} // namespace overlay

#endif // BOOLEAN_PARAMETER_H



    // --> class: BooleanParameter
    // for boolean attributes (e.g. visibility, condition):
    //    "false"/"no"/empty string        -> use as boolean false
    //    "true"/"yes"/generic string       -> use as boolean true
    //    numeric value                   -> 0 = false, non-zero = true
    //    "key(...)"                  -> get string value from track at timestamp using key name inside parentheses
    //                                   (interpreted:
    //                                      if value is bool - as is
    //                                      if value is numeric - as above (0 = false, non-zero = true)
    //                                      if value is string - as above ("false"/"no"/empty = false, other = true)
    //                                      if key does not exist - false)
    //    "eval(...)"                 -> evaluate string inside parentheses as expression (resulting value interpreted as above bool/numeric)
    //    "key_exists(...)"           -> check if key inside parentheses is available in track at timestamp (resulting value is bool)
    //
    //    "not(...)"                  -> logical NOT of the boolean inside parentheses (only one NOT allowed)


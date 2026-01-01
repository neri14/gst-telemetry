#ifndef PARAMETER_H
#define PARAMETER_H

#include <map>
#include <string>
#include <memory>
#include "backend/utils/time.h"

namespace telemetry {
namespace overlay {

enum class ParameterType {
    Numeric,
    Color,
    String,
    Boolean,
    FormattedValue
};

class Parameter {
public:
    Parameter() = default;
    virtual ~Parameter() = default;
    virtual bool update(time::microseconds_t timestamp) = 0;

protected:
    enum class UpdateStrategy {
        Static,
        Expression,
        TrackKey
    };
};

using parameter_ptr_t = std::shared_ptr<Parameter>;
using parameter_map_t = std::map<std::string, parameter_ptr_t>;
using parameter_map_ptr = std::shared_ptr<parameter_map_t>;
using parameter_type_map_t = std::map<std::string, ParameterType>;




    // --> class: StringParameter
    // for string attributes (e.g. font name):
    //    string value         -> use as is
    //    "key(...)"           -> get string value from track at timestamp using key name inside parentheses

    
    // --> class: FormattedValueParameter
    // for string/numeric attribute - special "value" attribute case (can evaluate to string or numeric internally):
    //    string value         -> use as is (as string)
    //    numeric value        -> use as is (as double)
    //    "key(...)"           -> get value from track at timestamp using key name inside parentheses
    //                            (can result in any track::Value type - string, double, bool, time_point)
    //                            (since it'll be passed to std::vformat - no issue expected)
    //    "eval(...)"          -> evaluate string inside parentheses as expression (can result in numeric value only and take numeric key values only)
    //
    // format attribute interpreted like string attributes (direct string or key(...))
    //
    // result is formatted according to format attribute (if not provided - default "{}" is used)


} // namespace overlay
} // namespace telemetry

#endif // PARAMETER_H

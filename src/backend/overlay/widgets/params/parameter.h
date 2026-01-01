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
        TrackKey,
        Expression,
        SubParameter
    };
};

using parameter_ptr_t = std::shared_ptr<Parameter>;
using parameter_map_t = std::map<std::string, parameter_ptr_t>;
using parameter_map_ptr = std::shared_ptr<parameter_map_t>;
using parameter_type_map_t = std::map<std::string, ParameterType>;


//TODO move those to marmkdown files for documentation on layout definition

    //DONE
    // --> class: NumericParameter
    // for numeric attributes (x, y, radius, border-width):
    //    numeric value        -> use as is
    //    "eval(...)"          -> evaluate string inside parentheses as expression
    //    "key(...)"           -> get value from track at timestamp using key name inside parentheses
    //                            (note it can be achieved by eval(...) too, but this has less overhead)

    //DONE
    // --> class: ColorParameter
    // for color attributes (bg-color, border-color):
    //    string value         -> color name from predefined colors or "#RRGGBB" / "#RRGGBBAA" hex code
    //    "key(...)"           -> get string value from track at timestamp using key name inside parentheses (interpreted as above)
    //    "rgb(r,g,b)"         -> color from r,g,b values (0-1.0)
    //    "rgba(r,g,b,a)"      -> color from r,g,b,a values (0-1.0)
    //             each of r,g,b,a is interpreted like numeric attributes - same rules apply
    //             resulting value is clamped to 0.0-1.0 range

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

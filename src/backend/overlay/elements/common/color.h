#ifndef COLOR_H
#define COLOR_H

#include <string>
#include <map>

namespace telemetry {
namespace overlay {

struct rgba {
    double r;
    double g;
    double b;
    double a = 1.0;
};
using rgb = rgba;

namespace color {
    const rgb black   {0.00, 0.00, 0.00};
    const rgb white   {1.00, 1.00, 1.00};
    const rgb red     {1.00, 0.00, 0.00};
    const rgb lime    {0.00, 1.00, 0.00};
    const rgb blue    {0.00, 0.00, 1.00};
    const rgb yellow  {1.00, 1.00, 0.00};
    const rgb cyan    {0.00, 1.00, 1.00};
    const rgb aqua    {0.00, 1.00, 1.00};
    const rgb magenta {1.00, 0.00, 1.00};
    const rgb fuchsia {1.00, 0.00, 1.00};
    const rgb silver  {0.75, 0.75, 0.75};
    const rgb gray    {0.50, 0.50, 0.50};
    const rgb maroon  {0.50, 0.00, 0.00};
    const rgb olive   {0.50, 0.50, 0.00};
    const rgb green   {0.00, 0.50, 0.00};
    const rgb purple  {0.50, 0.00, 0.50};
    const rgb teal    {0.00, 0.50, 0.50};
    const rgb navy    {0.00, 0.00, 0.50};

    const std::map<std::string, rgba> map = {
        {"black"  , color::black},
        {"white"  , color::white},
        {"red"    , color::red},
        {"lime"   , color::lime},
        {"blue"   , color::blue},
        {"yellow" , color::yellow},
        {"cyan"   , color::cyan},
        {"aqua"   , color::aqua},
        {"magenta", color::magenta},
        {"fuchsia", color::fuchsia},
        {"silver" , color::silver},
        {"gray"   , color::gray},
        {"maroon" , color::maroon},
        {"olive"  , color::olive},
        {"green"  , color::green},
        {"purple" , color::purple},
        {"teal"   , color::teal},
        {"navy"   , color::navy}
    };
}

inline rgba color_from_string(const std::string& str)
{
    if (color::map.contains(str)) {
        return color::map.at(str);
    } else if (str.starts_with('#') && str.size() >= 7 && str.size() <= 9) {
        rgba res {
            std::stoi(str.substr(1, 2), nullptr, 16) / 255.0,
            std::stoi(str.substr(3, 2), nullptr, 16) / 255.0,
            std::stoi(str.substr(5, 2), nullptr, 16) / 255.0
        };

        if (str.size() == 9) {
            res.a = std::stoi(str.substr(7, 2), nullptr, 16) / 255.0;
        }

        return res;
    }

    return color::white;
}


} // namespace overlay
} // namespace telemetry

#endif // COLOR_H

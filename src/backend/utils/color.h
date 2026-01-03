#ifndef COLOR_H
#define COLOR_H

#include <string>
#include <map>
#include <limits>
#include <cmath>

namespace telemetry {

struct rgba {
    double r;
    double g;
    double b;
    double a = 1.0;

    bool operator==(const rgba& other) const {
        return std::abs(r - other.r) < std::numeric_limits<double>::epsilon() &&
               std::abs(g - other.g) < std::numeric_limits<double>::epsilon() &&
               std::abs(b - other.b) < std::numeric_limits<double>::epsilon() &&
               std::abs(a - other.a) < std::numeric_limits<double>::epsilon();
    }
    bool operator!=(const rgba& other) const {
        return !(*this == other);
    }
};
using rgb = rgba;

namespace color {// TODO more predefined colors
    const rgb transparent {0.0, 0.0, 0.0, 0.0};
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
    const rgb orange  {1.00, 0.65, 0.00};
    const rgb brown   {0.65, 0.16, 0.16};
    const rgb pink    {1.00, 0.75, 0.80};
    const rgb indigo  {0.29, 0.00, 0.51};
    const rgb violet  {0.93, 0.51, 0.93};
    const rgb gold    {1.00, 0.84, 0.00};
    const rgb coral   {1.00, 0.50, 0.31};
    const rgb salmon  {0.98, 0.50, 0.45};
    const rgb crimson {0.86, 0.08, 0.24};
    const rgb skyblue {0.53, 0.81, 0.92};
    const rgb lightblue {0.68, 0.85, 0.90};
    const rgb darkblue  {0.00, 0.00, 0.55};
    const rgb lightgreen{0.56, 0.93, 0.56};
    const rgb darkgreen {0.00, 0.39, 0.00};
    const rgb khaki   {0.94, 0.90, 0.55};
    const rgb beige   {0.96, 0.96, 0.86};
    const rgb chocolate{0.82, 0.41, 0.12};
    const rgb turquoise{0.25, 0.88, 0.82};
    const rgb orchid  {0.85, 0.44, 0.84};
    const rgb plum    {0.87, 0.63, 0.87};
    const rgb tan     {0.82, 0.71, 0.55};
    const rgb wheat   {0.96, 0.87, 0.70};
    const rgb invalid {-1.0, -1.0, -1.0, -1.0};

    const std::map<std::string, rgba> map = {
        {"transparent", color::transparent},
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
        {"navy"   , color::navy},
        {"orange" , color::orange},
        {"brown"  , color::brown},
        {"pink"   , color::pink},
        {"indigo" , color::indigo},
        {"violet" , color::violet},
        {"gold"   , color::gold},
        {"coral"  , color::coral},
        {"salmon" , color::salmon},
        {"crimson", color::crimson},
        {"skyblue", color::skyblue},
        {"lightblue", color::lightblue},
        {"darkblue" , color::darkblue},
        {"lightgreen", color::lightgreen},
        {"darkgreen" , color::darkgreen},
        {"khaki"  , color::khaki},
        {"beige"  , color::beige},
        {"chocolate", color::chocolate},
        {"turquoise", color::turquoise},
        {"orchid" , color::orchid},
        {"plum"   , color::plum},
        {"tan"    , color::tan},
        {"wheat"  , color::wheat}
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

    utils::logging::Logger log{"color_from_string"};
    log.warning("Unrecognized color string '{}', defaulting to white", str);
    return color::white;
}

} // namespace telemetry

#endif // COLOR_H

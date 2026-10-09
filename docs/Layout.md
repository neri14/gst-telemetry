# Layout

## Widget Types



### layout

Base widget, no parameters

#### Example
```xml
<layout>
...
</layout>
```


### container

Container widget, draw encapsulated widgets at position relative to x,y.

#### Example
```xml
<container x="1700" y="750">
...
</container>
```

#### Parameters
|      name      |  type     |  required  |  default  |              description           |
|----------------|-----------|------------|-----------|------------------------------------|
|  x             |  numeric  |  yes       |           |  container x coordinate            |
|  y             |  numeric  |  yes       |           |  container y coordinate            |
|  visible       |  boolean  |  no        |  true     |  show widgets children if true     |


### if

Conditonal widget, draw encapsulated widgets only if condition is met.

#### Example
```xml
<if condition="eval(round(video_time)%2)">
...
</if>
```

#### Parameters
|      name      |  type     |  required  |  default  |              description              |
|----------------|-----------|------------|-----------|---------------------------------------|
|  condition     |  boolean  |  yes       |           |  show widgets children if true        |



### text

*ToDo: to be described*



## composite-text

*ToDo: to be described*



### timestamp

*ToDo: to be described*



### rectangle

*ToDo: to be described*



### circle

Draw a circle with center at x,y.

#### Example
```xml
<circle x="750" y="400" radius="140" color="rgb(0, 0, 1)"
        border-width="5" border-color="black" visible="true" />
```

#### Notes
coordinates of widgets located under circle are relative to circle center

#### Parameters
|      name      |  type     |  required  |  default  |              description               |
|----------------|-----------|------------|-----------|----------------------------------------|
|  x             |  numeric  |  yes       |           |  circle center x coordinate            |
|  y             |  numeric  |  yes       |           |  circle center y coordinate            |
|  radius        |  numeric  |  yes       |           |  circle radius                         |
|  color         |  color    |  no        |  white    |  background color                      |
|  border-width  |  numeric  |  no        |  0        |  border width; 0 = no border           |
|  border-color  |  color    |  no        |  black    |  border color                          |
|  visible       |  boolean  |  no        |  true     |  show widget and its children if true  |



### line

*ToDo: to be described*



### chart

Draw a line chart of `y-value` against `x-value`, optionally with local-extreme
markers (a circle + formatted label at the highest/lowest y-value within a
configurable x-axis window).

**Note:** redrawing chart line is expensive, for static charts - avoid changing configuration.

**Note:** limitation: filters are meant for producing chart only - if point and filter are both configured - point will not be filtered out

**Note:** local-extreme detection only considers a point once it has a full
`marker-window/2` neighborhood on *both* sides within the currently displayed/
filtered data. On a live moving-window chart (e.g. a trailing-30s speed chart)
this means a genuine peak only becomes a marker once roughly `marker-window/2`
worth of newer, descending data has scrolled past it - this is intentional, to
avoid flagging a point that's simply still rising as a fake "local max".

#### Example
```xml
<chart x="0" y="4" width="900" height="300" line-color="white" line-width="4"
       x-value="key(video_time)" y-value="key(pchip_point_speed)"
       max-marker="true" marker-window="5" marker-format="{:.1f} km/h"
       max-marker-border-color="red" max-marker-label-position="top" />
```

#### Parameters
|      name      |  type     |  required  |  default      |              description               |
|----------------|-----------|------------|---------------|------------------------------------------|
|  x             |  numeric  |  yes       |               |  chart x coordinate                       |
|  y             |  numeric  |  yes       |               |  chart y coordinate                       |
|  width         |  numeric  |  yes       |               |  chart width                              |
|  height        |  numeric  |  yes       |               |  chart height                             |
|  line-color    |  color    |  no        |  white        |  line color                               |
|  line-width    |  numeric  |  no        |  2            |  line width; 0 = no line                  |
|  point-color   |  color    |  no        |  transparent  |  current-value point color                |
|  point-size    |  numeric  |  no        |  0            |  current-value point size; 0 = no point   |
|  point-border-color |  color |  no      |  transparent  |  current-value point border color         |
|  point-border-width |  numeric | no     |  0            |  current-value point border width         |
|  background-below |  color  |  no        |               |  fill color below the line (unset = no fill) |
|  x-value       |  numeric  |  yes       |               |  track key/expression for x values        |
|  y-value       |  numeric  |  yes       |               |  track key/expression for y values        |
|  value-time-step |  numeric | no        |               |  resample x/y values at a fixed time step instead of native trackpoint timestamps |
|  stretch-to-fill |  boolean | no        |  true         |  stretch line to fill width/height, ignoring aspect ratio |
|  min-x/max-x   |  numeric  |  no        |  auto         |  fixed x-axis bounds (both required together to lock)  |
|  min-y/max-y   |  numeric  |  no        |  auto         |  fixed y-axis bounds (both required together to lock)  |
|  visible       |  boolean  |  no        |  true         |  show widget and its children if true     |
|  filter-value/filter-min/filter-max | numeric | no |    |  restrict drawn x/y values to a moving window (see example above) |
|  zoom-to-filter-x/zoom-to-filter-y | boolean | no | false | auto-scale that axis to the filtered window only |
|  marker-window | numeric   |  see note  |               |  x-axis window width for local-extreme detection; **required** if `max-marker` or `min-marker` is set |
|  marker-format | string    |  no        |  `{:.1f}`     |  `std::vformat` format string applied to the marked y-value |
|  marker-font-name | string |  no        |  Arial        |  marker label font name                   |
|  marker-font-size | numeric | no        |  12           |  marker label font size                   |
|  marker-label-offset | numeric | no     |  4            |  gap between the marker ring and its label |
|  marker-label-border-width | numeric | no | 0           |  marker label outline width; 0 = no outline |
|  marker-label-border-color | color | no |  black        |  marker label outline color                |
|  max-marker    |  boolean  |  no        |  false        |  enable local-maximum markers              |
|  max-marker-color | color  |  no        |  transparent  |  local-maximum marker fill color           |
|  max-marker-radius | numeric | no       |  8            |  local-maximum marker radius               |
|  max-marker-border-width | numeric | no |  2            |  local-maximum marker ring width           |
|  max-marker-border-color | color | no   |  red          |  local-maximum marker ring color           |
|  max-marker-label-color | color | no    |  = border color | local-maximum label text color          |
|  max-marker-label-position | string | no | top          |  one of `top`/`bottom`/`left`/`right`/`top-left`/`top-right`/`bottom-left`/`bottom-right` |
|  min-marker    |  boolean  |  no        |  false        |  enable local-minimum markers              |
|  min-marker-color | color  |  no        |  transparent  |  local-minimum marker fill color           |
|  min-marker-radius | numeric | no       |  8            |  local-minimum marker radius               |
|  min-marker-border-width | numeric | no |  2            |  local-minimum marker ring width           |
|  min-marker-border-color | color | no   |  blue         |  local-minimum marker ring color           |
|  min-marker-label-color | color | no    |  = border color | local-minimum label text color          |
|  min-marker-label-position | string | no | bottom       |  one of `top`/`bottom`/`left`/`right`/`top-left`/`top-right`/`bottom-left`/`bottom-right` |


## Widget Parameter Types

### Numeric

|    value        |    examples               |                description                           |
|-----------------|---------------------------|------------------------------------------------------|
|  `key(...)`     |  `key(point_timer)`       |  value of track key in parenthesis defines value     |
|  `eval(...)`    |  `eval(point_speed*3.6)`  |  ExprTk expression inside parenthesis defines value  |
|  numeric value  |  `1`, `3.14`              |  static value to be used as is                       |

**Note:** ExprTk expressions are tested with simple arithmetic functionality only.


### String

*ToDo: to be described*


### Formatted

*ToDo: to be described*


### Timestamp

*ToDo: to be described*


### Color

|    value          |    examples                        |                description                                                              |
|-------------------|------------------------------------|-----------------------------------------------------------------------------------------|
|  `key(...)`       |  `key(x)`                          |  value of track key is interpreted as color (must be hex color or one of basic colors)  |
|  `rgb(r,g,b)`     |  `rgb(eval((x%15.0)/15.0), 0, 0)`  |  each r,g,b is interpreted like numeric parameter, clamped to <0.0, 1.0> values         |
|  `rgba(r,g,b,a)`  |  `rgba(1,1,1,1)`                 |  like `rgb(r,g,b)` with alpha                                                           |
|  `#RRGGBB`        |  `#ffffff`                         |  hex color code                                                                         |
|  `#RRGGBBAA`      |  `#000000f0`                       |  hex color code with alpha                                                              |
|  string value     |  `white`, `red`                    |  basic color name                                                                       |


### TextAlign

*ToDo: to be described*


### Boolean

*ToDo: to be described*



## GPX Key mapping

**Note** extension fields have namespace trimmed - the field name is preserved.

- metadata (data applicable to whole trk) has `meta_` prefix appended
- point related data (data applicable to single trkpt) has `point_` prefix appended
- point related data is available as a "latched value" - last received value is provided
- (to be implemented) linear interpolation of point fields values have `lerp_` appended before `point_`
- (to be implemented) piecewise cubic hermite interpolation of point fields values have `pchip_` appended before `point_`


|  GPX path                     |  layout key mapping           |  description                                                                          |
|-------------------------------|-------------------------------|---------------------------------------------------------------------------------------|
|  metadata/bounds.minlat       |  meta_minlat                  |  minimal latitude                                                                     |
|  metadata/bounds.maxlat       |  meta_maxlat                  |  maximal latitude                                                                     |
|  metadata/bounds.minlon       |  meta_minlon                  |  minimal longitude                                                                    |
|  metadata/bounds.maxlon       |  meta_maxlon                  |  maximal longitude                                                                    |
|  metadata/name                |  meta_name                    |  activity name (later name in gpx file is preserved)                                  |
|  trk/name                     |  meta_name                    |  activity name (later name in gpx file is preserved)                                  |
|  trk/src                      |  meta_src                     |  source of gpx file (e.g. device used to record data)                                 |
|  trk/type                     |  meta_typ                     |  type of activity (usually a sport identifier)                                        |
|                               |                               |                                                                                       |
|  *ToDo metadata extensions*   |                               |                                                                                       |
|                               |                               |                                                                                       |
|  *ToDo segments*              |                               |                                                                                       |
|                               |                               |                                                                                       |
|  *ToDo trackpoint fields*     |                               |                                                                                       |
|                               |                               |                                                                                       |
|                               |  timestamp                    |  virtual - timestamp available over whole video not only when track is active         |
|                               |  video_time                   |  virtual - video runtime in seconds counting from 0                                   |
|                               |  time_elapsed                 |  virtual - elapsed activity time in seconds (negative before start)                   |
|                               |  time_remaining               |  virtual - remaining time of activity in seconds (negative after end)                 |
|                               |  active                       |  virtual - boolean flag indicating if activity is ongoing                             |
|                               |  countdown                    |  virtual - time till activity starts in seconds, exists only before activity          |
|                               |  overtime                     |  virtual - time since activity finished in seconds, exists only after activity        |
|                               |                               |                                                                                       |

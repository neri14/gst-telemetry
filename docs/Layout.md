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

**Note:** redrawing chart line is expensive, for static charts - avoid changing configuration.

**Note:** limitation: filters are meant for producing chart only - if point and filter are both configured - point will not be filtered out

*ToDo: to be described*


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

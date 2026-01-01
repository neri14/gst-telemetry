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




## Widget Parameter Types

### Numeric

|    value        |    examples               |                description                           |
|-----------------|---------------------------|------------------------------------------------------|
|  `key(...)`     |  `key(point_timer)`       |  value of track key in parenthesis defines value     |
|  `eval(...)`    |  `eval(point_speed*3.6)`  |  ExprTk expression inside parenthesis defines value  |
|  numeric value  |  `1`, `3.14`              |  static value to be used as is                       |

**Note:** ExprTk expressions are tested with simple arithmetic functionality only.



### Color

|    value          |    examples                        |                description                                                              |
|-------------------|------------------------------------|-----------------------------------------------------------------------------------------|
|  `key(...)`       |  `key(x)`                          |  value of track key is interpreted as color (must be hex color or one of basic colors)  |
|  `rgb(r,g,b)`     |  `rgb(eval((x%15.0)/15.0), 0, 0)`  |  each r,g,b is interpreted like numeric parameter, clamped to <0.0, 1.0> values         |
|  `rgba(r,g,b,a)`  |  `rgba(1,1,1,1)`                 |  like `rgb(r,g,b)` with alpha                                                           |
|  `#RRGGBB`        |  `#ffffff`                         |  hex color code                                                                         |
|  `#RRGGBBAA`      |  `#000000f0`                       |  hex color code with alpha                                                              |
|  string value     |  `white`, `red`                    |  basic color name                                                                       |



### Boolean

todo



## GPX Key mapping

todo
# TODO

## 1.0
- images (load image, display at x,y, plus rotation and rotation pivot point - so e.g. gauge needle can be made, plus scale)

## 1.0 for release
- full documentation
  - widgets
  - parameters
  - gpx fields mapping
  - segments fields and usage examples


## 2.0 (further improvements)

- circle fill based on value (start angle end angle?)
- rotation of different types of widgets

- TBD how to handle events (like jumps) that are available in single track point
  - add latching data to widget?
  - or add special field types?
  - or...?

## 3.0 (future ideas)
- extended animation support
    - fadein / fadeout ? (theoretically could be achieved with dynamic color?)
- conditional chart colors (because composing colors from different widgets gets slow)
- maybe: segment cumulative fields (only accpower?)

## other projects interaction
- gpst: ascent to go (+ gst-telemetry to calculate ascent to go virtual?)
- minele maxele metadata - needed to scale additional data onto graphs with elevation

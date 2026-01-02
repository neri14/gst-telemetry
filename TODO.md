# TODO

## 0.1 (videographer feature parity)
- text with datetime handling improvements (second level precision, timezone config)
- charts, different flavors:
    - from beginning to end
    - from Xs ago till now (for moving charts)
    - from X timestamp to Y timestamp (for segments)

## 1.0 (target minimal)
- compound text widget (seprate widget?), with attributes like:
    - value-1="..." - e.g. calculate hours part from activity time
    - value-2="..." - e.g. calculate minutes part from activity time
    - value-3="..." - e.g. calculate seconds part from activity time
    - format="{:.0f}:{:02.0f}{:05.3f}" - gives time in "H:MM:SS.mmm"
  taking multiple values and e.g. for timestamp format: "{:.0f}:{:.0f}{:.3f}"
- segments handling

## 2.0 (further improvements)
- images (load image, display at x,y, plus rotation and rotation pivot point - so e.g. gauge needle can be made, plus scale)
- rectangles, plus rectangle fill based on value (and direction of fill?)
- circle fill based on value (and direction of fill?)

## 3.0 (future ideas)
- extended animation support
    - fadein / fadeout

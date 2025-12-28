# gst-telemetry


## building

```
meson setup builddir --werror --prefix /usr/
ninja -C builddir
```


## install

```
sudo ninja -C builddir install
```

## inspect

```
gst-inspect-1.0 telemetry
```

## background debug prints

```
export TELEMETRY_DEBUG=LVL
```

where LVL enables:
```
2 - DEBUG and higher severity
1 - INFO and higher severity
0 / other / not set - WARNING and higher severity

```

## gstreamer debug prints

```
export GST_DEBUG=LVL
```

where LVL enables prints up to:
```
0 - none
1 - ERROR
2 - WARNING
3 - FIXME
4 - INFO
5 - DEBUG
6 - LOG
7 - TRACE
9 - MEMDUMP
```

can also be controlled on per plugin basis e.g.:
```
export GST_DEBUG=2,telemetry:5
```

[see description in gstreamer documentation](https://gstreamer.freedesktop.org/documentation/tutorials/basic/debugging-tools.html?gi-language=c)


## gstreamer generate dot pipeline diagram

```
export GST_DEBUG_DUMP_DOT_DIR=./tmp
```

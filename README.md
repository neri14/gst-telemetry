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

## debug prints

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
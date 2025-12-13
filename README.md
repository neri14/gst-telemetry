# gst-telemetry


## building

```
cd project
meson setup builddir --werror --prefix /usr/
ninja -C builddir
```


## install

```
sudo ninja -C builddir install
```

## inspect

```
gst-inspect-1.0 telemetryoverlay
```

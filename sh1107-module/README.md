# SH1107 OLED Display Driver for Zephyr

Zephyr display driver for SH1107-based monochrome OLED displays (64x128, 128x128).

## Features

- I2C interface support
- Works with LVGL (LV_COLOR_DEPTH_1)
- Compatible with ZMK and Zephyr RTOS
- Includes shield for 64x128 displays

## Installation

### As ZMK Module

Add to your `config/west.yml`:

```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: Babajko
      url-base: https://github.com/Babajko/zmk-config
  projects:
    - name: zmk
      remote: zmkfirmware
      revision: main
      import: app/west.yml
    - name: sh1107-zephyr-driver
      remote: disp_drv_sh1107
      revision: main
  self:
    path: config
```

### Manual Integration

Copy `sh1107-module/` to your project and add to CMakeLists.txt:

```cmake
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/sh1107-module)
```

## Usage

### With Included Shield (64x128)

Add shield to your build:

```
west build -b nice_nano_v2 -- -DSHIELD="sh1107_64x128"
```

### Custom Configuration

Create device tree overlay:

```dts
&i2c0 {
    status = "okay";
    
    sh1107: sh1107@3c {
        compatible = "sinowealth,sh1107";
        reg = <0x3c>;
        width = <64>;
        height = <128>;
        display-offset = <96>;  /* Required for 64x128 */
        multiplex-ratio = <127>;
        segment-remap;
        com-invdir;
    };
};

/ {
    chosen {
        zephyr,display = &sh1107;
    };
};
```

## Device Tree Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `width` | int | required | Display width in pixels |
| `height` | int | required | Display height in pixels |
| `display-offset` | int | 0 | COM offset (96 for 64x128) |
| `segment-offset` | int | 0 | Column offset |
| `page-offset` | int | 0 | Page offset |
| `multiplex-ratio` | int | 127 | MUX ratio (height - 1) |
| `segment-remap` | bool | false | Flip horizontally |
| `com-invdir` | bool | false | Flip vertically |
| `contrast` | int | 0x80 | Initial contrast |
| `prechargep` | int | 0x22 | Pre-charge period |

## LVGL Configuration

Required Kconfig options:

```
CONFIG_LV_Z_VDB_SIZE=100
CONFIG_LV_Z_FULL_REFRESH=y
CONFIG_LV_Z_BITS_PER_PIXEL=1
CONFIG_LV_COLOR_DEPTH_1=y
```

## Module Structure

```
sh1107-module/
├── boards/shields/sh1107_64x128/  # Generic 64x128 shield
├── drivers/display/               # SH1107 driver
├── dts/bindings/display/          # Device tree bindings
├── examples/                      # Example code
└── zephyr/                        # Zephyr module config
```

## License

GPL

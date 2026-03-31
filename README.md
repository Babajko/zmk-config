# ZMK Lily58 Configuration

ZMK firmware configuration for Lily58 keyboard with CH1115 display.

## Docker Build (Recommended)

### Requirements
- Docker
- Docker Compose

### Quick Start

1. **Build firmware:**
   ```bash
   ./build.sh
   ```

2. **Firmware files** will be in `firmware/` directory:
   - `lily58_left-nice_nano-zmk.uf2` - left half
   - `lily58_right-nice_nano-zmk.uf2` - right half

### Alternative Commands

```bash
# Build (default)
./build.sh

# Full rebuild without cache
./build.sh rebuild

# Clean generated files
./build.sh clean

# Clean Docker cache
./build.sh clean-cache

# Open shell for debugging
./build.sh shell

# Show configuration
./build.sh info

# Show all commands
./build.sh help
```

See [docker/BUILD_SYSTEM.md](docker/BUILD_SYSTEM.md) for more details about the build system.

## Flashing the Keyboard

1. Connect keyboard half via USB
2. Double-tap the reset button
3. USB drive will appear
4. Copy the corresponding `.uf2` file to the drive
5. Keyboard will automatically reboot with new firmware

## Configuration

- **Display:** CH1115 OLED 128×64 (I2C address 0x3c)
- **Controller:** nice!nano v2
- **Keymap:** [config/lily58.keymap](config/lily58.keymap)
- **Shields:**
  - `lily58_left` / `lily58_right` - Lily58 keyboard layout
  - `oled_ch1115_128x64` - Hardware driver for CH1115 OLED display
  - `nice_oled` - UI widgets for display (battery, layers, status)

### Display Stack

The OLED display requires two shields working together:

1. **Hardware Layer** (`oled_ch1115_128x64`):
   - I2C communication with CH1115 controller
   - LVGL initialization
   - Display buffer management

2. **UI Layer** (`nice_oled`):
   - Battery status widget
   - Layer indicators
   - Connection status
   - WPM counter
   - Custom status screen

Both shields must be included in `build.yaml` for the display to work properly.

## GitHub Actions Build

Alternatively, you can use automatic builds via GitHub Actions:

1. Commit changes: `git add . && git commit -m "Update config"`
2. Push to GitHub: `git push`
3. Go to **Actions** tab in repository
4. Download firmware files from Artifacts

## Project Structure

```
├── config/              # Configuration files
│   ├── lily58.keymap   # Keyboard layout
│   ├── lily58.conf     # Settings
│   └── west.yml        # Dependencies
├── boards/             # Custom shield overlays
│   └── shields/
│       ├── nice_oled/           # Display UI widgets
│       └── oled_ch1115_128x64/  # CH1115 hardware driver
├── docker/             # Docker build system
│   ├── Dockerfile          # Docker image for building
│   ├── docker-compose.yml  # Docker Compose configuration
│   ├── BUILD_SYSTEM.md     # Build system documentation
│   └── scripts/            # Build scripts
│       ├── parse_build_yaml.py    # Build config parser
│       └── build_firmware.sh      # Firmware build script
├── build.yaml          # Build configuration
└── build.sh            # Build automation script
```

## Display Configuration

If your CH1115 display uses a different I2C address, edit:
- `boards/shields/oled_ch1115_128x64/oled_ch1115_128x64.overlay`
- Change `reg = <0x3c>;` to the required address

## Documentation

- [ZMK Documentation](https://zmk.dev/)
- [Lily58 Build Guide](https://github.com/kata0510/Lily58)

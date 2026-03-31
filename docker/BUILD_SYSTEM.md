# ZMK Firmware Build System

Automated ZMK firmware build system for Lily58 keyboard using Docker.

## Quick Start

```bash
# Build firmware
./build.sh

# Or explicitly
./build.sh build
```

Firmware files will be in `./firmware/` directory

## Available Commands

```bash
./build.sh help              # Show all commands
./build.sh build             # Build firmware (uses cache)
./build.sh rebuild           # Completely rebuild container
./build.sh clean             # Remove generated .uf2 files
./build.sh clean-cache       # Clean Docker build cache
./build.sh stop              # Stop Docker containers
./build.sh shell             # Open shell in container
./build.sh info              # Show build configuration
```

## How It Works

### 1. Build Configuration - `build.yaml`

The `build.yaml` file defines which boards and shields to build:

```yaml
include:
  - board: nice_nano_v2
    shield: lily58_left oled_ch1115_128x64
    snippet: zmk-usb-logging
  - board: nice_nano_v2
    shield: lily58_right
```

The system automatically reads this file and builds the corresponding firmware.

### 2. Caching

Docker uses multi-stage build with caching:

- **ZMK repository** is cached in Docker layer - downloaded only once
- **Build directory** is stored in Docker volume `zmk-build-cache`
- **Subsequent builds** execute **much faster** (only recompiling changes)

### 3. File Permissions

Files are created with your UID:GID, so **sudo is not required**.

## Project Structure

```
zmk-config/
├── build.yaml              # Build configuration
├── build.sh                # Build script with commands
├── Dockerfile             # Multi-stage Docker image
├── docker-compose.yml     # Docker Compose configuration
├── scripts/
│   ├── parse_build_yaml.py    # build.yaml parser
│   └── build_firmware.sh      # Build script
├── config/
│   ├── lily58.keymap     # Key layout
│   ├── lily58.conf       # ZMK configuration
│   └── west.yml          # West dependencies
├── boards/               # Custom boards/shields
└── firmware/             # Output .uf2 files (generated)
```

## Usage Examples

### First Build

```bash
# Builds Docker container and compiles firmware
./build.sh build
```

This takes ~10-15 minutes (downloading ZMK, dependencies, compilation).

### Subsequent Builds

```bash
# Fast - uses cache
./build.sh
```

Takes ~2-5 minutes (compilation only).

### Changing Keyboard Configuration

1. Edit `config/lily58.keymap` or `config/lily58.conf`
2. Run `./build.sh`
3. Firmware updates automatically

### Changing Board or Shield

1. Edit `build.yaml`
2. Run `./build.sh`
3. System automatically builds new configuration

### Cleanup

```bash
# Remove only generated .uf2 files
./build.sh clean

# Remove Docker cache (free up space)
./build.sh clean-cache
```

### Debug

```bash
# Open shell in container
./build.sh shell

# Inside container shell:
west build -p -s zmk/app -b nice_nano_v2 -- -DSHIELD="lily58_left"
```

## Flashing the Keyboard

1. Build firmware: `./build.sh`
2. Connect keyboard half via USB
3. Double-tap the RESET button
4. Half appears as USB drive
5. Copy the corresponding `.uf2` file to the drive
6. Keyboard automatically reboots with new firmware
7. Repeat for the other half

## Troubleshooting

### Docker volume full

```bash
./build.sh clean-cache
```

### Build error

```bash
# Rebuild container from scratch
./build.sh rebuild
```

### Check configuration

```bash
./build.sh info
```

### "Permission denied" during build

Check that UID/GID are exported:
```bash
echo $UID $GID
```

## Requirements

- Docker
- Docker Compose
- Python 3 (for local `./build.sh info`)
- Bash

## Technical Details

- **Base image**: `zmkfirmware/zmk-build-arm:stable`
- **Multi-stage build**: Caches ZMK separately from build configuration
- **Build cache volume**: Stores compiled objects between builds
- **Auto-parsing**: Automatically reads `build.yaml` for building
- **Smart caching**: ZMK downloaded once, subsequent builds are faster

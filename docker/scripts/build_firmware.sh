#!/bin/bash

set -e

# Disable Kconfig warnings as errors (workaround for ZMK warnings)
export KCONFIG_WERROR=n

echo "Parsing build.yaml configuration..."
/usr/local/bin/parse_build_yaml.py --bash > /tmp/build_commands.sh
chmod +x /tmp/build_commands.sh

echo ""
echo "Starting firmware build..."
/tmp/build_commands.sh

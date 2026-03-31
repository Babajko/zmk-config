# ZMK Firmware Build Environment - Multi-stage build
# Stage 1: ZMK base with caching
FROM zmkfirmware/zmk-build-arm:stable AS zmk-base

# Install additional utilities
RUN apt-get update && apt-get install -y \
    git \
    wget \
    curl \
    nano \
    python3-pip \
    python3-yaml \
    && rm -rf /var/lib/apt/lists/*

# Create working directory
WORKDIR /workspace

# Copy west.yml for initialization (cached separately)
COPY config/west.yml ./config/west.yml

# Initialize west workspace (cached if west.yml unchanged)
RUN west init -l config/ && \
    west update && \
    west zephyr-export

# Install Python dependencies (cached)
RUN python3 -m pip install --break-system-packages --no-cache-dir -r zephyr/scripts/requirements.txt

# Stage 2: Build environment
FROM zmk-base AS builder

# Copy project configuration
COPY config ./config
COPY build.yaml ./
COPY boards ./boards

# Copy script for parsing build.yaml
COPY docker/scripts/parse_build_yaml.py /usr/local/bin/
RUN chmod +x /usr/local/bin/parse_build_yaml.py

# Copy build script
COPY docker/scripts/build_firmware.sh /workspace/build.sh
RUN chmod +x /workspace/build.sh

CMD ["/workspace/build.sh"]

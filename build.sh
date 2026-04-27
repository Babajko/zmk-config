#!/bin/bash
# ZMK Firmware Build Script

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Export UID and GID for docker-compose
export BUILD_UID=$(id -u)
export BUILD_GID=$(id -g)

# Functions
show_help() {
    echo -e "${BLUE}ZMK Firmware Build System${NC}"
    echo ""
    echo "Usage: ./build.sh [command]"
    echo ""
    echo "Commands:"
    echo -e "  ${GREEN}build${NC}        Build firmware (default)"
    echo -e "  ${GREEN}rebuild${NC}      Rebuild container from scratch without cache"
    echo -e "  ${GREEN}clean${NC}        Clean generated .uf2 files"
    echo -e "  ${GREEN}clean-cache${NC}  Clean Docker build cache"
    echo -e "  ${GREEN}stop${NC}         Stop containers"
    echo -e "  ${GREEN}shell${NC}        Open shell in container"
    echo -e "  ${GREEN}info${NC}         Show build configuration"
    echo -e "  ${GREEN}help${NC}         Show this message"
}

build() {
    echo -e "${BLUE}Building ZMK firmware...${NC}"
    mkdir -p firmware
    docker compose -f docker/docker-compose.yml up --build
    
    echo ""
    echo -e "${GREEN}✓ Build complete!${NC}"
    if [ -n "$(ls -A firmware/ 2>/dev/null)" ]; then
        echo -e "Firmware files in ${BLUE}./firmware${NC}:"
        ls -lh firmware/
    fi
}

rebuild() {
    echo -e "${YELLOW}Rebuilding Docker container from scratch...${NC}"
    docker compose -f docker/docker-compose.yml down
    docker compose -f docker/docker-compose.yml build --no-cache
    echo -e "${GREEN}✓ Container rebuilt${NC}"
}

clean() {
    echo -e "${YELLOW}Cleaning firmware files...${NC}"
    rm -rf firmware/*.uf2
    echo -e "${GREEN}✓ Firmware files cleaned${NC}"
}

clean_cache() {
    echo -e "${YELLOW}Cleaning Docker build cache...${NC}"
    docker compose -f docker/docker-compose.yml down -v
    docker builder prune -f
    echo -e "${GREEN}✓ Build cache cleaned${NC}"
}

stop() {
    echo "Stopping containers..."
    docker compose -f docker/docker-compose.yml down
}

shell() {
    echo -e "${BLUE}Opening shell in builder container...${NC}"
    docker compose -f docker/docker-compose.yml run --rm zmk-builder /bin/bash
}

info() {
    echo -e "${BLUE}Build Configuration:${NC}"
    echo ""
    if [ -f docker/scripts/parse_build_yaml.py ]; then
        python3 docker/scripts/parse_build_yaml.py
    else
        echo "Error: parse_build_yaml.py not found"
    fi
    echo ""
    echo -e "${BLUE}Docker Volumes:${NC}"
    docker volume ls | grep zmk || echo "  No ZMK volumes found"
    echo ""
    echo -e "${BLUE}Current UID:GID:${NC} $(id -u):$(id -g)"
}

# Command handling
case "${1:-build}" in
    build)
        build
        ;;
    rebuild)
        rebuild
        ;;
    clean)
        clean
        ;;
    clean-cache)
        clean_cache
        ;;
    stop)
        stop
        ;;
    shell)
        shell
        ;;
    info)
        info
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        echo -e "${RED}Unknown command: $1${NC}"
        echo ""
        show_help
        exit 1
        ;;
esac

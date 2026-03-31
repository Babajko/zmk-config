#!/usr/bin/env python3

import yaml
import sys
import os

def parse_build_yaml(yaml_file='build.yaml'):
    try:
        with open(yaml_file, 'r') as f:
            data = yaml.safe_load(f)
        
        builds = []
        if 'include' in data:
            for item in data['include']:
                board = item.get('board', 'unknown')
                shield = item.get('shield', '')
                snippet = item.get('snippet', '')
                cmake_args = item.get('cmake-args', '')
                
                builds.append({
                    'board': board,
                    'shield': shield,
                    'snippet': snippet,
                    'cmake_args': cmake_args
                })
        
        return builds
    except Exception as e:
        print(f"Error parsing build.yaml: {e}", file=sys.stderr)
        sys.exit(1)

def generate_build_commands(builds, output_dir='/output'):
    commands = []
    
    for idx, build in enumerate(builds):
        board = build['board']
        shield = build['shield']
        snippet = build['snippet']
        cmake_args = build['cmake_args']
        
        shield_name = shield.split()[0] if shield else 'default'
        output_file = f"{output_dir}/{shield_name}-{board}-zmk.uf2"
        
        cmd_parts = [
            'west build -p -s zmk/app',
            f'-b {board}',
            '--'
        ]
        
        if shield:
            cmd_parts.append(f'-DSHIELD="{shield}"')
        
        cmd_parts.append('-DZMK_CONFIG="/workspace/config"')
        cmd_parts.append('-DBOARD_ROOT="/workspace"')
        
        if cmake_args:
            cmd_parts.append(cmake_args)
        
        build_cmd = ' \\\n  '.join(cmd_parts)
        
        commands.append({
            'name': f"{shield_name} ({board})",
            'build_cmd': build_cmd,
            'output_file': output_file,
            'snippet': snippet
        })
    
    return commands

if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1] == '--bash':
        builds = parse_build_yaml()
        commands = generate_build_commands(builds)
        
        print("#!/bin/bash")
        print("set -e\n")
        print("echo 'Building ZMK firmware from build.yaml configuration...'\n")
        
        for idx, cmd in enumerate(commands, 1):
            print(f"echo '===================================='")
            print(f"echo 'Building {cmd['name']}...'")
            print(f"echo '===================================='\n")
            print(cmd['build_cmd'])
            print()
            print(f"cp build/zephyr/zmk.uf2 {cmd['output_file']}")
            print(f"echo '✓ Built: {cmd['output_file']}'\n")
        
        print('chown -R ${BUILD_UID:-1000}:${BUILD_GID:-1000} /output/ 2>/dev/null || true')
        print()
        print('echo ""')
        print('echo "Build complete! Firmware files:"')
        print('ls -lh /output/')
    else:
        builds = parse_build_yaml()
        print("Build configurations:")
        for build in builds:
            print(f"  - Board: {build['board']}, Shield: {build['shield']}")

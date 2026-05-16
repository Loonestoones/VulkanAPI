#!/usr/bin/env python3
"""
Shader Compiler for Vulkan SPIR-V
Compiles GLSL shader files to SPIR-V bytecode using glslc
"""

import subprocess
import os
import sys
from pathlib import Path

# Configuration
VULKAN_SDK_PATH = r"C:\VulkanSDK\1.4.341.1"
GLSLC_PATH = os.path.join(VULKAN_SDK_PATH, "Bin", "glslc.exe")

# Shader directories
PROJECT_ROOT = os.path.dirname(os.path.abspath(__file__))
SHADER_DIR = os.path.join(PROJECT_ROOT, "Vulkan", "Tutorials", "tutorial09", "shader")
OUTPUT_DIR = SHADER_DIR  # Output .spv files in same directory

# Shaders to compile
SHADERS = [
    ("test.vert", "test.vert.spv"),
    ("test.frag", "test.frag.spv"),
]


def check_glslc_exists():
    """Check if glslc is available"""
    if not os.path.exists(GLSLC_PATH):
        print(f"Error: glslc not found at {GLSLC_PATH}")
        print("Make sure Vulkan SDK is installed at:", VULKAN_SDK_PATH)
        return False
    return True


def compile_shader(input_file, output_file):
    """Compile a single shader file"""
    input_path = os.path.join(SHADER_DIR, input_file)
    output_path = os.path.join(OUTPUT_DIR, output_file)

    # Check if input file exists
    if not os.path.exists(input_path):
        print(f"Error: Shader file not found: {input_path}")
        return False

    print(f"Compiling: {input_file} → {output_file}")

    try:
        # Run glslc compiler
        result = subprocess.run(
            [GLSLC_PATH, input_path, "-o", output_path],
            capture_output=True,
            text=True
        )

        if result.returncode != 0:
            print(f"Error compiling {input_file}:")
            print(result.stderr)
            return False

        print(f"✓ Successfully compiled: {output_file}")
        return True

    except Exception as e:
        print(f"Error: Failed to run glslc: {e}")
        return False


def main():
    """Main compilation function"""
    print("=" * 60)
    print("Vulkan Shader Compiler (GLSL → SPIR-V)")
    print("=" * 60)
    print()

    # Check if glslc exists
    if not check_glslc_exists():
        sys.exit(1)

    # Create output directory if it doesn't exist
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # Compile all shaders
    all_success = True
    for input_file, output_file in SHADERS:
        if not compile_shader(input_file, output_file):
            all_success = False

    print()
    print("=" * 60)
    if all_success:
        print("✓ All shaders compiled successfully!")
        print(f"Output directory: {OUTPUT_DIR}")
        sys.exit(0)
    else:
        print("✗ Some shaders failed to compile")
        sys.exit(1)


if __name__ == "__main__":
    main()


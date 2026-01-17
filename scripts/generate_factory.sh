#!/bin/bash
set -e

# Check if jq is available
if ! command -v jq &> /dev/null; then
  echo "Error: jq is required but not found in PATH"
  echo "Install it with: brew install jq"
  exit 1
fi

mkdir -p firmware

# Get platformio config to extract flash settings
config_json=$(pio project config --json-output)

for env in esp32-s3-supermini xiao_esp32c3 xiao_esp32s3 t_dongle_s3; do
  echo "Processing $env..."
  
  # Extract chip info from the compiled bootloader binary
  bootloader_info=$(esptool.py image-info .pio/build/${env}/bootloader.bin 2>/dev/null)
  
  if [[ -z "$bootloader_info" ]]; then
    echo "Error: Could not read bootloader.bin for environment $env"
    echo "Make sure to run 'pio run -e $env' first"
    exit 1
  fi
  
  # Extract chip type from bootloader (e.g., "ESP32-S3")
  chip_family=$(echo "$bootloader_info" | grep "Chip ID:" | sed -n 's/.*(\(ESP32[^)]*\)).*/\1/p')
  
  # Convert to lowercase for esptool chip parameter (ESP32-S3 -> esp32s3)
  chip=$(echo "$chip_family" | tr '[:upper:]' '[:lower:]' | tr -d '-')
  
  # Get flash settings from platformio.ini config (NOT from bootloader)
  # The bootloader uses board defaults, but we want the configured overrides
  env_config=$(echo "$config_json" | jq -r ".[] | select(.[0] == \"env:$env\") | .[1]")
  
  flash_size=$(echo "$env_config" | jq -r '.[] | select(.[0] == "board_build.flash_size") | .[1] // empty')
  if [[ -z "$flash_size" ]]; then
    flash_size=$(echo "$env_config" | jq -r '.[] | select(.[0] == "board_upload.flash_size") | .[1] // empty')
  fi
  
  flash_mode=$(echo "$env_config" | jq -r '.[] | select(.[0] == "board_build.flash_mode") | .[1] // empty')
  
  # Fall back to bootloader values if not in config
  if [[ -z "$flash_size" ]]; then
    flash_size=$(echo "$bootloader_info" | grep "Flash size:" | awk '{print $3}')
  fi
  
  if [[ -z "$flash_mode" ]]; then
    flash_mode=$(echo "$bootloader_info" | grep "Flash mode:" | awk '{print tolower($3)}')
  fi

  echo "Chip: $chip, Flash Size: $flash_size, Flash Mode: $flash_mode, Chip Family: $chip_family"
  
  # Validate that we got all required values
  if [[ -z "$chip" || -z "$chip_family" || -z "$flash_size" || -z "$flash_mode" ]]; then
    echo "Error: Could not extract all required information"
    echo "  chip=$chip, chip_family=$chip_family, flash_size=$flash_size, flash_mode=$flash_mode"
    exit 1
  fi
  
  # Merge binaries
  esptool --chip $chip merge-bin \
    --output firmware/${env}-factory.bin \
    0x0 .pio/build/${env}/bootloader.bin \
    0x8000 .pio/build/${env}/partitions.bin \
    0x10000 .pio/build/${env}/firmware.bin
  echo "Created firmware/${env}-factory.bin"
  
  # Generate individual manifest (skip for XIAO boards since they have combined manifest)
  if [[ $env != xiao_esp32s3 && $env != xiao_esp32c3 ]]; then
    cat > firmware/manifest_${env}.json << EOF
{
  "name": "Flock You - ${env}",
  "version": "1.0.0",
  "new_install_prompt_erase": true,
  "builds": [
    {
      "chipFamily": "${chip_family}",
      "parts": [
        {
          "path": "${env}-factory.bin",
          "offset": 0
        }
      ]
    }
  ]
}
EOF
    echo "Created firmware/manifest_${env}.json"
  fi
done

# Create combined XIAO manifest if both XIAO boards exist
if [[ -f "firmware/xiao_esp32s3-factory.bin" && -f "firmware/xiao_esp32c3-factory.bin" ]]; then
  cat > firmware/manifest_xiao.json << EOF
{
  "name": "Flock You - XIAO Boards",
  "version": "1.0.0",
  "new_install_prompt_erase": true,
  "builds": [
    {
      "chipFamily": "ESP32-S3",
      "parts": [
        {
          "path": "xiao_esp32s3-factory.bin",
          "offset": 0
        }
      ]
    },
    {
      "chipFamily": "ESP32-C3",
      "parts": [
        {
          "path": "xiao_esp32c3-factory.bin",
          "offset": 0
        }
      ]
    }
  ]
}
EOF
  echo "Created firmware/manifest_xiao.json (combined XIAO S3/C3)"
fi

echo "All factory binaries and manifests generated successfully!"

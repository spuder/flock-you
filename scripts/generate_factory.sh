#!/bin/bash
set -e

mkdir -p firmware

for env in esp32-s3-supermini xiao_esp32c3 xiao_esp32s3; do
  # Determine chip
  if [[ $env == *"esp32c3"* ]]; then
    chip="esp32c3"
    chip_family="ESP32-C3"
  else
    chip="esp32s3"
    chip_family="ESP32-S3"
  fi
  
  # Merge binaries
  esptool.py --chip $chip merge-bin \
    -o firmware/${env}-factory.bin \
    --flash-mode dio --flash-size 4MB \
    0x1000 .pio/build/${env}/bootloader.bin \
    0x8000 .pio/build/${env}/partitions.bin \
    0x10000 .pio/build/${env}/firmware.bin
  echo "Created firmware/${env}-factory.bin"
  
  # Generate manifest
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
done

echo "All factory binaries and manifests generated successfully!"

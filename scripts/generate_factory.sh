#!/bin/bash
set -e

mkdir -p firmware

for env in esp32-s3-supermini xiao_esp32c3 xiao_esp32s3 t_dongle_s3; do
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

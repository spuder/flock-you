.PHONY: all clean xiao_esp32s3 xiao_esp32c3 esp32-s3-supermini factory help

# Default target
all: build factory

# Build all environments
build:
	@echo "Building all environments..."
	platformio run -e esp32-s3-supermini -e xiao_esp32c3 -e xiao_esp32s3

# Generate factory binaries and manifests
factory:
	@echo "Generating factory binaries..."
	bash scripts/generate_factory.sh

# Clean all build artifacts
clean:
	@echo "Cleaning build artifacts..."
	platformio run -t clean
	rm -rf firmware/

# Individual board targets
xiao_esp32s3:
	@echo "Building xiao_esp32s3..."
	platformio run -e xiao_esp32s3

xiao_esp32c3:
	@echo "Building xiao_esp32c3..."
	platformio run -e xiao_esp32c3

esp32-s3-supermini:
	@echo "Building esp32-s3-supermini..."
	platformio run -e esp32-s3-supermini

# Help target
help:
	@echo "Available targets:"
	@echo "  make all              - Build all environments and generate factory binaries"
	@echo "  make build            - Build all environments"
	@echo "  make factory          - Generate factory binaries from existing builds"
	@echo "  make clean            - Clean all build artifacts"
	@echo "  make xiao_esp32s3     - Build only xiao_esp32s3"
	@echo "  make xiao_esp32c3     - Build only xiao_esp32c3"
	@echo "  make esp32-s3-supermini - Build only esp32-s3-supermini"

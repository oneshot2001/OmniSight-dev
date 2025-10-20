# OMNISIGHT Build System
# Makefile for ACAP Native SDK development

# Project configuration
PROJECT_NAME = omnisight
VERSION = 0.1.0
ARCH ?= aarch64

# Docker configuration
DOCKER_IMAGE = omnisight-dev
DOCKER_CONTAINER = omnisight-dev
ACAP_SDK_IMAGE = axisecp/acap-native-sdk:1.15-$(ARCH)-ubuntu22.04

# Camera configuration (can be overridden)
CAMERA_IP ?= 192.168.1.100
CAMERA_USER ?= root
CAMERA_PASS ?= pass

# Build directories
BUILD_DIR = build
SRC_DIR = src
MODELS_DIR = models
PACKAGE_DIR = package

# Compiler settings
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
STRIP = $(CROSS_COMPILE)strip

# ACAP package output
PACKAGE_NAME = $(PROJECT_NAME)_$(VERSION)_$(ARCH).eap

# Colors for output
GREEN = \033[0;32m
YELLOW = \033[0;33m
RED = \033[0;31m
NC = \033[0m # No Color

.PHONY: all help build clean install test docker-build docker-run dev package deploy

# Default target
all: help

## help: Display this help message
help:
	@echo "$(GREEN)OMNISIGHT Build System$(NC)"
	@echo "======================"
	@echo ""
	@echo "Available targets:"
	@echo ""
	@grep -E '^##' $(MAKEFILE_LIST) | sed 's/^## /  /' | column -t -s ':'
	@echo ""

## docker-build: Build the development Docker container
docker-build:
	@echo "$(GREEN)Building Docker development environment...$(NC)"
	docker build -t $(DOCKER_IMAGE):latest .
	@echo "$(GREEN)Docker image built successfully!$(NC)"

## docker-run: Run the development container
docker-run:
	@echo "$(GREEN)Starting development container...$(NC)"
	docker-compose up -d
	docker exec -it $(DOCKER_CONTAINER) /bin/bash

## dev: Start development environment (build + run)
dev: docker-build
	@echo "$(GREEN)Starting OMNISIGHT development environment...$(NC)"
	docker-compose up -d
	@echo "$(GREEN)Development environment ready!$(NC)"
	@echo "$(YELLOW)Run 'make shell' to enter the container$(NC)"

## shell: Open a shell in the development container
shell:
	@docker exec -it $(DOCKER_CONTAINER) /bin/bash

## build: Build the ACAP application
build:
	@echo "$(GREEN)Building OMNISIGHT application...$(NC)"
	@mkdir -p $(BUILD_DIR)
	docker run --rm \
		-v $(PWD):/opt/app \
		-w /opt/app \
		$(ACAP_SDK_IMAGE) \
		/bin/bash -c "cd $(BUILD_DIR) && cmake .. && make"
	@echo "$(GREEN)Build completed!$(NC)"

## package: Create ACAP package (.eap file)
package: build
	@echo "$(GREEN)Creating ACAP package...$(NC)"
	@mkdir -p $(PACKAGE_DIR)
	docker run --rm \
		-v $(PWD):/opt/app \
		-w /opt/app \
		$(ACAP_SDK_IMAGE) \
		/bin/bash -c "cd $(BUILD_DIR) && make package"
	@echo "$(GREEN)Package created: $(PACKAGE_DIR)/$(PACKAGE_NAME)$(NC)"

## install: Install ACAP package on camera
install: package
	@echo "$(GREEN)Installing OMNISIGHT on camera $(CAMERA_IP)...$(NC)"
	@curl -k --anyauth -u $(CAMERA_USER):$(CAMERA_PASS) \
		-F file=@$(PACKAGE_DIR)/$(PACKAGE_NAME) \
		https://$(CAMERA_IP)/axis-cgi/applications/upload.cgi
	@echo "$(GREEN)Installation complete!$(NC)"

## deploy: Build, package, and deploy to camera
deploy: package install
	@echo "$(GREEN)OMNISIGHT deployed successfully!$(NC)"

## clean: Remove build artifacts
clean:
	@echo "$(YELLOW)Cleaning build artifacts...$(NC)"
	rm -rf $(BUILD_DIR)
	rm -rf $(PACKAGE_DIR)
	@echo "$(GREEN)Clean complete!$(NC)"

## clean-all: Remove all generated files including Docker volumes
clean-all: clean
	@echo "$(YELLOW)Removing Docker containers and volumes...$(NC)"
	docker-compose down -v
	docker rmi $(DOCKER_IMAGE):latest 2>/dev/null || true
	@echo "$(GREEN)Deep clean complete!$(NC)"

## test: Run all tests
test:
	@echo "$(GREEN)Running OMNISIGHT tests...$(NC)"
	docker run --rm \
		-v $(PWD):/opt/app \
		-w /opt/app \
		$(DOCKER_IMAGE):latest \
		/bin/bash -c "cd tests && python3 -m pytest -v"

## test-perception: Run perception engine tests
test-perception:
	@echo "$(GREEN)Testing perception engine...$(NC)"
	docker run --rm \
		-v $(PWD):/opt/app \
		$(DOCKER_IMAGE):latest \
		/bin/bash -c "cd tests && python3 -m pytest -v test_perception.py"

## test-timeline: Run timeline threading tests
test-timeline:
	@echo "$(GREEN)Testing Timeline Threading™...$(NC)"
	docker run --rm \
		-v $(PWD):/opt/app \
		$(DOCKER_IMAGE):latest \
		/bin/bash -c "cd tests && python3 -m pytest -v test_timeline.py"

## test-swarm: Run swarm intelligence tests
test-swarm:
	@echo "$(GREEN)Testing Swarm Intelligence...$(NC)"
	docker run --rm \
		-v $(PWD):/opt/app \
		$(DOCKER_IMAGE):latest \
		/bin/bash -c "cd tests && python3 -m pytest -v test_swarm.py"

## test-camera: Run tests on actual camera
test-camera:
	@echo "$(GREEN)Running integration tests on camera $(CAMERA_IP)...$(NC)"
	@echo "$(YELLOW)Not yet implemented$(NC)"

## logs: View application logs from camera
logs:
	@echo "$(GREEN)Fetching logs from camera $(CAMERA_IP)...$(NC)"
	@curl -k --anyauth -u $(CAMERA_USER):$(CAMERA_PASS) \
		https://$(CAMERA_IP)/axis-cgi/systemlog.cgi?appname=$(PROJECT_NAME)

## status: Check application status on camera
status:
	@echo "$(GREEN)Checking OMNISIGHT status on $(CAMERA_IP)...$(NC)"
	@curl -k --anyauth -u $(CAMERA_USER):$(CAMERA_PASS) \
		https://$(CAMERA_IP)/axis-cgi/applications/list.cgi

## convert-model: Convert ML model to TFLite format
convert-model:
	@echo "$(GREEN)Converting model to TFLite...$(NC)"
	docker run --rm \
		-v $(PWD)/models:/models \
		$(DOCKER_IMAGE):latest \
		python3 /opt/app/scripts/convert_model.py

## format: Format source code
format:
	@echo "$(GREEN)Formatting source code...$(NC)"
	docker run --rm \
		-v $(PWD):/opt/app \
		$(DOCKER_IMAGE):latest \
		/bin/bash -c "find src -name '*.cpp' -o -name '*.h' | xargs clang-format -i"
	@echo "$(GREEN)Formatting complete!$(NC)"

## docs: Generate documentation
docs:
	@echo "$(GREEN)Generating documentation...$(NC)"
	@echo "$(YELLOW)Documentation generation not yet implemented$(NC)"

## info: Display build environment information
info:
	@echo "$(GREEN)OMNISIGHT Build Information$(NC)"
	@echo "============================"
	@echo "Project Name:    $(PROJECT_NAME)"
	@echo "Version:         $(VERSION)"
	@echo "Architecture:    $(ARCH)"
	@echo "Docker Image:    $(DOCKER_IMAGE)"
	@echo "ACAP SDK:        $(ACAP_SDK_IMAGE)"
	@echo "Target Camera:   $(CAMERA_IP)"
	@echo "Package Name:    $(PACKAGE_NAME)"
	@echo ""

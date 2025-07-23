#!/bin/bash

# Script to build and push the devcontainer Docker image to Docker Hub
# Usage: ./scripts/build-devcontainer.sh [DOCKER_USERNAME] [IMAGE_TAG]

set -e

# Default values
DEFAULT_DOCKER_USERNAME="your-github-username"
DEFAULT_IMAGE_TAG="heimdall-devcontainer:latest"

# Get parameters
DOCKER_USERNAME=${1:-$DEFAULT_DOCKER_USERNAME}
IMAGE_TAG=${2:-$DEFAULT_IMAGE_TAG}

# Full image name (using GitHub Container Registry)
FULL_IMAGE_NAME="ghcr.io/${DOCKER_USERNAME}/${IMAGE_TAG}"

echo "Building devcontainer image..."
echo "Docker Username: $DOCKER_USERNAME"
echo "Image Tag: $IMAGE_TAG"
echo "Full Image Name: $FULL_IMAGE_NAME"
echo ""

# Check if user is logged in to GitHub Container Registry
if ! docker info >/dev/null 2>&1; then
    echo "Error: Docker is not running or you're not logged in"
    echo "Please run: docker login ghcr.io"
    exit 1
fi

# Build the image
echo "Building image from .devcontainer/Dockerfile..."
docker build -t "$FULL_IMAGE_NAME" -f .devcontainer/Dockerfile .

echo ""
echo "Image built successfully!"
echo ""

# Ask user if they want to push to Docker Hub
read -p "Do you want to push this image to Docker Hub? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Pushing image to GitHub Container Registry..."
    docker push "$FULL_IMAGE_NAME"
    echo ""
    echo "Image pushed successfully!"
    echo ""
    echo "To use this image in your devcontainer.json, update it to:"
    echo "  \"image\": \"$FULL_IMAGE_NAME\""
    echo ""
    echo "Or add it as an option:"
    echo "  \"image\": \"$FULL_IMAGE_NAME\","
    echo "  \"build\": {"
    echo "    \"dockerfile\": \"./Dockerfile\","
    echo "    \"context\": \".\""
    echo "  }"
else
    echo "Image built but not pushed. You can push it later with:"
    echo "  docker push $FULL_IMAGE_NAME"
    echo ""
    echo "Note: Make sure you're logged in to GitHub Container Registry:"
    echo "  docker login ghcr.io"
fi

echo ""
echo "Done!" 
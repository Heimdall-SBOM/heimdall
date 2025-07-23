#!/bin/bash

# Script to build and push the devcontainer Docker image to GitHub Container Registry
# Usage: ./scripts/build-devcontainer.sh [GITHUB_USERNAME] [IMAGE_TAG]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show usage and help
show_usage() {
    echo "Heimdall DevContainer Build Script"
    echo "=================================="
    echo ""
    echo "Usage: $0 [OPTIONS] [GITHUB_USERNAME] [IMAGE_TAG]"
    echo ""
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  --dry-run               Show what would be done without executing"
    echo "  --no-push               Build only, don't push to registry"
    echo "  --force                 Skip confirmation prompts"
    echo "  --verbose               Show detailed output"
    echo ""
    echo "Arguments:"
    echo "  GITHUB_USERNAME         Your GitHub username (default: your-github-username)"
    echo "  IMAGE_TAG              Image tag (default: heimdall-devcontainer:latest)"
    echo ""
    echo "Examples:"
    echo "  $0                                    # Build with defaults"
    echo "  $0 tbakker                           # Build for user 'tbakker'"
    echo "  $0 tbakker my-dev:latest            # Build with custom tag"
    echo "  $0 --dry-run tbakker                # Show what would be built"
    echo "  $0 --no-push tbakker                # Build only, don't push"
    echo "  $0 --force tbakker                  # Skip confirmation prompts"
    echo ""
    echo "Prerequisites:"
    echo "  1. Docker installed and running"
    echo "  2. GitHub Personal Access Token with 'write:packages' permission"
    echo "  3. Logged in to GitHub Container Registry: docker login ghcr.io"
    echo ""
    echo "For detailed setup instructions, see: docs/github-container-registry-setup.md"
    echo ""
    echo "Exit Codes:"
    echo "  0  - Success"
    echo "  1  - General error"
    echo "  2  - Authentication error"
    echo "  3  - Build error"
    echo "  4  - Push error"
    echo ""
}

# Default values
DEFAULT_GITHUB_USERNAME="your-github-username"
DEFAULT_IMAGE_TAG="heimdall-devcontainer:latest"

# Parse command line arguments
DRY_RUN=false
NO_PUSH=false
FORCE=false
VERBOSE=false
GITHUB_USERNAME=""
IMAGE_TAG=""

# Parse options
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_usage
            exit 0
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --no-push)
            NO_PUSH=true
            shift
            ;;
        --force)
            FORCE=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        -*)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
        *)
            if [ -z "$GITHUB_USERNAME" ]; then
                GITHUB_USERNAME="$1"
            elif [ -z "$IMAGE_TAG" ]; then
                IMAGE_TAG="$1"
            else
                print_error "Too many arguments"
                show_usage
                exit 1
            fi
            shift
            ;;
    esac
done

# Set defaults if not provided
GITHUB_USERNAME=${GITHUB_USERNAME:-$DEFAULT_GITHUB_USERNAME}
IMAGE_TAG=${IMAGE_TAG:-$DEFAULT_IMAGE_TAG}

# Full image name (using GitHub Container Registry)
FULL_IMAGE_NAME="ghcr.io/${GITHUB_USERNAME}/${IMAGE_TAG}"

# Show configuration
print_status "Configuration:"
echo "  GitHub Username: $GITHUB_USERNAME"
echo "  Image Tag: $IMAGE_TAG"
echo "  Full Image Name: $FULL_IMAGE_NAME"
echo "  Dry Run: $DRY_RUN"
echo "  No Push: $NO_PUSH"
echo "  Force: $FORCE"
echo "  Verbose: $VERBOSE"
echo ""

# Validate GitHub username
if [ "$GITHUB_USERNAME" = "$DEFAULT_GITHUB_USERNAME" ]; then
    print_error "Please provide your GitHub username"
    echo "Usage: $0 [OPTIONS] GITHUB_USERNAME [IMAGE_TAG]"
    echo "Example: $0 tbakker"
    exit 1
fi

if [ "$DRY_RUN" = true ]; then
    print_status "DRY RUN MODE - No actual changes will be made"
    echo ""
    echo "Would execute:"
    echo "  docker build -t $FULL_IMAGE_NAME -f .devcontainer/Dockerfile ."
    if [ "$NO_PUSH" = false ]; then
        echo "  docker push $FULL_IMAGE_NAME"
    fi
    echo ""
    exit 0
fi

# Check if Docker is running
print_status "Checking Docker..."
if ! docker info >/dev/null 2>&1; then
    print_error "Docker is not running or not accessible"
    echo "Please start Docker and try again"
    exit 1
fi
print_success "Docker is running"

# Check if logged in to GitHub Container Registry
print_status "Checking GitHub Container Registry authentication..."
# Try to access the registry - if this fails, we're not authenticated
if ! docker search ghcr.io/hello-world >/dev/null 2>&1; then
    print_warning "Could not verify GitHub Container Registry access"
    echo ""
    echo "This might be due to:"
    echo "1. Not logged in to GitHub Container Registry"
    echo "2. Network connectivity issues"
    echo "3. Registry access restrictions"
    echo ""
    echo "To ensure you're logged in:"
    echo "1. Create a GitHub Personal Access Token:"
    echo "   - Go to GitHub.com → Settings → Developer settings → Personal access tokens"
    echo "   - Generate a new token with 'write:packages' permission"
    echo "2. Login to GitHub Container Registry:"
    echo "   docker login ghcr.io"
    echo "   - Username: your GitHub username"
    echo "   - Password: your GitHub Personal Access Token"
    echo ""
    echo "For detailed instructions, see: docs/github-container-registry-setup.md"
    echo ""
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        print_status "Aborted by user"
        exit 2
    fi
    print_warning "Continuing without authentication verification..."
else
    print_success "Authenticated with GitHub Container Registry"
fi

# Check if Dockerfile exists
print_status "Checking Dockerfile..."
if [ ! -f ".devcontainer/Dockerfile" ]; then
    print_error ".devcontainer/Dockerfile not found"
    echo "Make sure you're running this script from the project root directory"
    exit 3
fi
print_success "Dockerfile found"

# Build the image
print_status "Building image from .devcontainer/Dockerfile..."
if [ "$VERBOSE" = true ]; then
    docker build -t "$FULL_IMAGE_NAME" -f .devcontainer/Dockerfile .
else
    docker build -t "$FULL_IMAGE_NAME" -f .devcontainer/Dockerfile . >/dev/null 2>&1
fi

if [ $? -eq 0 ]; then
    print_success "Image built successfully!"
else
    print_error "Failed to build image"
    exit 3
fi

# Handle push
if [ "$NO_PUSH" = true ]; then
    print_status "Skipping push (--no-push flag used)"
    echo ""
    print_success "Image built successfully but not pushed"
    echo "You can push it later with:"
    echo "  docker push $FULL_IMAGE_NAME"
else
    # Ask user if they want to push (unless --force is used)
    if [ "$FORCE" = false ]; then
        echo ""
        read -p "Do you want to push this image to GitHub Container Registry? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            print_status "Skipping push (user declined)"
            echo ""
            print_success "Image built successfully but not pushed"
            echo "You can push it later with:"
            echo "  docker push $FULL_IMAGE_NAME"
            exit 0
        fi
    fi

    # Push the image
    print_status "Pushing image to GitHub Container Registry..."
    if [ "$VERBOSE" = true ]; then
        docker push "$FULL_IMAGE_NAME"
    else
        docker push "$FULL_IMAGE_NAME" >/dev/null 2>&1
    fi

    if [ $? -eq 0 ]; then
        print_success "Image pushed successfully!"
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
        print_error "Failed to push image to GitHub Container Registry"
        echo ""
        echo "This might be due to:"
        echo "1. Insufficient permissions (check your Personal Access Token)"
        echo "2. Network connectivity issues"
        echo "3. Image name conflicts"
        echo ""
        echo "You can try pushing manually:"
        echo "  docker push $FULL_IMAGE_NAME"
        exit 4
    fi
fi

echo ""
print_success "Done!" 
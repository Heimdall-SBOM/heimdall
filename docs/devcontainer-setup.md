# DevContainer Setup with Docker Hub

This guide explains how to build and use a pre-built devcontainer Docker image to avoid rebuilding the container every time you open a codespace.

## Quick Start

1. **Build and push the image to Docker Hub:**
   ```bash
   ./scripts/build-devcontainer.sh your-dockerhub-username heimdall-devcontainer:latest
   ```

2. **Update the devcontainer.json:**
   Replace `your-dockerhub-username` in `.devcontainer/devcontainer.json` with your actual Docker Hub username.

3. **Use the pre-built image:**
   The devcontainer will now use the pre-built image from Docker Hub instead of building locally.

## Detailed Steps

### Step 1: Create Docker Hub Account

1. Go to [Docker Hub](https://hub.docker.com) and create an account
2. Create a repository for your devcontainer image (optional, but recommended)

### Step 2: Login to Docker Hub

```bash
docker login
```

### Step 3: Build and Push the Image

```bash
# Build and push with default settings
./scripts/build-devcontainer.sh

# Or specify custom username and tag
./scripts/build-devcontainer.sh your-username heimdall-dev:latest
```

### Step 4: Update devcontainer.json

Edit `.devcontainer/devcontainer.json` and replace `your-dockerhub-username` with your actual Docker Hub username:

```json
{
    "image": "your-actual-username/heimdall-devcontainer:latest",
    "build": {
        "dockerfile": "./Dockerfile",
        "context": "."
    },
    // ... rest of configuration
}
```

### Step 5: Use the Pre-built Image

When you open the codespace, VS Code will:
1. First try to pull the pre-built image from Docker Hub
2. If the image doesn't exist or can't be pulled, it will fall back to building locally

## Benefits

- **Faster startup**: No need to rebuild the container every time
- **Consistent environment**: Same image used across all team members
- **Reduced resource usage**: Less CPU and memory usage during codespace creation
- **Better reliability**: Pre-built image is tested and known to work

## Configuration Options

### Using Different Image Tags

You can use different tags for different versions or configurations:

```bash
# Build with specific tag
./scripts/build-devcontainer.sh your-username heimdall-dev:v1.0

# Build with latest tag
./scripts/build-devcontainer.sh your-username heimdall-dev:latest
```

### Fallback Configuration

The `devcontainer.json` is configured to fall back to local build if the pre-built image is unavailable:

```json
{
    "image": "your-username/heimdall-devcontainer:latest",
    "build": {
        "dockerfile": "./Dockerfile",
        "context": "."
    }
}
```

This means:
- If the image exists on Docker Hub, it will be used
- If the image doesn't exist or can't be pulled, it will build locally
- You can always force a local build by commenting out the `"image"` line

### Team Usage

For team projects, you can:

1. **Use a shared Docker Hub organization:**
   ```json
   {
       "image": "your-org/heimdall-devcontainer:latest"
   }
   ```

2. **Use versioned tags for stability:**
   ```json
   {
       "image": "your-username/heimdall-devcontainer:v1.2.3"
   }
   ```

3. **Use different images for different branches:**
   ```json
   {
       "image": "your-username/heimdall-devcontainer:${env:GITHUB_REF_NAME:-main}"
   }
   ```

## Troubleshooting

### Image Not Found

If you get an error that the image doesn't exist:

1. Check that you've pushed the image to Docker Hub
2. Verify the image name and tag in `devcontainer.json`
3. Try pulling the image manually: `docker pull your-username/heimdall-devcontainer:latest`

### Build Fails

If the local build fails:

1. Check that Docker is running
2. Verify you have sufficient disk space
3. Check the Dockerfile for any issues
4. Try building manually: `docker build -t test-image -f .devcontainer/Dockerfile .`

### Permission Issues

If you get permission errors:

1. Make sure you're logged in to Docker Hub: `docker login`
2. Check that you have permission to push to the repository
3. Verify your Docker Hub username is correct

## Updating the Image

When you make changes to the Dockerfile:

1. **Build and push a new version:**
   ```bash
   ./scripts/build-devcontainer.sh your-username heimdall-devcontainer:v1.1
   ```

2. **Update devcontainer.json to use the new version:**
   ```json
   {
       "image": "your-username/heimdall-devcontainer:v1.1"
   }
   ```

3. **Or update the latest tag:**
   ```bash
   ./scripts/build-devcontainer.sh your-username heimdall-devcontainer:latest
   ```

## Best Practices

1. **Use semantic versioning** for image tags
2. **Keep the latest tag updated** for the most recent stable version
3. **Test the image locally** before pushing to Docker Hub
4. **Document any changes** to the Dockerfile
5. **Use organization repositories** for team projects
6. **Set up automated builds** for CI/CD integration

## Automation

You can automate the build and push process by:

1. **Adding it to your CI/CD pipeline**
2. **Creating a GitHub Action** to build on Dockerfile changes
3. **Setting up automated testing** of the devcontainer image

Example GitHub Action:

```yaml
name: Build DevContainer Image

on:
  push:
    paths:
      - '.devcontainer/Dockerfile'
  workflow_dispatch:

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build and push
        run: |
          docker build -t your-username/heimdall-devcontainer:latest -f .devcontainer/Dockerfile .
          docker push your-username/heimdall-devcontainer:latest
``` 
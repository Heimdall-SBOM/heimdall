# Quick DevContainer Setup

This guide will help you set up a pre-built devcontainer image to avoid rebuilding every time you open a codespace.

## Option 1: Manual Setup (Recommended for first time)

### 1. Create Docker Hub Account
- Go to [Docker Hub](https://hub.docker.com) and create an account
- Note your username

### 2. Login to Docker Hub
```bash
docker login
```

### 3. Build and Push the Image
```bash
# Replace 'your-username' with your actual Docker Hub username
./scripts/build-devcontainer.sh your-username heimdall-devcontainer:latest
```

### 4. Update devcontainer.json
Edit `.devcontainer/devcontainer.json` and replace `your-dockerhub-username` with your actual username:
```json
{
    "image": "your-actual-username/heimdall-devcontainer:latest",
    // ... rest of config
}
```

### 5. Test the Setup
- Close your codespace
- Reopen it - it should now use the pre-built image

## Option 2: Automated Setup with GitHub Actions

### 1. Set up GitHub Secrets
In your GitHub repository, go to Settings → Secrets and variables → Actions, and add:
- `DOCKER_USERNAME`: Your Docker Hub username
- `DOCKER_PASSWORD`: Your Docker Hub password or access token

### 2. Trigger the Workflow
- Go to Actions tab in your repository
- Select "Build DevContainer Image" workflow
- Click "Run workflow"
- Enter your Docker Hub username
- Click "Run workflow"

### 3. Update devcontainer.json
The workflow will automatically update the `devcontainer.json` file with your username.

## Benefits

✅ **Faster startup** - No more waiting for container builds  
✅ **Consistent environment** - Same image across all team members  
✅ **Reduced resource usage** - Less CPU/memory during codespace creation  
✅ **Better reliability** - Pre-tested and known working image  

## Troubleshooting

### Image Not Found
```bash
# Check if image exists
docker pull your-username/heimdall-devcontainer:latest

# If it doesn't exist, build and push it
./scripts/build-devcontainer.sh your-username heimdall-devcontainer:latest
```

### Fallback to Local Build
If the pre-built image fails to load, the devcontainer will automatically fall back to building locally. You can also force local build by commenting out the `"image"` line in `devcontainer.json`.

### Update the Image
When you make changes to `.devcontainer/Dockerfile`:
```bash
# Build new version
./scripts/build-devcontainer.sh your-username heimdall-devcontainer:v1.1

# Update devcontainer.json to use new version
# Or just update latest tag
./scripts/build-devcontainer.sh your-username heimdall-devcontainer:latest
```

## Team Usage

For team projects, consider:
- Using a shared Docker Hub organization
- Using versioned tags for stability
- Setting up automated builds on Dockerfile changes

See [docs/devcontainer-setup.md](docs/devcontainer-setup.md) for detailed documentation. 
# GitHub Container Registry Setup

This guide explains how to set up authentication for GitHub Container Registry (ghcr.io) to build and push devcontainer images.

## Prerequisites

- GitHub account
- Docker installed and running
- Access to the repository

## Step 1: Create a GitHub Personal Access Token

1. **Go to GitHub Settings**
   - Visit [GitHub.com](https://github.com)
   - Click your profile picture → Settings

2. **Navigate to Developer Settings**
   - Scroll down to "Developer settings" (bottom left)
   - Click "Personal access tokens"
   - Click "Tokens (classic)"

3. **Generate New Token**
   - Click "Generate new token (classic)"
   - Give it a descriptive name (e.g., "Heimdall DevContainer")

4. **Set Permissions**
   - **Expiration**: Choose an appropriate expiration (30 days recommended)
   - **Scopes**: Select the following:
     - ✅ `write:packages` - Upload packages to GitHub Package Registry
     - ✅ `read:packages` - Download packages from GitHub Package Registry
     - ✅ `delete:packages` - Delete packages from GitHub Package Registry

5. **Generate Token**
   - Click "Generate token"
   - **Important**: Copy the token immediately - you won't see it again!

## Step 2: Login to GitHub Container Registry

```bash
docker login ghcr.io
```

When prompted:
- **Username**: Your GitHub username
- **Password**: The Personal Access Token you just created

## Step 3: Build and Push the DevContainer Image

```bash
# Build and push the devcontainer image
./scripts/build-devcontainer.sh tbakker heimdall-devcontainer:latest
```

The script will:
1. Check if you're authenticated with ghcr.io
2. Build the devcontainer image
3. Ask if you want to push it
4. Push to `ghcr.io/tbakker/heimdall-devcontainer:latest`

## Step 4: Verify the Image

You can verify the image was pushed successfully:

```bash
# Pull the image to test
docker pull ghcr.io/tbakker/heimdall-devcontainer:latest

# Or check on GitHub
# Go to your GitHub profile → Packages → heimdall-devcontainer
```

## Troubleshooting

### Authentication Errors

**Error**: `requested access to the resource is denied`

**Solutions**:
1. **Check token permissions**: Ensure your token has `write:packages` permission
2. **Verify login**: Run `docker login ghcr.io` again
3. **Check token expiration**: Generate a new token if the current one expired

### Push Errors

**Error**: `denied: requested access to the resource is denied`

**Solutions**:
1. **Check repository permissions**: Ensure you have write access to the repository
2. **Verify image name**: The image name must match your GitHub username
3. **Check token scope**: Ensure the token has `write:packages` permission

### Build Errors

**Error**: `Dockerfile not found`

**Solutions**:
1. **Check working directory**: Run the script from the project root
2. **Verify file exists**: Ensure `.devcontainer/Dockerfile` exists

## Security Best Practices

1. **Token Expiration**: Set reasonable expiration dates (30-90 days)
2. **Minimal Permissions**: Only grant necessary permissions
3. **Regular Rotation**: Generate new tokens periodically
4. **Secure Storage**: Store tokens securely, never commit them to code

## Alternative: GitHub Actions

You can also use the GitHub Action workflow to automatically build and push the devcontainer image:

1. **Set up secrets** in your repository:
   - Go to Settings → Secrets and variables → Actions
   - Add `CR_PAT` with your Personal Access Token

2. **Trigger the workflow**:
   - Go to Actions → Build DevContainer Image
   - Click "Run workflow"
   - Enter your GitHub username
   - Click "Run workflow"

This will automatically build and push the image to GitHub Container Registry.

## Next Steps

Once the image is pushed successfully:

1. **Update devcontainer.json**: The image reference should already be correct
2. **Test the devcontainer**: Open a codespace to verify it works
3. **CI will work**: The GitHub Actions CI workflow will now use the pre-built image

## Support

If you encounter issues:

1. **Check the logs**: Look for specific error messages
2. **Verify permissions**: Ensure your token has the right permissions
3. **Test manually**: Try building and pushing manually to isolate issues
4. **Check GitHub status**: Ensure GitHub Container Registry is operational 
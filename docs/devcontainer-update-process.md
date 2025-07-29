# DevContainer Update Process

This document explains the complete process for updating the Heimdall development container after making changes to the `.devcontainer/Dockerfile`.

## Overview

When you modify the `.devcontainer/Dockerfile`, you need to rebuild the container image and update all CI workflows to use the new image. This process is automated but requires manual intervention for security reasons.

## Prerequisites

- Access to the Heimdall repository
- Docker installed locally (for testing)
- Understanding of the [Container Digest Updates](../.github/CONTAINER_DIGEST_UPDATES.md) process

## Complete Update Process

### Step 1: Make Your Dockerfile Changes

Edit the `.devcontainer/Dockerfile` with your desired changes:

```bash
# Edit the Dockerfile
vim .devcontainer/Dockerfile

# Example changes:
# - Add new development tools
# - Update package versions
# - Install additional dependencies
# - Modify build configurations
```

### Step 2: Build and Push the New Container Image

The new container image will be built automatically when you push your changes, or you can trigger it manually.

#### Option A: Automatic Trigger (Recommended)

```bash
# Commit and push your Dockerfile changes
git add .devcontainer/Dockerfile
git commit -m "feat: update devcontainer with new dependencies"
git push origin your-branch
```

This will automatically trigger the `build-devcontainer.yml` workflow.

#### Option B: Manual Trigger

1. Go to GitHub Actions in your repository
2. Find the "Build DevContainer Image" workflow
3. Click "Run workflow"
4. Fill in the parameters:
   - **Registry**: `ghcr.io` (default)
   - **Namespace**: `heimdall-sbom` (default)
   - **Image tag**: `latest` (default) or specify a version
5. Click "Run workflow"

### Step 3: Monitor the Build Process

The build workflow will:

1. ✅ **Build the new container image** from your updated Dockerfile
2. ✅ **Push the image** to the container registry
3. ✅ **Update digest tracking files** (`.github/container-digests.json`)
4. ✅ **Create patch files** for workflow updates
5. ❌ **Fail to push changes** (this is expected - security feature!)

#### Expected Build Output

```
🚀 DevContainer image built and pushed: ghcr.io/heimdall-sbom/heimdall-devcontainer:latest

✅ Security Updates:
- Container digest tracking updated automatically
- Security documentation updated with new digest
- Workflow update patch created for manual review

📋 Next Steps:
- Review the generated patch file: .github/workflow-digest-update.patch
- Apply the patch manually: patch -p1 < .github/workflow-digest-update.patch
- Test the updated workflows before committing

The devcontainer will now use the pre-built image instead of building locally.
```

#### Important: Permission Errors Are Expected

You may see errors like:
```
remote: Permission to Heimdall-SBOM/heimdall.git denied to github-actions[bot].
fatal: unable to access 'https://github.com/Heimdall-SBOM/heimdall/': The requested URL returned error: 403
```

**This is expected behavior** and indicates that the security measures are working correctly. The automated process cannot push changes directly to protect against unauthorized modifications.

**Note:** The build workflow has been updated to handle these permission errors gracefully, so the workflow will show as successful even when permission errors occur. This is the correct behavior.

### Step 4: Apply the Workflow Updates

After the build completes, you need to manually apply the workflow updates:

#### Option A: Use the Update Script

```bash
# Run the container digest update script
.github/scripts/update-container-digests.sh
```

This script will:
- Pull the latest container image
- Update digest tracking files
- Create patch files for manual workflow updates
- Provide clear instructions

#### Option B: Apply the Generated Patch

```bash
# Check if a patch file was created
ls -la .github/workflow-digest-update.patch

# If patch exists, apply it
patch -p1 < .github/workflow-digest-update.patch

# Review the changes
git diff
```

#### Option C: Manual Update

If the patch doesn't work, you can manually update the digest:

```bash
# Get the new digest from the build logs or tracking file
cat .github/container-digests.json

# Update all workflow files with the new digest
sed -i 's/OLD_DIGEST/NEW_DIGEST/g' .github/workflows/ci.yml
```

### Step 5: Test the Updated Workflows

Before committing, test that the workflows work with the new container:

```bash
# Run a local test build (if possible)
docker pull ghcr.io/heimdall-sbom/heimdall-devcontainer:latest
docker run --rm ghcr.io/heimdall-sbom/heimdall-devcontainer:latest echo "Container works!"

# Or trigger a test workflow run
# Go to GitHub Actions → CI workflow → Run workflow
```

### Step 6: Commit and Push the Updates

```bash
# Review all changes
git diff

# Add the updated files
git add .github/workflows/ci.yml .github/container-digests.json

# Commit with a descriptive message
git commit -m "security(deps): update container digest after Dockerfile changes

- Updated container digest to latest version
- Applied changes from .devcontainer/Dockerfile updates
- All CI workflows now use the new secure container image
- Digest: [NEW_DIGEST_HERE]"

# Push the changes
git push origin your-branch
```

## Complete Example Workflow

Here's a complete example of the entire process:

```bash
# 1. Make Dockerfile changes
vim .devcontainer/Dockerfile
# Add: RUN apt-get update && apt-get install -y new-tool

# 2. Commit and push (triggers build)
git add .devcontainer/Dockerfile
git commit -m "feat: add new development tool to devcontainer"
git push origin feature/new-tool

# 3. Wait for build to complete (check GitHub Actions)
# The build will fail with permission error (expected!)

# 4. Apply workflow updates
.github/scripts/update-container-digests.sh

# 5. Review and commit the updates
git diff
git add .github/workflows/ci.yml .github/container-digests.json
git commit -m "security(deps): update container digest after adding new tool"
git push origin feature/new-tool

# 6. Create pull request (if not already done)
# The CI workflows will now use the updated container
```

## Monitoring and Troubleshooting

### Checking Build Status

1. **GitHub Actions**: Monitor the "Build DevContainer Image" workflow
2. **Build Logs**: Check for successful image push and digest updates
3. **Patch Files**: Look for `.github/workflow-digest-update.patch` creation
4. **Digest Tracking**: Review `.github/container-digests.json` for updates

### Common Issues and Solutions

#### Build Fails with Permission Error

**Issue**: Build workflow fails with 403 permission error
**Solution**: This is expected behavior. The workflow cannot push changes directly for security reasons. Apply the updates manually using the patch file or update script.

#### Patch File Not Created

**Issue**: No `.github/workflow-digest-update.patch` file is generated
**Solution**: Run the update script manually:
```bash
.github/scripts/update-container-digests.sh
```

#### Patch Application Fails

**Issue**: `patch -p1 < .github/workflow-digest-update.patch` fails
**Solution**: Apply changes manually by updating the digest in all workflow files:
```bash
# Get the new digest
cat .github/container-digests.json | grep digest

# Update all occurrences
sed -i 's/OLD_DIGEST/NEW_DIGEST/g' .github/workflows/ci.yml
```

#### Workflow Tests Fail

**Issue**: CI workflows fail after updating the container digest
**Solution**: 
1. Check if the new container image is accessible
2. Verify that all dependencies are properly installed
3. Test the container locally if possible
4. Review the workflow logs for specific errors

### Verification Checklist

After completing the update process, verify:

- [ ] New container image is built and pushed successfully
- [ ] Digest tracking files are updated
- [ ] All CI workflows use the new container digest
- [ ] Workflow tests pass with the new container
- [ ] Security documentation is updated
- [ ] Changes are committed and pushed

## Security Considerations

### Why Manual Updates Are Required

The automated process is designed to require human review for workflow changes to:

1. **Prevent unauthorized modifications** to CI/CD pipelines
2. **Ensure security review** of container image changes
3. **Maintain audit trail** of all workflow modifications
4. **Comply with security best practices** for automated systems

### Container Image Security

- All container images are pinned to specific SHA256 digests
- This prevents supply chain attacks via mutable tags
- Digest updates are tracked and documented
- Security documentation is automatically updated

## Related Documentation

- [Container Digest Updates](../.github/CONTAINER_DIGEST_UPDATES.md) - Detailed explanation of the digest update system
- [Architecture Documentation](architecture.md) - Overall system architecture
- [Setup and Usage](setup_usage.md) - General setup instructions
- [Testing Guide](testing-heimdall.md) - Testing procedures

## Support

If you encounter issues with the devcontainer update process:

1. Check the [troubleshooting section](#common-issues-and-solutions) above
2. Review the [Container Digest Updates](../.github/CONTAINER_DIGEST_UPDATES.md) documentation
3. Check GitHub Actions logs for detailed error information
4. Create an issue in the repository with detailed error logs 
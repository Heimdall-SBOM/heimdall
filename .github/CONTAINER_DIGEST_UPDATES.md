# Container Digest Updates

This document explains how container image digests are managed for security in the Heimdall project.

## Overview

Container image digests are automatically updated to ensure supply chain security. The process has been designed to work within GitHub's security constraints while maintaining security best practices.

## How It Works

### Automated Updates

1. **Weekly Schedule**: The `update-container-digests.yml` workflow runs every Tuesday at 9 AM UTC
2. **Manual Trigger**: You can also trigger updates manually via the "Update Container Digests" workflow
3. **Safe Updates**: The workflow updates digest files without modifying workflow files directly

### Files Created/Updated

- `.github/container-digests.json` - Contains current container digests
- `.github/workflow-digest-update.patch` - Patch file for manual workflow updates (if needed)
- `.github/SECURITY_DEPENDENCIES.md` - Updated security documentation

## Manual Workflow Updates

When container digests change, the automated process creates a patch file that you can apply manually:

```bash
# Apply the patch to update workflow files
patch -p1 < .github/workflow-digest-update.patch

# Review the changes
git diff

# Test the updated workflow
# ... run your tests ...

# Commit and push
git add .
git commit -m "security(deps): update container digests"
git push
```

## Security Benefits

1. **Pinned Digests**: Container images are pinned to specific SHA256 digests
2. **Supply Chain Security**: Prevents supply chain attacks via mutable tags
3. **Audit Trail**: All digest updates are tracked and documented
4. **Manual Review**: Workflow changes require human review

## Troubleshooting

### Permission Errors

If you see permission errors like:
```
refusing to allow a GitHub App to create or update workflow `.github/workflows/ci.yml` without `workflows` permission
```

This is expected behavior. The automated process is designed to work within these constraints by:
- Creating digest files instead of directly modifying workflows
- Providing patch files for manual application
- Requiring human review for workflow changes

### Manual Digest Updates

To manually update container digests:

```bash
# Run the update script
.github/scripts/update-container-digests.sh

# Follow the instructions provided by the script
```

## Configuration

The container digest update process is configured in:
- `.github/workflows/update-container-digests.yml` - Automated workflow
- `.github/scripts/update-container-digests.sh` - Update script
- `.github/dependabot.yml` - Dependabot configuration (excludes workflow files)

## Monitoring

- Check the "Update Container Digests" workflow runs in GitHub Actions
- Review pull requests created by the automated process
- Monitor the `.github/container-digests.json` file for changes
- Watch for patch files that need manual application 
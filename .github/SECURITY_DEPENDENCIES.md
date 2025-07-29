# Supply Chain Security - Pinned Dependencies

This document tracks all external dependencies pinned to specific commit SHAs or digests for supply chain security.

## GitHub Actions (Pinned to Commit SHAs)

All GitHub Actions are pinned to specific commit SHAs to prevent supply chain attacks. These should be updated periodically or managed with automated tools like Dependabot/Renovate.

### Main CI Workflow (`ci.yml`)
- `actions/checkout@692973e3d937129bcbf40652eb9f2f61becf3332` # v4.1.7

### Documentation Workflow (`build_docs.yml`)
- `actions/checkout@692973e3d937129bcbf40652eb9f2f61becf3332` # v4.1.7  
- `actions/setup-node@1e60f620b9541d16bece96c5465dc8ee9832be0b` # v4.0.3
- `actions/configure-pages@983d7736d9b0ae728b81ab479565c72886d7745b` # v5.0.0  
- `actions/upload-pages-artifact@56afc609e74202658d3ffba0e8f6dda462b719fa` # v3.0.1
- `actions/deploy-pages@d6db90164ac5ed86f2b6aed7e0febac5b3c0c03e` # v4.0.5

### Reviewdog Workflow (`reviewdog.yml`)
- `actions/checkout@f43a0e5ff2bd294095638e18286ca9a3d1956744` # v3.6.0
- `actions/setup-python@65d7f2d534ac1bc67fcd62888c5f4f3d2cb2b236` # v4.7.1

### DevContainer Build Workflow (`build-devcontainer.yml`)
- `actions/checkout@692973e3d937129bcbf40652eb9f2f61becf3332` # v4.1.7
- `docker/setup-buildx-action@988b5a0280414f521da01fcc63a27aeeb4b104db` # v3.6.1
- `docker/login-action@9780b0c442fbb1117ed29e0efdff1e18412f7567` # v3.3.0
- `docker/build-push-action@16ebe778df0e7752d2cfcbd924afdbbd89c1a755` # v6.6.1
- `actions/github-script@60a0d83039c74a4aee543508d2ffcb1c3799cdea` # v7.0.1

## External Scripts (Pinned to Commit SHAs)

### Reviewdog Installer
- **URL**: `https://raw.githubusercontent.com/reviewdog/reviewdog/fd59714416d6d9a1c0692d872e38e7f8448df4fc/install.sh`
- **Reason**: Reviewdog was compromised in March 2025. This commit is post-incident and verified safe.
- **Used in**: `reviewdog.yml`

### LLVM clang-tidy Script  
- **URL**: `https://raw.githubusercontent.com/llvm/llvm-project/6009708b4367171ccdbf4b5905cb6a803753fe18/clang-tools-extra/clang-tidy/tool/run-clang-tidy.py`
- **Reason**: Pinned to stable LLVM commit instead of main branch
- **Used in**: `reviewdog.yml`

## Container Images (Pin to SHA256 Digests)

### Development Container
- **Current**: `ghcr.io/heimdall-sbom/heimdall-devcontainer:latest` (TODO: Pin to digest)
- **Security**: Pin to specific SHA256 digest instead of mutable tags
- **Update script**: `.github/scripts/update-container-digests.sh` (automated helper)
- **Manual command**: `docker inspect ghcr.io/heimdall-sbom/heimdall-devcontainer:latest --format='{{index .RepoDigests 0}}'`
- **Used in**: `ci.yml` (4 job definitions)

## Maintenance Guidelines

### Automated Dependency Management
This repository uses **Dependabot** for automated security updates:

#### ✅ **Dependabot Configuration** (`.github/dependabot.yml`)
- **GitHub Actions**: Weekly updates (Mondays) - monitors for new commit SHAs
- **Docker Images**: Weekly updates (Tuesdays) - monitors base images and containers
- **NPM Dependencies**: Weekly updates (Wednesdays) - documentation build dependencies  
- **Python Dependencies**: Weekly updates (Thursdays) - script requirements
- **Git Submodules**: Monthly updates (First Monday) - external/json, external/json-schema-validator
- **Security-focused**: Prioritizes security updates with proper labeling and reviews

#### 📋 **Manual Update Tools**
- **Container Digest Script**: `.github/scripts/update-container-digests.sh`
- **Renovate**: Alternative comprehensive dependency management
- **StepSecurity Secure Workflow**: Additional automated security hardening

### Manual Update Process
1. Check for new releases/security updates quarterly
2. Verify commit SHAs are still valid and secure
3. Test workflows with new SHAs in development branch
4. Update this documentation with new SHAs and versions

### Security Monitoring
- Monitor security advisories for all pinned dependencies
- Subscribe to security notifications for GitHub Actions
- Watch for supply chain attack reports affecting our dependencies

## Emergency Response

If a dependency is compromised:
1. Immediately update to a known-safe commit SHA
2. Review recent workflow runs for potential compromise
3. Rotate any secrets that may have been exposed
4. Update this documentation with new safe commit SHA

## Automation Scripts

### Container Digest Update Script
```bash
# Update all container images to latest digests
./.github/scripts/update-container-digests.sh

# This script will:
# 1. Pull the latest container image
# 2. Extract the current SHA256 digest  
# 3. Update CI workflow files with pinned digest
# 4. Update security documentation
# 5. Show changes made for review
```

### Automatic Container Digest Updates
The `build-devcontainer.yml` workflow automatically runs the digest update script after successful container builds.

**Required Permissions** (already configured):
- `contents: write` - Push commits with digest updates
- `actions: write` - Update workflow files  
- `packages: write` - Push to container registry
- `pull-requests: write` - Comment on PRs

**Workflow Integration**: Container digest updates are fully automated when the devcontainer image is updated, ensuring CI always uses the latest secure digest.

## Verification Commands

```bash
# Verify GitHub Action commit exists and is legitimate
git ls-remote https://github.com/actions/checkout.git 692973e3d937129bcbf40652eb9f2f61becf3332

# Verify external script integrity (example)
curl -s https://raw.githubusercontent.com/reviewdog/reviewdog/fd59714416d6d9a1c0692d872e38e7f8448df4fc/install.sh | sha256sum

# Get container image digest
docker inspect ghcr.io/heimdall-sbom/heimdall-devcontainer:latest --format='{{index .RepoDigests 0}}'

# Verify Dependabot configuration
gh api repos/OWNER/REPO/dependabot/alerts --paginate
```

---
**Last Updated**: July 2025  
**Next Review**: October 2025
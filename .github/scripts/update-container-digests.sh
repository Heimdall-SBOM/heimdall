#!/bin/bash
# Helper script to update container image digests for security
# Usage: ./update-container-digests.sh

set -euo pipefail

echo "🔒 Updating container image digests for supply chain security..."

# Configuration
CONTAINER_IMAGE="ghcr.io/heimdall-sbom/heimdall-devcontainer:latest"
CI_WORKFLOW_FILE=".github/workflows/ci.yml"
DIGEST_FILE=".github/container-digests.json"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}📦 Pulling latest container image...${NC}"
if ! docker pull "$CONTAINER_IMAGE" 2>/dev/null; then
    echo -e "${RED}❌ Failed to pull container image. Make sure Docker is running and you have access.${NC}"
    exit 1
fi

echo -e "${BLUE}🔍 Getting current digest...${NC}"
CURRENT_DIGEST=$(docker inspect "$CONTAINER_IMAGE" --format='{{index .RepoDigests 0}}' 2>/dev/null || echo "")

if [ -z "$CURRENT_DIGEST" ]; then
    echo -e "${RED}❌ Failed to get container digest${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Current digest: $CURRENT_DIGEST${NC}"

# Extract just the digest part after @
DIGEST_ONLY=$(echo "$CURRENT_DIGEST" | cut -d'@' -f2)
IMAGE_WITH_DIGEST="ghcr.io/heimdall-sbom/heimdall-devcontainer@$DIGEST_ONLY"

echo -e "${BLUE}📝 Creating digest file...${NC}"

# Create or update the digest file
cat > "$DIGEST_FILE" << EOF
{
  "container_digests": {
    "heimdall-devcontainer": {
      "image": "ghcr.io/heimdall-sbom/heimdall-devcontainer",
      "digest": "$DIGEST_ONLY",
      "full_image": "$IMAGE_WITH_DIGEST",
      "updated_at": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
      "updated_by": "update-container-digests.sh"
    }
  }
}
EOF

echo -e "${GREEN}✅ Created digest file: $DIGEST_FILE${NC}"

# Check if workflow file needs updating
echo -e "${BLUE}🔍 Checking if workflow file needs updating...${NC}"
if [ -f "$CI_WORKFLOW_FILE" ]; then
    CURRENT_WORKFLOW_DIGEST=$(grep -o 'ghcr\.io/heimdall-sbom/heimdall-devcontainer@sha256:[a-f0-9]*' "$CI_WORKFLOW_FILE" | head -1 | cut -d'@' -f2 || echo "")
    
    if [ "$CURRENT_WORKFLOW_DIGEST" != "$DIGEST_ONLY" ]; then
        echo -e "${YELLOW}⚠️  Workflow file has different digest: $CURRENT_WORKFLOW_DIGEST${NC}"
        echo -e "${YELLOW}⚠️  New digest available: $DIGEST_ONLY${NC}"
        echo -e "${BLUE}📋 Manual update required for workflow file${NC}"
        
        # Create a patch file for manual application
        PATCH_FILE=".github/workflow-digest-update.patch"
        cat > "$PATCH_FILE" << EOF
# Manual workflow digest update patch
# Apply this patch to update the container digest in .github/workflows/ci.yml
# 
# To apply: patch -p1 < $PATCH_FILE
# 
# Generated on: $(date)
# New digest: $DIGEST_ONLY

--- a/.github/workflows/ci.yml
+++ b/.github/workflows/ci.yml
EOF
        
        # Generate the actual patch content
        sed -n '/image: ghcr\.io\/heimdall-sbom\/heimdall-devcontainer:/p' "$CI_WORKFLOW_FILE" | while read -r line; do
            echo "---" >> "$PATCH_FILE"
            echo "-$line" >> "$PATCH_FILE"
            NEW_LINE=$(echo "$line" | sed "s|ghcr\.io/heimdall-sbom/heimdall-devcontainer:latest.*|ghcr.io/heimdall-sbom/heimdall-devcontainer@$DIGEST_ONLY  # Pinned $(date +%Y-%m-%d)|")
            echo "+$NEW_LINE" >> "$PATCH_FILE"
        done
        
        echo -e "${GREEN}✅ Created patch file: $PATCH_FILE${NC}"
    else
        echo -e "${GREEN}✅ Workflow file already has the latest digest${NC}"
    fi
else
    echo -e "${YELLOW}⚠️  Workflow file not found: $CI_WORKFLOW_FILE${NC}"
fi

# Update security documentation
SECURITY_DOC=".github/SECURITY_DEPENDENCIES.md"
if [ -f "$SECURITY_DOC" ]; then
    echo -e "${BLUE}📄 Updating security documentation...${NC}"
    
    # Update the container images section
    if grep -q "ghcr.io/heimdall-sbom/heimdall-devcontainer" "$SECURITY_DOC"; then
        sed -i.tmp "s|ghcr.io/heimdall-sbom/heimdall-devcontainer:latest|$IMAGE_WITH_DIGEST|g" "$SECURITY_DOC"
        sed -i.tmp "s|TODO.*Pin to specific SHA256 digest|✅ Pinned to digest (updated $(date +%Y-%m-%d))|g" "$SECURITY_DOC"
        rm "$SECURITY_DOC.tmp" 2>/dev/null || true
        echo -e "${GREEN}✅ Updated security documentation${NC}"
    fi
fi

echo -e "${GREEN}🎉 Container digest update complete!${NC}"
echo -e "${YELLOW}📝 Next steps:${NC}"
echo "   1. Review the changes: git diff"
echo "   2. If workflow file needs updating, apply the patch: patch -p1 < .github/workflow-digest-update.patch"
echo "   3. Test the updated workflow"
echo "   4. Commit the changes: git add . && git commit -m 'security(deps): update container digests'"
echo "   5. Push the changes: git push"

echo -e "${BLUE}ℹ️  Image details:${NC}"
echo "   Original: $CONTAINER_IMAGE"
echo "   Pinned:   $IMAGE_WITH_DIGEST"
echo "   Digest:   $DIGEST_ONLY"

# Verify the digest file was created
if [ -f "$DIGEST_FILE" ] && grep -q "$DIGEST_ONLY" "$DIGEST_FILE"; then
    echo -e "${GREEN}✅ Verification: Digest file created successfully${NC}"
else
    echo -e "${RED}❌ Verification failed: Digest file not created properly${NC}"
    exit 1
fi
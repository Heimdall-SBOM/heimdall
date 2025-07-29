#!/bin/bash
# Helper script to update container image digests for security
# Usage: ./update-container-digests.sh

set -euo pipefail

echo "🔒 Updating container image digests for supply chain security..."

# Configuration
CONTAINER_IMAGE="ghcr.io/heimdall-sbom/heimdall-devcontainer:latest"
CI_WORKFLOW_FILE=".github/workflows/ci.yml"

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

echo -e "${BLUE}🔄 Updating CI workflow file...${NC}"

# Check if the file exists
if [ ! -f "$CI_WORKFLOW_FILE" ]; then
    echo -e "${RED}❌ CI workflow file not found: $CI_WORKFLOW_FILE${NC}"
    exit 1
fi

# Create backup
cp "$CI_WORKFLOW_FILE" "$CI_WORKFLOW_FILE.backup"

# Update the workflow file
sed -i.tmp "s|image: ghcr.io/heimdall-sbom/heimdall-devcontainer:latest.*|image: $IMAGE_WITH_DIGEST  # Pinned $(date +%Y-%m-%d)|g" "$CI_WORKFLOW_FILE"
rm "$CI_WORKFLOW_FILE.tmp" 2>/dev/null || true

echo -e "${GREEN}✅ Updated $CI_WORKFLOW_FILE with new digest${NC}"

# Show the diff
echo -e "${YELLOW}📋 Changes made:${NC}"
diff "$CI_WORKFLOW_FILE.backup" "$CI_WORKFLOW_FILE" || true

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

# Clean up backup
rm "$CI_WORKFLOW_FILE.backup" 2>/dev/null || true

echo -e "${GREEN}🎉 Container digest update complete!${NC}"
echo -e "${YELLOW}📝 Next steps:${NC}"
echo "   1. Review the changes: git diff"
echo "   2. Test the updated workflow"
echo "   3. Commit the changes: git add . && git commit -m 'security(deps): pin container to digest'"
echo "   4. Push the changes: git push"

echo -e "${BLUE}ℹ️  Image details:${NC}"
echo "   Original: $CONTAINER_IMAGE"
echo "   Pinned:   $IMAGE_WITH_DIGEST"
echo "   Digest:   $DIGEST_ONLY"

# Verify the update worked
if grep -q "$DIGEST_ONLY" "$CI_WORKFLOW_FILE"; then
    echo -e "${GREEN}✅ Verification: Digest successfully updated in workflow file${NC}"
else
    echo -e "${RED}❌ Verification failed: Digest not found in workflow file${NC}"
    exit 1
fi
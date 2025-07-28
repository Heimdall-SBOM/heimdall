# Setting Up GitHub Container Registry for Your Organization

This guide explains how to set up GitHub Container Registry (GHCR) for your organization instead of using personal accounts.

## 🏢 **Why Use Organization GHCR?**

**Benefits:**
- **Team Access**: Multiple team members can publish packages
- **Centralized Management**: Organization admins control access
- **Better Security**: Organization-level permissions and audit logs
- **Professional Branding**: Packages appear under your organization name
- **Cost Efficiency**: Shared storage and bandwidth limits

## 📋 **Prerequisites**

1. **Organization Admin Access**: You need admin permissions in your GitHub organization
2. **GitHub Packages Enabled**: Ensure GitHub Packages is enabled for your organization
3. **Personal Access Token**: With appropriate permissions

## 🔧 **Step 1: Organization Setup**

### **1.1 Enable GitHub Packages**

1. Go to your organization: `https://github.com/organizations/Heimdall-SBOM`
2. Navigate to **Settings** → **Packages** → **GitHub Packages**
3. Ensure **GitHub Packages** is enabled
4. Configure **Inherit access from source repository** (recommended)

### **1.2 Configure Package Permissions**

1. In **Organization Settings** → **Packages** → **GitHub Packages**
2. Set **Package creation and deletion**:
   - **Allow members to create packages**: ✅ Enabled
   - **Allow members to delete packages**: ✅ Enabled (or restrict to admins)
3. Set **Package visibility**:
   - **Public packages**: ✅ Enabled (for open source projects)
   - **Private packages**: ✅ Enabled (if needed)

### **1.3 Repository Access Control**

1. Go to **Organization Settings** → **Packages** → **Package creation and deletion**
2. Configure which repositories can publish packages:
   - **All repositories**: Allow all repos to create packages
   - **Selected repositories**: Choose specific repos (recommended)
   - **None**: Disable package creation

## 🔑 **Step 2: Personal Access Token Setup**

### **2.1 Create Organization Token**

1. Go to **GitHub Settings** → **Developer settings** → **Personal access tokens** → **Tokens (classic)**
2. Click **Generate new token (classic)**
3. Set **Note**: `Heimdall-SBOM Organization GHCR`
4. Set **Expiration**: Choose appropriate duration (90 days recommended)
5. Select **Scopes**:
   - ✅ `write:packages` - Publish packages to organization
   - ✅ `read:packages` - Read packages from organization
   - ✅ `delete:packages` - Delete packages (optional)
   - ✅ `repo` - Repository access
   - ✅ `write:org` - Organization write access (if needed)

### **2.2 Token Security**

- **Store securely**: Use environment variables or GitHub Secrets
- **Rotate regularly**: Set expiration and renew before expiry
- **Limit scope**: Only grant necessary permissions
- **Monitor usage**: Check token usage in GitHub settings

## 🐳 **Step 3: Docker Authentication**

### **3.1 Login to GHCR**

```bash
# Login with your personal access token
docker login ghcr.io -u YOUR_GITHUB_USERNAME -p YOUR_PERSONAL_ACCESS_TOKEN
```

### **3.2 Verify Authentication**

```bash
# Test access to organization packages
docker search ghcr.io/heimdall-sbom/
```

## 📦 **Step 4: Build and Push Organization Package**

### **4.1 Build the DevContainer Image**

```bash
# Build for organization (using updated script)
./scripts/build-devcontainer.sh --force heimdall-sbom heimdall-devcontainer:latest
```

### **4.2 Manual Build (if needed)**

```bash
# Build image
docker build -t ghcr.io/heimdall-sbom/heimdall-devcontainer:latest -f .devcontainer/Dockerfile .

# Push to organization registry
docker push ghcr.io/heimdall-sbom/heimdall-devcontainer:latest
```

## 🔄 **Step 5: Update CI Configuration**

The CI workflow has been updated to use the organization package:

```yaml
container:
  image: ghcr.io/heimdall-sbom/heimdall-devcontainer:latest
  options: --user root
```

## 👥 **Step 6: Team Member Setup**

### **6.1 For Team Members**

Each team member needs:

1. **Organization Access**: Member or admin role in the organization
2. **Personal Access Token**: With `write:packages` scope
3. **Docker Login**: Authenticate with GHCR

```bash
# Team member login
docker login ghcr.io -u TEAM_MEMBER_USERNAME -p THEIR_PERSONAL_ACCESS_TOKEN
```

### **6.2 For CI/CD**

Use GitHub Secrets for CI authentication:

1. **Organization Secret**: `GHCR_TOKEN` with organization PAT
2. **Repository Secret**: `GHCR_USERNAME` with organization name

## 🔒 **Step 7: Security Best Practices**

### **7.1 Access Control**

- **Principle of Least Privilege**: Grant minimum necessary permissions
- **Regular Audits**: Review package access and permissions
- **Token Rotation**: Rotate PATs regularly
- **Monitor Usage**: Track package downloads and uploads

### **7.2 Package Security**

- **Scan Images**: Use security scanning tools
- **Vulnerability Monitoring**: Monitor for known vulnerabilities
- **Access Logs**: Review who accesses packages
- **Backup Strategy**: Consider backing up important packages

## 📊 **Step 8: Monitoring and Management**

### **8.1 Package Management**

1. **View Packages**: `https://github.com/orgs/Heimdall-SBOM/packages`
2. **Usage Analytics**: Monitor download statistics
3. **Storage Usage**: Track organization storage consumption
4. **Access Logs**: Review who accessed packages

### **8.2 Organization Settings**

1. **Billing**: Monitor package storage costs
2. **Rate Limits**: Check API rate limits
3. **Security Alerts**: Monitor for security issues
4. **Compliance**: Ensure compliance with policies

## 🚀 **Step 9: Automation**

### **9.1 GitHub Actions Integration**

The CI workflow automatically uses the organization package:

```yaml
# .github/workflows/ci.yml
jobs:
  build:
    container:
      image: ghcr.io/heimdall-sbom/heimdall-devcontainer:latest
```

### **9.2 Automated Builds**

Set up automated package builds:

```yaml
# .github/workflows/build-package.yml
name: Build DevContainer Package
on:
  push:
    paths: ['.devcontainer/**']
  schedule:
    - cron: '0 0 * * 0'  # Weekly rebuild

jobs:
  build-package:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build and Push Package
        run: |
          docker build -t ghcr.io/heimdall-sbom/heimdall-devcontainer:latest -f .devcontainer/Dockerfile .
          docker push ghcr.io/heimdall-sbom/heimdall-devcontainer:latest
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
```

## 🔧 **Troubleshooting**

### **Common Issues**

1. **Permission Denied**:
   - Check organization package permissions
   - Verify PAT has correct scopes
   - Ensure user has organization access

2. **Package Not Found**:
   - Verify package name and organization
   - Check package visibility settings
   - Ensure authentication is correct

3. **Rate Limits**:
   - Monitor API usage
   - Implement caching strategies
   - Contact GitHub support if needed

### **Useful Commands**

```bash
# Check authentication
docker login ghcr.io

# List organization packages
docker search ghcr.io/heimdall-sbom/

# Pull package
docker pull ghcr.io/heimdall-sbom/heimdall-devcontainer:latest

# Check package details
docker inspect ghcr.io/heimdall-sbom/heimdall-devcontainer:latest
```

## 📚 **Additional Resources**

- [GitHub Packages Documentation](https://docs.github.com/en/packages)
- [Container Registry Best Practices](https://docs.github.com/en/packages/guides/using-github-packages-with-github-actions)
- [Organization Management](https://docs.github.com/en/organizations)
- [Security Best Practices](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure)

## 🎯 **Next Steps**

1. **Set up organization permissions** as described above
2. **Create and configure your PAT** with appropriate scopes
3. **Build and push the organization package** using the updated script
4. **Test the CI workflow** to ensure it uses the organization package
5. **Share setup instructions** with your team members

This setup provides a professional, secure, and scalable container registry solution for your organization! 
# SPDX 3.0.x Schema and SBOM Limitations

## Overview
This document summarizes the current limitations, challenges, and realities of working with SPDX 3.0.x (3.0.0 and 3.0.1) schemas, JSON(-LD) output, and SBOM (Software Bill of Materials) support. It is based on practical experience, official documentation, and community knowledge as of mid-2025.

---

## 1. Official Schema vs. Real SBOM Needs
- The official SPDX 3.0.x JSON(-LD) schemas are generated from the full SPDX ontology, not from the SBOM profile.
- These schemas are **not SBOM-profile aware** and only allow a minimal set of fields for `SpdxDocument` and related classes.
- Most fields needed for a real, useful SBOM (e.g., `files`, `packages`, `relationships`, `creationInfo`, etc.) are **not allowed** by the official schema due to `unevaluatedProperties: false`.

---

## 2. JSON-LD vs. Classic JSON
- **JSON-LD**: Uses `@context` and `@graph`, with RDF-style property names (e.g., `@id`, `type`, `spdx:` prefixes).
- **Classic JSON**: Flat object, no `@context`/`@graph`, property names like `spdxId`, `type`, `specVersion`, etc.
- The official schema is primarily designed for JSON-LD, but some tools and users expect classic JSON.
- There is no single, canonical schema for classic JSON SBOMs in SPDX 3.x.

---

## 3. What Fields Are Allowed by the Official Schema?
- Only a minimal set, such as:
  - `spdxId`
  - `type`
  - Sometimes `dataLicense`, `import`, `namespaceMap`
- **Most SBOM fields are NOT allowed** (e.g., `name`, `documentNamespace`, `creationInfo`, `files`, `packages`, `relationships`, `profileIdentifier`, `primaryPurpose`, `specVersion`).

---

## 4. What Is Missing for Real SBOMs?
- Real SBOMs require:
  - Document name, namespace, version, creation info
  - List of files, packages, and their metadata (hashes, licenses, etc.)
  - Relationships between elements
  - Profile and purpose identifiers
  - Tooling and provenance information
- **All of these are forbidden by the official schema unless you patch it.**

---

## 5. State of Third-Party Validation
- Third-party validators (e.g., `spdx3-validate`, online tools) use the official, unmodified schema.
- They will **reject any SBOM** that includes fields not allowed by the schema.
- Patching the schema locally will not affect third-party validators.
- There is currently no widely-recognized, community-maintained SBOM-specific schema for SPDX 3.0.1.

---

## 6. Workarounds and Recommendations
- **For internal/CI use:** Patch the schema to allow all SBOM fields (e.g., set `unevaluatedProperties: true` for `SpdxDocument`).
- **For public/third-party validation:** Emit only the minimal fields allowed by the official schema (not useful for real SBOMs).
- **For real SBOMs:** Wait for the SPDX working group to publish a profile-specific SBOM schema, or participate in the community to help drive this effort.
- **Emit both formats** (classic and JSON-LD) if you need to support both types of validators.

---

## 7. References and Resources
- [SPDX 3 Model GitHub](https://github.com/spdx/spdx-3-model)
- [SPDX 3.0.1 Model and Serializations](https://spdx.github.io/spdx-spec/v3.0.1/serializations/)
- [SPDX 3.0.1 JSON-LD context](https://spdx.org/rdf/3.0.1/spdx-context.jsonld)
- [SPDX 3.0.1 JSON Schema](https://spdx.org/schema/3.0.1/spdx-json-schema.json)
- [SPDX Tech Mailing List](https://lists.spdx.org/g/Spdx-tech)

---

## 8. Guidance for Users and Developers
- **If you need strict third-party validation:** Only emit the minimal fields allowed by the official schema.
- **If you need real SBOMs:** Patch the schema for your own use, but be aware this will not help with third-party validation.
- **Monitor the SPDX project** for updates on SBOM profile schemas and participate in the community to help shape the future of SPDX validation.

---

*This document will be updated as the SPDX ecosystem evolves and more robust SBOM profile schemas become available.* 
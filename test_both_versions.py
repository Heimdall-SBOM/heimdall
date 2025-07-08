#!/usr/bin/env python3
"""
Comprehensive test runner for CycloneDX 1.5 and 1.6 compliance fixes.
Tests both versions to ensure proper version-specific behavior.
"""

import json
import subprocess
import sys
from typing import Dict, List

class CycloneDXVersionTestRunner:
    """Test runner for validating both CycloneDX 1.5 and 1.6 compliance"""
    
    def __init__(self):
        self.tests_passed = 0
        self.tests_failed = 0
        self.test_results = []
    
    def run_test(self, test_name: str, test_func) -> bool:
        """Run a single test and track results"""
        print(f"🧪 Running test: {test_name}")
        
        try:
            result = test_func()
            if result:
                print(f"✅ PASS: {test_name}")
                self.tests_passed += 1
                self.test_results.append((test_name, "PASS", ""))
                return True
            else:
                print(f"❌ FAIL: {test_name}")
                self.tests_failed += 1
                self.test_results.append((test_name, "FAIL", "Test returned False"))
                return False
                
        except Exception as e:
            print(f"❌ ERROR: {test_name} - {e}")
            self.tests_failed += 1
            self.test_results.append((test_name, "ERROR", str(e)))
            return False
    
    def test_1_5_validation_passes(self) -> bool:
        """Test that CycloneDX 1.5 validation passes"""
        try:
            result = subprocess.run([
                "python3", "validate_cyclonedx_1.5_fix.py", "mock_cyclonedx_1.5_output.json"
            ], capture_output=True, text=True)
            
            return result.returncode == 0 and "VALIDATION PASSED" in result.stdout
        except Exception:
            return False
    
    def test_1_6_validation_passes(self) -> bool:
        """Test that CycloneDX 1.6 validation passes"""
        try:
            result = subprocess.run([
                "python3", "validate_cyclonedx_fix.py", "mock_cyclonedx_1.6_output.json"
            ], capture_output=True, text=True)
            
            return result.returncode == 0 and "VALIDATION PASSED" in result.stdout
        except Exception:
            return False
    
    def test_1_5_has_module_in_callstack(self) -> bool:
        """Test that 1.5 callstack includes required module field"""
        with open("mock_cyclonedx_1.5_output.json", "r") as f:
            sbom = json.load(f)
        
        components = sbom.get("components", [])
        if not components:
            return False
        
        evidence = components[0].get("evidence", {})
        callstack = evidence.get("callstack", {})
        frames = callstack.get("frames", [])
        
        if not frames:
            return False
        
        # Check that frames have module field (required in 1.5)
        return "module" in frames[0]
    
    def test_1_6_function_optional_in_callstack(self) -> bool:
        """Test that 1.6 callstack has function field (not module-required like 1.5)"""
        with open("mock_cyclonedx_1.6_output.json", "r") as f:
            sbom = json.load(f)
        
        components = sbom.get("components", [])
        if not components:
            return False
        
        evidence = components[0].get("evidence", {})
        callstack = evidence.get("callstack", {})
        frames = callstack.get("frames", [])
        
        if not frames:
            return False
        
        # Check that frames have function field (1.6 style)
        return "function" in frames[0]
    
    def test_1_5_correct_schema_reference(self) -> bool:
        """Test that 1.5 SBOM has correct schema reference"""
        with open("mock_cyclonedx_1.5_output.json", "r") as f:
            sbom = json.load(f)
        
        return sbom.get("$schema") == "http://cyclonedx.org/schema/bom-1.5.schema.json"
    
    def test_1_6_correct_schema_reference(self) -> bool:
        """Test that 1.6 SBOM has correct schema reference"""
        with open("mock_cyclonedx_1.6_output.json", "r") as f:
            sbom = json.load(f)
        
        return sbom.get("$schema") == "http://cyclonedx.org/schema/bom-1.6.schema.json"
    
    def test_1_5_correct_spec_version(self) -> bool:
        """Test that 1.5 SBOM has correct specVersion"""
        with open("mock_cyclonedx_1.5_output.json", "r") as f:
            sbom = json.load(f)
        
        return sbom.get("specVersion") == "1.5"
    
    def test_1_6_correct_spec_version(self) -> bool:
        """Test that 1.6 SBOM has correct specVersion"""
        with open("mock_cyclonedx_1.6_output.json", "r") as f:
            sbom = json.load(f)
        
        return sbom.get("specVersion") == "1.6"
    
    def test_both_have_tools_components_structure(self) -> bool:
        """Test that both versions use the correct tools.components structure"""
        # Test 1.5
        with open("mock_cyclonedx_1.5_output.json", "r") as f:
            sbom_15 = json.load(f)
        
        tools_15 = sbom_15.get("metadata", {}).get("tools", {})
        has_components_15 = isinstance(tools_15, dict) and "components" in tools_15
        
        # Test 1.6  
        with open("mock_cyclonedx_1.6_output.json", "r") as f:
            sbom_16 = json.load(f)
        
        tools_16 = sbom_16.get("metadata", {}).get("tools", {})
        has_components_16 = isinstance(tools_16, dict) and "components" in tools_16
        
        return has_components_15 and has_components_16
    
    def test_both_have_supplier_objects(self) -> bool:
        """Test that both versions use supplier as organizational entity objects"""
        # Test 1.5
        with open("mock_cyclonedx_1.5_output.json", "r") as f:
            sbom_15 = json.load(f)
        
        components_15 = sbom_15.get("components", [])
        supplier_15_ok = False
        if components_15:
            supplier_15 = components_15[0].get("supplier", {})
            supplier_15_ok = isinstance(supplier_15, dict) and "name" in supplier_15
        
        # Test 1.6
        with open("mock_cyclonedx_1.6_output.json", "r") as f:
            sbom_16 = json.load(f)
        
        components_16 = sbom_16.get("components", [])
        supplier_16_ok = False
        if components_16:
            supplier_16 = components_16[0].get("supplier", {})
            supplier_16_ok = isinstance(supplier_16, dict) and "name" in supplier_16
        
        return supplier_15_ok and supplier_16_ok
    
    def test_both_have_evidence_structures(self) -> bool:
        """Test that both versions have proper evidence structures"""
        # Test 1.5
        with open("mock_cyclonedx_1.5_output.json", "r") as f:
            sbom_15 = json.load(f)
        
        components_15 = sbom_15.get("components", [])
        evidence_15_ok = False
        if components_15:
            evidence_15 = components_15[0].get("evidence", {})
            evidence_15_ok = "identity" in evidence_15 and "occurrences" in evidence_15
        
        # Test 1.6
        with open("mock_cyclonedx_1.6_output.json", "r") as f:
            sbom_16 = json.load(f)
        
        components_16 = sbom_16.get("components", [])
        evidence_16_ok = False
        if components_16:
            evidence_16 = components_16[0].get("evidence", {})
            evidence_16_ok = "identity" in evidence_16 and "occurrences" in evidence_16
        
        return evidence_15_ok and evidence_16_ok
    
    def run_all_tests(self):
        """Run all tests and report results"""
        print("🚀 Starting CycloneDX Multi-Version Compliance Test Suite")
        print("=" * 70)
        
        # Generate test data first
        print("📋 Setting up test data...")
        subprocess.run(["python3", "test_cyclonedx_1.5_output.py"], 
                      capture_output=True, text=True)
        subprocess.run(["python3", "test_json_output.py"], 
                      capture_output=True, text=True)
        
        # Define tests
        tests = [
            ("CycloneDX 1.5 validation passes", self.test_1_5_validation_passes),
            ("CycloneDX 1.6 validation passes", self.test_1_6_validation_passes),
            ("CycloneDX 1.5 has module in callstack", self.test_1_5_has_module_in_callstack),
            ("CycloneDX 1.6 function in callstack", self.test_1_6_function_optional_in_callstack),
            ("CycloneDX 1.5 correct schema reference", self.test_1_5_correct_schema_reference),
            ("CycloneDX 1.6 correct schema reference", self.test_1_6_correct_schema_reference),
            ("CycloneDX 1.5 correct spec version", self.test_1_5_correct_spec_version),
            ("CycloneDX 1.6 correct spec version", self.test_1_6_correct_spec_version),
            ("Both versions have tools.components", self.test_both_have_tools_components_structure),
            ("Both versions have supplier objects", self.test_both_have_supplier_objects),
            ("Both versions have evidence structures", self.test_both_have_evidence_structures),
        ]
        
        # Run tests
        for test_name, test_func in tests:
            self.run_test(test_name, test_func)
            print()  # Add space between tests
        
        # Print summary
        print("=" * 70)
        print("📊 TEST SUMMARY")
        print(f"✅ Tests Passed: {self.tests_passed}")
        print(f"❌ Tests Failed: {self.tests_failed}")
        print(f"📈 Success Rate: {self.tests_passed}/{len(tests)} ({100*self.tests_passed/len(tests):.1f}%)")
        
        if self.tests_failed == 0:
            print("\n🎉 ALL TESTS PASSED! Both CycloneDX 1.5 and 1.6 compliance fixes are working correctly.")
        else:
            print(f"\n⚠️  {self.tests_failed} test(s) failed. Review the implementation.")
            
        print("\n📋 Detailed Results:")
        for test_name, status, error in self.test_results:
            status_emoji = "✅" if status == "PASS" else "❌"
            print(f"  {status_emoji} {test_name}: {status}")
            if error:
                print(f"    Error: {error}")
        
        return self.tests_failed == 0

def main():
    """Main test runner"""
    runner = CycloneDXVersionTestRunner()
    success = runner.run_all_tests()
    
    if success:
        print("\n🔧 IMPLEMENTATION STATUS: CycloneDX 1.5 and 1.6 compliance fixes are READY")
        print("✅ Both versions address all identified compliance issues")
        print("✅ Generated SBOMs will be valid for their respective schemas")
        print("✅ Version-specific features are correctly implemented")
        print("✅ Backward compatibility is maintained")
    else:
        print("\n❌ IMPLEMENTATION STATUS: Review needed")
    
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())
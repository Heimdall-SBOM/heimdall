#!/bin/bash

echo "🔍 Clang-tidy Timing Test on MetadataExtractor.cpp"
echo "=================================================="
echo

# Test file
TEST_FILE="src/common/MetadataExtractor.cpp"

# Check if file exists
if [ ! -f "$TEST_FILE" ]; then
    echo "❌ Error: $TEST_FILE not found!"
    exit 1
fi

echo "📁 Test file: $TEST_FILE"
echo "📊 File size: $(stat -c%s "$TEST_FILE" 2>/dev/null || echo "unknown") bytes"
echo

# Function to run timing test
run_timing_test() {
    local config_name="$1"
    local config_file="$2"
    
    echo "🧪 Testing with $config_name configuration..."
    echo "   Config file: $config_file"
    
    # Clear any previous output
    rm -f /tmp/clang-tidy-output.txt
    
    # Run clang-tidy with timing
    start_time=$(date +%s.%N)
    timeout 600s clang-tidy --config-file="$config_file" -p build "$TEST_FILE" > /tmp/clang-tidy-output.txt 2>&1
    exit_code=$?
    end_time=$(date +%s.%N)
    
    # Calculate duration
    duration=$(echo "$end_time - $start_time" | bc -l 2>/dev/null || echo "unknown")
    
    # Count warnings/errors
    warning_count=$(grep -c "warning:" /tmp/clang-tidy-output.txt 2>/dev/null || echo "0")
    error_count=$(grep -c "error:" /tmp/clang-tidy-output.txt 2>/dev/null || echo "0")
    
    echo "   ⏱️  Duration: ${duration}s"
    echo "   📊 Warnings: $warning_count, Errors: $error_count"
    
    if [ $exit_code -eq 124 ]; then
        echo "   ⚠️  TIMEOUT (600s limit reached)"
    elif [ $exit_code -eq 0 ]; then
        echo "   ✅ Completed successfully"
    else
        echo "   ❌ Failed with exit code $exit_code"
    fi
    
    echo
}

# Test 1: Original configuration
run_timing_test "Original" ".clang-tidy-original"

# Test 2: New optimized configuration
run_timing_test "New Optimized" ".clang-tidy"

# Test 3: Light configuration
run_timing_test "Light" ".clang-tidy-light"

echo "🎯 Timing Test Summary"
echo "====================="
echo "All tests completed. Check the output above for detailed results."
echo
echo "💡 Note: The original configuration includes clang-analyzer-* checks"
echo "   which are computationally expensive and may cause timeouts." 
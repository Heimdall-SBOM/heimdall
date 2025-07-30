#!/bin/bash

# Apache License header template
LICENSE_HEADER='/*
Copyright 2025 The Heimdall Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

'

# Function to add license to a file
add_license_to_file() {
    local file="$1"
    local temp_file=$(mktemp)
    
    # Check if file starts with a comment block (like /** @file)
    if head -1 "$file" | grep -q "^/\*\*"; then
        # Insert license before the first comment block
        echo "$LICENSE_HEADER" > "$temp_file"
        cat "$file" >> "$temp_file"
    else
        # Insert license at the very beginning
        echo "$LICENSE_HEADER" > "$temp_file"
        cat "$file" >> "$temp_file"
    fi
    
    mv "$temp_file" "$file"
    echo "Added license to: $file"
}

# Process each file in the list
while IFS= read -r file; do
    if [ -f "$file" ]; then
        add_license_to_file "$file"
    fi
done < files_missing_license.txt

echo "License addition complete!" 
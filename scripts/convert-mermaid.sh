#!/bin/bash

# Convert Mermaid diagrams to images
# This script finds Mermaid code blocks in markdown files and converts them to images

set -e

# Create output directory for images
mkdir -p docs/images/mermaid

# Function to convert a Mermaid diagram to an image
convert_mermaid() {
    local input_file="$1"
    local output_file="$2"
    local diagram_content="$3"
    
    # Create a temporary file with the Mermaid content
    temp_file=$(mktemp)
    echo "$diagram_content" > "$temp_file"
    
    # Convert to PNG using mermaid-cli
    npx mmdc -i "$temp_file" -o "$output_file" -t default
    
    # Clean up
    rm "$temp_file"
    
    echo "Converted: $output_file"
}

# Process each markdown file
for file in docs/*.md; do
    if [ ! -f "$file" ]; then
        continue
    fi
    
    echo "Processing: $file"
    
    # Extract Mermaid diagrams and convert them
    diagram_count=0
    while IFS= read -r line; do
        if [[ "$line" =~ ^\`\`\`text$ ]]; then
            # Start of a code block
            in_code_block=true
            diagram_content=""
        elif [[ "$line" =~ ^\`\`\`$ ]] && [ "$in_code_block" = true ]; then
            # End of a code block
            in_code_block=false
            if [ -n "$diagram_content" ]; then
                # Check if this looks like a Mermaid diagram
                if echo "$diagram_content" | grep -q "graph\|flowchart\|sequenceDiagram\|classDiagram\|stateDiagram\|gantt\|pie\|journey\|gitgraph"; then
                    diagram_count=$((diagram_count + 1))
                    base_name=$(basename "$file" .md)
                    output_file="docs/images/mermaid/${base_name}_diagram_${diagram_count}.png"
                    
                    # Convert the diagram
                    convert_mermaid "$file" "$output_file" "$diagram_content"
                    
                    # Replace the code block with an image reference
                    # This is a simplified approach - in practice, you might want to be more careful
                    echo "![Mermaid Diagram](${output_file#docs/})" >> "${file}.tmp"
                else
                    # Not a Mermaid diagram, keep as text
                    echo "```text" >> "${file}.tmp"
                    echo "$diagram_content" >> "${file}.tmp"
                    echo "```" >> "${file}.tmp"
                fi
            fi
            diagram_content=""
        elif [ "$in_code_block" = true ]; then
            # Inside a code block, collect content
            diagram_content="${diagram_content}${line}"$'\n'
        else
            # Regular line, copy as-is
            echo "$line" >> "${file}.tmp"
        fi
    done < "$file"
    
    # Replace original file with modified version
    if [ -f "${file}.tmp" ]; then
        mv "${file}.tmp" "$file"
        echo "Updated: $file"
    fi
done

echo "Mermaid conversion complete!" 
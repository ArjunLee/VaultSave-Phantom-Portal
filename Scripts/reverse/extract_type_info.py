import sys
import json
import re

def extract_type_info(file_path, type_name):
    print(f"Searching for '{type_name}' in '{file_path}'...")
    
    # Pattern to match the start of the type definition
    # Matches: "TypeName": {
    # We allow flexible whitespace
    start_pattern = re.compile(f'\\s*"{re.escape(type_name)}":\\s*{{')
    
    with open(file_path, 'r', encoding='utf-8') as f:
        in_type = False
        brace_count = 0
        lines_found = []
        
        for line in f:
            if not in_type:
                if start_pattern.match(line):
                    in_type = True
                    brace_count = 1
                    lines_found.append(line)
                    # Check if the line also contains the closing brace (e.g. "Key": {})
                    if '}' in line:
                         # This is a simplification; a rigorous parser would be better but this is likely sufficient for dump files
                         # Count { and } in the line
                         open_braces = line.count('{')
                         close_braces = line.count('}')
                         brace_count = open_braces - close_braces
                         if brace_count <= 0:
                             break
            else:
                lines_found.append(line)
                open_braces = line.count('{')
                close_braces = line.count('}')
                brace_count += (open_braces - close_braces)
                
                if brace_count <= 0:
                    break
        
        if lines_found:
            print("".join(lines_found))
        else:
            print(f"Type '{type_name}' not found.")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python extract_type_info.py <dump_file> <type_name>")
        sys.exit(1)
        
    dump_file = sys.argv[1]
    target_type = sys.argv[2]
    
    extract_type_info(dump_file, target_type)

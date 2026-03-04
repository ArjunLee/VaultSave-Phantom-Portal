import sys
import json
import re

def search_types(file_path, pattern):
    print(f"Searching for keys matching '{pattern}' in '{file_path}'...")
    
    # We will read line by line and look for "TypeName": {
    # Since we don't want to parse the whole JSON structure (it's huge),
    # we just look for lines that look like keys at the top level or near it.
    # The dump format is usually "TypeName": { ... }
    
    regex = re.compile(f'"{pattern}":')
    
    with open(file_path, 'r', encoding='utf-8') as f:
        for line in f:
            # Check if line contains the pattern
            if regex.search(line):
                # Print the line stripped of whitespace
                print(line.strip())

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python search_types.py <dump_file> <pattern>")
        sys.exit(1)
        
    dump_file = sys.argv[1]
    pattern = sys.argv[2]
    
    search_types(dump_file, pattern)

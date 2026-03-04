import json
import re
import sys

file_path = r"e:\Dev\mini-program\Browser-Storage-Backup\game_mod_develop\dump\cc_dump.json"
search_term = sys.argv[1] if len(sys.argv) > 1 else "warp"

print(f"Searching for '{search_term}' in '{file_path}'...")

try:
    with open(file_path, "r", encoding="utf-8") as f:
        lines = f.readlines()
        
    for i, line in enumerate(lines):
        if search_term in line:
            print(f"Line {i+1}: {line.strip()}")
            # Print next 20 lines for context
            for j in range(1, 21):
                if i + j < len(lines):
                    print(f"  {lines[i+j].strip()}")
            print("-" * 40)
            
except Exception as e:
    print(f"Error: {e}")

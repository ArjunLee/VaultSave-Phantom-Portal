import sys
import json
import re

def search_combined(file_path, term1, term2):
    print(f"Searching for keys matching '{term1}' AND '{term2}' in '{file_path}'...")
    
    regex1 = re.compile(f'{term1}', re.IGNORECASE)
    regex2 = re.compile(f'{term2}', re.IGNORECASE)
    
    with open(file_path, 'r', encoding='utf-8') as f:
        for line in f:
            if regex1.search(line) and regex2.search(line):
                print(line.strip())

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python search_combined.py <dump_file> <term1> <term2>")
        sys.exit(1)
        
    dump_file = sys.argv[1]
    term1 = sys.argv[2]
    term2 = sys.argv[3]
    
    search_combined(dump_file, term1, term2)
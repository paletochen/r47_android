import re
import os

# File paths
header_path = '../src/c47/items.h'
c_path = '../src/c47/items.c'
output_path = 'C47_Items_Complete.tsv'

# 1. Build Lookup Dictionary from Header File
# This maps the index (e.g., 1) to the Macro Name (e.g., ITM_LBL)
lookup = {}
if os.path.exists(header_path):
    with open(header_path, 'r', encoding='utf-8') as h:
        for line in h:
            # Look for lines like: #define ITM_LBL 1 [4]
            match = re.search(r'#define\s+(\w+)\s+(\d+)', line)
            if match:
                name, idx = match.groups()
                lookup[idx] = name

# 2. Define standard columns for the TSV
columns = ["Index", "Macro_Name", "Function", "Param", "Catalog_Name", "Softmenu_Name", "TAM_Bits", "Status_Flags"]

# 3. Process the C Source File
with open(c_path, 'r', encoding='utf-8') as c, open(output_path, 'w', encoding='utf-8') as out:
    # Write the TSV Header
    out.write('\t'.join(columns) + '\n')
    
    for line in c:
        # Step A: Identify the Index (e.g., /* 1 */) [1]
        idx_match = re.search(r'/\*\s*(\d+)\s*\*/', line)
        if idx_match:
            idx = idx_match.group(1)
            macro_name = lookup.get(idx, f"UNKNOWN_{idx}")
            
            # Step B: Remove all comments [// or /* ... */]
            # Remove multi-line or inline block comments
            clean_line = re.sub(r'/\*.*?\*/', '', line)
            # Remove single-line comments
            clean_line = re.sub(r'//.*', '', clean_line).strip()
            
            # Step C: Extract data between curly braces { ... }
            content_match = re.search(r'\{(.*)\}', clean_line)
            
            if content_match:
                # Get the internal data and split by comma
                raw_data = content_match.group(1)
                # Replace OR indicators (|) with tabs for column alignment
                tabbed_data = raw_data.replace('|', '\t')
                
                # Split and clean parts to fit columns
                parts = [p.strip() for p in tabbed_data.split(',')]
                
                # Write to TSV: Index, Macro, then the processed parts
                # We join with tabs to maintain the spreadsheet structure
                out.write(f"{idx}\t{macro_name}\t" + "\t".join(parts) + "\n")
            
            # Special Handling: UNIT_CONV macros [5, 6]
            elif "UNIT_CONV" in clean_line:
                # These lines use a helper macro instead of the { ... } structure [7]
                conv_match = re.search(r'UNIT_CONV\((.*)\)', clean_line)
                if conv_match:
                    conv_params = conv_match.group(1).replace(',', '\t').replace('|', '\t')
                    out.write(f"{idx}\t{macro_name}\tfnUnitConvert\t{conv_params.strip()}\n")

print(f"Successfully created: {output_path}")

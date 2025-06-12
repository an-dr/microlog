#!/usr/bin/env python3
"""
pack_ulog.py - Concatenate ulog feature modules into single ulog.h and ulog.c for distribution.
"""
import os
from glob import glob

# Define the order of feature modules for header and source
header_modules = [
    'include/ulog.h',
]
source_modules = [
    'src/ulog.c',
]

def concat_files(files, out_path, guard=None):
    with open(out_path, 'w', encoding='utf-8') as out:
        if guard:
            out.write(f'#ifndef {guard}\n#define {guard}\n\n')
        for f in files:
            with open(f, encoding='utf-8') as src:
                out.write(f'// --- {os.path.basename(f)} ---\n')
                out.write(src.read())
                out.write('\n\n')
        if guard:
            out.write(f'#endif // {guard}\n')

def main():
    os.makedirs('dist', exist_ok=True)
    concat_files(header_modules, 'dist/ulog.h', guard='ULOG_H')
    concat_files(source_modules, 'dist/ulog.c')
    print('Generated dist/ulog.h and dist/ulog.c')

if __name__ == '__main__':
    main()

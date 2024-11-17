"""
This script updates the year in the copyright header of all files matching the given filter

Parameters:
arg1: (optional) Base directory for folders to search for files. If not present working directory is used.
"""

import sys
import os
import re
from datetime import datetime
import fileinput

directories = ['src', 'tests']
file_endings = ('.h', '.cpp', '.py')

# custom files that should be included
additional_files = ['.gitlab-ci.yml']

blacklist = ['src/ui/helper/qtColorTriangle.cpp', 'src/ui/helper/qtColorTriangle.h']
blacklist_dir = ('.pytest_cache', '__pycache__', 'regression_test/data')

# adjust directory path
base_dir = sys.argv[1] if len(sys.argv) > 1 else '.'
directories = [f'{base_dir}/{directory}' for directory in directories]
blacklist = [f'{base_dir}/{f}' for f in blacklist]
additional_files = [f'{base_dir}/{f}' for f in additional_files]
files = []

for directory in directories:
    for file in os.listdir(directory):
        file = f'{directory}/{file}'
        if os.path.isdir(file):
            if file.endswith(blacklist_dir):
                continue
            directories.append(file)
        else:
            files.append(f'{file}')

filtered = [f for f in files if f not in blacklist and f.endswith(file_endings)]
filtered.extend(additional_files)

current_year = datetime.now().year
regex = r"Copyright \(C\) \d{4} Forschungszentrum J.*lich GmbH"
pattern = re.compile(regex)
error_count = 0
update_count = 0

for f in filtered:
    try:
        with fileinput.FileInput(f, inplace=True, encoding="utf-8") as file:
            for i, line in enumerate(file, start=0):
                # Replace the copyright year in the third line
                if i == 2 and pattern.search(line):
                    print(re.sub(r"\d{4}", str(current_year), line), end="")
                    update_count += 1
                else:
                    print(line, end="")

    except Exception as e:
        print(str(e))
        error_count += 1

print(f"Updated Copyright in {update_count} files, {error_count} errors")

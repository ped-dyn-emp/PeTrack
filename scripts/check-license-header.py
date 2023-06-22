"""
This script checks if all files matching the given filter have the correct petrack license header.
It can be executed in the git-pipeline and finds typos or other mistakes like wrong copyright year.

Parameters:
arg1: (optional) Base directory for foldes to search for files. If not present executing directory is used.
"""

import sys
import os
import re
import difflib
from datetime import datetime

# directories and file endings to search fore license header
# base directory is given as command line attribute
directories = ['include', 'src', 'tests']
file_endings = ['h', 'cpp', 'py']

# custom files that should be included
additional_files = ['.gitlab-ci.yml']

blacklist = ['src/qtColorTriangle.cpp', 'include/qtColorTriangle.h']
blacklist_dir = ('.pytest_cache', '__pycache__', 'regression_test/data')

# adjust directory path
base_dir = sys.argv[1] if len(sys.argv) > 1 else '.'
directories = [f'{base_dir}/{directory}' for directory in directories]
blacklist = [f'{base_dir}/{f}' for f in blacklist]


current_year = datetime.now().year

license_c = f"""/*
 * PeTrack - Software for tracking pedestrians movement in videos
 * Copyright (C) {current_year} Forschungszentrum Jülich GmbH, IAS-7
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */""".strip()

license_py = f"""#
# PeTrack - Software for tracking pedestrians movement in videos
# Copyright (C) {current_year} Forschungszentrum Jülich GmbH, IAS-7
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#"""

regex = f'.*\.({"|".join(file_endings)})'
pattern = re.compile(regex)

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

filtered = [f for f in files if pattern.match(f) and not f in blacklist]

filtered.extend(additional_files)

print(f'checking {len(filtered)} files')

errors = {}

for path in filtered:
    try:
        # open with utf-8 to ensure correct umlauts
        with open(path, "r", encoding="utf-8-sig") as file:
            license = license_py if file.read(1) == '#' else license_c
            file.seek(0)
            preamble = file.read(len(license) + 1).strip()

            if preamble != license:
                diff = difflib.ndiff(license.strip().splitlines(keepends=True),
                                    preamble.strip().splitlines(keepends=True))
                errors[path] = ''.join(diff)
    except PermissionError:
        errors[path] = 'Permission denied'
    except UnicodeDecodeError:
        errors[path] = 'cannot decode file'

# printing results for better debugging if test fails

print(f'error count: {len(errors)}')

if len(errors) > 0:
    print('---------------------------')
    print('          details          ')

    for key in errors.keys():
        print(key)
        print(errors[key])
        print()

    print('---------------------------')
    # repeat, to not scroll up and search for this important information
    print(f'error count: {len(errors)}')
    sys.exit(1)

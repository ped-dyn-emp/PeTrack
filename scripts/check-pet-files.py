"""
This script validates all pet files in the given directory. This is needed because the xml parser from qt6 is very strict.

Parameters:
arg1: (optional) Base directory to search for files. If not present executing directory is used.
"""

import os
import sys
import xml.etree.ElementTree as ET


def validate_pet_files(directory):
    errors = []
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(".pet"):
                path = os.path.join(root, file)
                try:
                    tree = ET.parse(path)
                    errors.extend(check_src_attr(path, tree))
                except ET.ParseError as e:
                    errors.append((path, str(e)))
    return errors


def check_src_attr(path, tree):
    # files that should not have an src attribute
    disallowed = ["demo/00_files/00_empty.pet"]
    errors = []
    for critical_path in disallowed:
        if os.path.normpath(path).endswith(os.path.normpath(critical_path)):
            if not is_src_empty(tree):
                errors.append((path, 'MAIN tag "SRC" attribute must be empty.'))
    return errors


def is_src_empty(tree):
    main_elem = tree.getroot().find(".//MAIN")
    if main_elem is not None:
        src = main_elem.attrib.get("SRC")
        return not src.strip()
    return False


if __name__ == "__main__":
    directory = sys.argv[1] if len(sys.argv) > 1 else "."
    results = validate_pet_files(directory)

    if results:
        print("Invalid XML files found:\n")
        for path, error in results:
            print(f"{path}:\n  {error}\n")
        sys.exit(1)

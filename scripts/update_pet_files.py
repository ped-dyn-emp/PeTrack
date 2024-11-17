"""
This Script updates all .pet files used for the tests. Run this if the PeTrack version changed or you made some changes that
modified .pet files
How to use:
Run 'python scripts/update_pet_files.py' from either the petrack root directory, or another directory. If you don't run it from the petrack root directory,
make sure to pass the relative path to it.
After that try running the regression_tests again.
Parameters:
-r (--root): Relative path to the root petrack directory ('.' per default)
-p (--path): Path to the build directory relative to root ('build' per default)
"""

import pathlib
import subprocess
import re
import argparse

# directories that should not be traversed
blacklist_dirs = ["old_pet_files"]
pet_files = []


def open_pet_file(pet_file: str, build_dir: str) -> None:
    executables = [
        f"{build_dir}/petrack",
        "petrack.exe"
        "petrack"
    ]

    for exe in executables:
        try:
            process = subprocess.run([exe, "-project", pet_file, "-autoSave", pet_file, "-platform", "offscreen"], check=True) 
            return
        except FileNotFoundError:
            continue # try next executable
        except subprocess.CalledProcessError as e:
            print(f"Error executing {exe}: {e.stderr}")
            return

    print("No petrack executable found. Ensure you provided the correct build directory or if petrack is in your path")

    

def refresh_all(start_directory: str, build_dir: str):
    exclude_pattern = r'old(?!.*_truth\.pet$)|_autosave\.pet$'
    dir_path = pathlib.Path(start_directory)
    if not dir_path.exists():
        print(f"ERROR: {start_directory} does not exist. Check if you have given the right root dir of petrack")
        return
    for item in dir_path.rglob("*.pet"):
        if not any(blacklisted in str(item.parent) for blacklisted in blacklist_dirs) and not re.search(exclude_pattern, str(item).lower()):
            open_pet_file(str(item), build_dir)
            pet_files.append(str(item))


def remove_abs_paths(files: list[str]):
    """Remove all absolute paths in the pet file and ensures everything is set up to pass the tests"""
    def is_absolute_path(path: str) -> bool:
        """Check if a path is absolut (Unix or Windows)"""
        return path.startswith("/") or(len(path) > 1 and path[1] == ':')

    def get_relative_path(path1: str, path2: str) -> str:
        """Return the relative path from a pair of paths"""
        if is_absolute_path(path1):
            return path2
        return path1


    def replace_all(match):
        """Handle both absolute path removal and SOURCE_FRAME_IN + FILTER_BEFORE update"""
        #check if match is for the path pattern
        if match.group(1):
            paths = match.group(1).split(';')
            if len(paths) == 2:
                return f'="{get_relative_path(paths[0], paths[1])}"'
        #check if match is for SOURCE_FRAME_IN
        elif match.group(2):
            return 'SOURCE_FRAME_IN="-1"'
        #check if match is for FILTER_BEFORE
        elif match.group(3):
            return 'FILTER_BEFORE IMMUTABLE="0"'
        return match.group(0) #return unchanged if no match

    combined_pattern = r'="([^"]+;[^"]+)"|(SOURCE_FRAME_IN="\d+")|(FILTER_BEFORE IMMUTABLE="\d+")'


    for file_path in files:
        with open(file_path, 'r') as file:
            content = file.read()

        # Perform both replacements for paths and SOURCE_FRAME_IN+FILTER_BEFORE
        new_content = re.sub(combined_pattern, replace_all, content)
        
        # Write the updated content back to the file
        with open(file_path, 'w') as file:
            file.write(new_content)

def remove_autosaves(start_dir: str):
    """removes all petrack autosaves recursively in start_dir"""
    autosave_pattern = r'_autosave\.pet$' 
    dir_path = pathlib.Path(start_dir)
    for item in dir_path.rglob("*.pet"):
        if re.search(autosave_pattern, str(item).lower()):
            # remove autosave
            item.unlink()

def parse_args():
    parser = argparse.ArgumentParser(
        description="This Script updates all .pet files used for the tests. Run this if the PeTrack version changed or you made some changes that modified .pet files")

    parser.add_argument(
        "-r", "--root",
        help="Relative path to the root petrack directory", required=False,
        type=str
    )
    parser.add_argument(
        "-p", "--path",
        help="Path to the build directory relative to root", required=False,
        type=str
    )
    args = parser.parse_args()

    return args


def main():
    args = parse_args()
    root_dir = args.root if args.root else "."
    build_dir = f"{root_dir}/{args.path}" if args.path else f"{root_dir}/build"
    remove_autosaves(root_dir)
    refresh_all(f"{root_dir}/tests/regression_test/data/", build_dir)
    refresh_all(f"{root_dir}/demo/", build_dir)
    remove_abs_paths(pet_files)


if __name__ == "__main__":
    main()

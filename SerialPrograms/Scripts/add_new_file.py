#!/usr/bin/python3

"""
Add a new file to CMakeList.txt and SerialPrograms.pro

Usage: python3 add_new_file.py <path to new file relative to Arduino-Source/

The script will try to find the root code folder "Arduino-Source/" from the path of the current directory
where this script is executed.

Then it will add the new file path (as the input parameter of this program) to proper places in CMakeList.txt
and SerialPrograms.pro.

Example usage:
python3 add_new_file.py Source/PokemonSV/Inference/Map/PokemonSV_MapFlyMenuDetectorDetector.h
python3 add_new_file.py ../Common/Cpp/Color.cpp
"""

import sys
import os

from typing import List, Tuple

# Target path for the new flie, like 
target_path = ""
if len (sys.argv) > 1:
	target_path = sys.argv[1]
	print(f"Adding a new file path {target_path}")

# Sanitize target path:
SerialPrograms_prefix = "SerialPrograms/"
if target_path.startswith(SerialPrograms_prefix):
	target_path = target_path[len(SerialPrograms_prefix):]
Common_prefix = "Common/"
if target_path.startswith(Common_prefix) or target_path.startswith("ClientSource") or target_path.startswith("3rdParty"):
	target_path = "../" + target_path

cur_folder_path = os.path.abspath(os.getcwd())

# print(cur_folder_path)

split_path = cur_folder_path.split(os.sep)

code_root_pos = split_path.index('Arduino-Source')
code_root_path = os.sep.join(split_path[0:code_root_pos+1])
# print(code_root_path)


def read_lines(file_path: str) -> List[str]:
	with open(file_path, "r") as f:
		file_lines = f.readlines()
	file_lines = [line.rstrip() for line in file_lines]
	return file_lines

def get_code_file_range(file_lines: List[str], starting_line: str, ending_line: str) -> Tuple[int, int]:
	"""
	...
	<starting_line>
	code_line_1.cpp
	code_line_2.cpp
	<endling_line>
	...

	Return the tuple of indices (start, end) in the input `file_lines` so that code lines can be found
	by file_lines[start:end]
	"""
	code_file_list_start_pos = file_lines.index(starting_line) + 1
	code_file_list_end_pos = code_file_list_start_pos
	for line_no in range(code_file_list_start_pos, len(file_lines)):
		line = file_lines[line_no]
		if line == ending_line:
			code_file_list_end_pos = line_no
			break
	return code_file_list_start_pos, code_file_list_end_pos



cmakelists_path = os.path.join(code_root_path, "SerialPrograms", "CMakeLists.txt")
pro_path = os.path.join(code_root_path, "SerialPrograms", "SerialPrograms.pro")
print(f"CMakeLists path: {cmakelists_path}")
print(f"QT Pro project file path: {pro_path}")

file_lines = read_lines(cmakelists_path)
old_file_lines = file_lines

code_file_start, code_file_end = get_code_file_range(file_lines, "file(GLOB MAIN_SOURCES", ")")
code_file_lines = set(file_lines[code_file_start:code_file_end])
old_num_code_files = len(code_file_lines)
print(f"{old_num_code_files} lines from CMakeLists are for code file paths")

# Add new file
if len(target_path) > 0:
	code_file_lines.add("    " + target_path)

code_file_lines = list(code_file_lines)
code_file_lines.sort()
if len(code_file_lines) == old_num_code_files:
	print("Warning: you are adding an existing file! The added file is ignored")

# print(f"\"{code_file_lines[0]}\"")

file_lines = file_lines[0:code_file_start] + code_file_lines + file_lines[code_file_end:]
file_lines = [line + "\r\n" for line in file_lines]
with open(cmakelists_path, "w") as f:
	f.writelines(file_lines)
print(f"Writed changes back to {cmakelists_path}")

if not os.path.exists(pro_path):
	print(f"Pro file not found. End.")
	exit(0)

file_lines = read_lines(pro_path)

# print("\n".join(file_lines[0:70]))

# lines = [line for line in file_lines if line.startswith("SOURCES")]
# print(lines)


def process_pro_lines(file_lines: List[str], starting_line: str, new_path: str) -> List[str]:

	code_file_start, code_file_end = get_code_file_range(file_lines, starting_line, "")
	# print(code_file_start, code_file_end)
	# print(f"Starting line {file_lines[code_file_start]}")

	code_file_lines = file_lines[code_file_start:code_file_end]

	code_file_lines = [ line[0:-1].rstrip() if line.endswith("\\") else line for line in code_file_lines]

	if len(new_path) > 0:
		code_file_lines.append("    " + new_path)

	# Remove duplicates and sort
	code_file_lines = list(set(code_file_lines))
	code_file_lines.sort()

	# Add back " \\":
	code_file_lines = [line + " \\" for line in code_file_lines]
	code_file_lines[-1] = code_file_lines[-1][0:-2]

	# print("\n".join(code_file_lines[0:10]))

	file_lines = file_lines[0:code_file_start] + code_file_lines + file_lines[code_file_end:]

	return file_lines

if target_path.endswith(".cpp"):
	file_lines = process_pro_lines(file_lines, "SOURCES += \\", target_path)
elif target_path.endswith(".h") or target_path.endswith(".tpp"):
	file_lines = process_pro_lines(file_lines, "HEADERS += \\", target_path)
else:
	file_lines = process_pro_lines(file_lines, "SOURCES += \\", "")
	file_lines = process_pro_lines(file_lines, "HEADERS += \\", "")

# print("\n".join(file_lines[0:30]))

# Add back CRLF
file_lines = [line + "\r\n" for line in file_lines]

with open(pro_path, "w") as f:
	f.writelines(file_lines)
print(f"Writed changes back to {pro_path}")





#!Python3

"""
Build C++ code from a template file.

This is used to remove the manual work of creating boilerplate code like visual detector classes.

Example usage:

- python3 build_code_from_template.py VisualDetector PokemonSV Map PokeCenter Icon
  Generates PokemonSV_MapPokeCenterIconDetector.h and PokemonSV_MapPokeCenterIconDetector.cpp.
- python3 build_code_from_template.py Program PokemonSV Auto Story
  Generates PokemonSV_AutoStory.h and PokemonSV_AutoStory.cpp.
"""

import sys
import os

from typing import Dict, List


def apply_line_replacement(line: str, mapping: Dict[str, str]) -> str:
	for source, target in mapping.items():
		line = line.replace(source, target)
	return line


def build_file_from_template(
	mapping: Dict[str, str],
	template_folder: str,
	template_filename: str,
) -> None:
	"""
	generate a file from a template file by mapping: template str -> target str
	The file is saved at the current working dir.
	"""

	file_ext: str = template_filename.split('.')[-1]
	template_filepath: str = os.path.join(template_folder, template_filename)

	target_filename: str = apply_line_replacement(template_filename, mapping)

	with open(template_filepath, "r") as f:
		lines = f.readlines()
	lines = [apply_line_replacement(line, mapping) for line in lines]
	
	with open(target_filename, "w", newline='\r\n') as f:
		f.writelines(lines)

	print(f"Saved template {file_ext} file to {target_filename}")


def build_files_from_templates(
	mapping: Dict[str, str],
	template_folder: str,
	template_filenames: List[str],
) -> None:
	"""
	generate files from template files by a mapping: template str -> target str
	The files are saved at the current working dir.
	"""

	for filename in template_filenames:
		build_file_from_template(
			mapping=mapping,
			template_folder=template_folder,
			template_filename=filename,
		)


def create_cpp_class_name(name: str) -> str:
	"""
	Given a name (e.g. "Three-Segment Dudunsparce Finder"), convert it into a C++ class name (like "ThreeSegmentDudunsparceFinder")
	"""
	return name.replace(" ", "").replace("-", "").replace("_", "")


if len(sys.argv) == 1:
	print(
		"Usage:\n"
		f"python3 {sys.argv[0]} VisualDetector PokemonSV Map PokeCenter Icon"
		"  - Generates PokemonSV_MapPokeCenterIconDetector.h and PokemonSV_MapPokeCenterIconDetector.cpp"
	)
	exit(0)

cur_folder_path = os.path.abspath(os.getcwd())

split_path = cur_folder_path.split(os.sep)

code_root_pos = split_path.index('Arduino-Source')
code_root_path = os.sep.join(split_path[0:code_root_pos+1])

print(f"Found code root folder: {code_root_path}")

template_folder = os.path.join(code_root_path, "SerialPrograms", "Scripts", "CodeTemplates")
print(f"Template folder: {template_folder}")

template_folder = os.path.join(template_folder, sys.argv[1])

if sys.argv[1] == "VisualDetector":
	assert len(sys.argv) >= 4
	game_name = sys.argv[2]  # e.g. "PokemonSV"
	object_name = " ".join(sys.argv[3:])  # e.g. "Map PokeCenter Icon"
	print(f"Building a visual detector for object {object_name} in game {game_name}.")

	mapping = {
		"GameName": game_name,
		"Object Name": object_name,
		"ObjectName": create_cpp_class_name(object_name)
	}

	build_files_from_templates(
		mapping=mapping,
		template_folder=template_folder,
		template_filenames=[
			"GameName_ObjectNameDetector.h",
			"GameName_ObjectNameDetector.cpp",
		]
	)
elif sys.argv[1] == "Program":
	assert len(sys.argv) >= 4
	game_name = sys.argv[2]  # e.g. "PokemonSV"
	program_name = " ".join(sys.argv[3:])  # e.g. "Auto Story"
	print(f"Building a Program with name {program_name} in game {game_name}.")

	mapping = {
		"GameName": game_name,
		"Program Name": program_name,
		"ProgramName": create_cpp_class_name(program_name)
	}

	build_files_from_templates(
		mapping=mapping,
		template_folder=template_folder,
		template_filenames=[
			"GameName_ProgramName.h",
			"GameName_ProgramName.cpp",
		]
	)







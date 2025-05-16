#!Python3

"""
Build C++ code from a template file.

This is used to remove the manual work of creating boilerplate code like visual detector classes.

Example usage:

- python3 build_code_from_template.py VisualDetector PokemonSV Map PokeCenter Icon
  Generates PokemonSV_MapPokeCenterIconDetector.h and PokemonSV_MapPokeCenterIconDetector.cpp.
"""

import sys
import os

from typing import Dict

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


def apply_replacement(line: str, mapping: Dict[str, str]) -> str:
	for source, target in mapping.items():
		line = line.replace(source, target)
	return line

if sys.argv[1] == "VisualDetector":
	assert len(sys.argv) >= 4
	game_name = sys.argv[2]  # e.g. "PokemonSV"
	object_name = " ".join(sys.argv[3:])  # e.g. "Map PokeCenter Icon"
	print(f"Building a visual detector for object {object_name} in game {game_name}.")

	mapping = {
		"GameName": game_name,
		"Object Name": object_name,
		"ObjectName": object_name.replace(" ", ""),
	}

	template_h_filename = "GameName_ObjectNameDetector.h"
	template_cpp_filename = "GameName_ObjectNameDetector.cpp"

	template_folder = os.path.join(template_folder, sys.argv[1])

	template_h_filepath = os.path.join(template_folder, template_h_filename)
	template_cpp_filepath = os.path.join(template_folder, template_cpp_filename)

	target_h_filename = apply_replacement(template_h_filename, mapping)
	target_cpp_filename = apply_replacement(template_cpp_filename, mapping)


	with open(template_h_filepath, "r") as f:
		lines = f.readlines()
	lines = [apply_replacement(line, mapping) for line in lines]
	
	with open(target_h_filename, "w", newline='\r\n') as f:
		f.writelines(lines)

	print(f"Saved template h file to {target_h_filename}")

	with open(template_cpp_filepath, "r") as f:
		lines = f.readlines()
	lines = [apply_replacement(line, mapping) for line in lines]
	
	with open(target_cpp_filename, "w", newline='\r\n') as f:
		f.writelines(lines)

	print(f"Saved template cpp file to {target_cpp_filename}")






"""
Generates an OCR json file for dictionary matching

Set-up:
- Create a file named "target_words.txt". Put all your target dictionary words in this file,
with a new line between each phrase. Put this file in the same folder as this script.
- Adjust the start_line and end_line variables, which are hard-coded.
This helps to limit the script's search of the text dump files.
This helps with dealing with cases where the same English word is used by different parts of the game, but in a different
language, different words may be used. e.g. "Close" in English can be "Nah" or "Schließen" in German.
- Put all the text dump files for all languages in the same folder as this script. You can get the text dump files
from data miners such as https://x.com/mattyoukhana_
  - Ensure the names of the text dump files match the file names in the function `get_text_dump_filename_from_language()`

How it works:
- each target word from "target_words.txt" is converted to a slug.
i.e. spaces are converted to dashes and converted to lowercase
- for each target word, "English.txt" is searched to find the location (line number and column) of the word. All the
text dump files for the various languages are formatted similarly. So, the above location will also point us to
the corresponding target word in other languages.
- the json OCR file is then created based on the slugs and the location of each target word for each of the languages
"""

from typing import TextIO


start_line, end_line = 39560, 39665 # for menu option items
# start_line, end_line = 50255, 51178  # for all moves

def find_location_from_target_word(text_dump_filename: str, target_word: str, start: int, end: int) -> tuple[int, int]:
    with open(text_dump_filename, 'r', encoding='utf-8', errors='ignore') as file:
        for row, line in enumerate(file):
            if row < start:
                continue
            if row > end:
                break
            split_line = line.split("\t")
            for column, cell in enumerate(split_line):
                if cell.strip() == target_word:
                    return row, column
        raise Exception("unable to find target word: " + target_word)

def find_target_word_from_location(text_dump_filename: str, location: tuple[int, int]) -> str:
    target_row = location[0]
    target_column = location[1]
    if target_row < 0:
        raise IndexError("target_row index out of range")
    with open(text_dump_filename, 'r', encoding='utf-8', errors='ignore') as file:
        for row, line in enumerate(file):
            if row == target_row:
                split_line = line.split("\t")
                if target_column < 0 or target_column >= len(split_line):
                    raise IndexError("target_column index out of range")
                for column, cell in enumerate(split_line):
                    if column == target_column:
                        return cell.strip()
    raise IndexError("target_row index out of range")



def get_slug_from_target_word(target_word: str) -> str:
    slug = (target_word.replace(" ", "-")
            .replace("'", "")
            .replace("’", "")
            .replace("é","e")
            .casefold())
    return slug

def generate_ocr_json_file(target_words_filename: str, start: int, end: int):
    list_of_slug_location_pair = get_list_of_slug_location_pair(target_words_filename, "English.txt", start, end)
    json_output_filename = "OCR.json"
    with open(json_output_filename, 'a', encoding='utf-8') as file:
        file.truncate(0) # clear file contents first
        file.write("{\n")
        generate_ocr_json_file_one_language(file, "deu", list_of_slug_location_pair)
        file.write(",\n")
        generate_ocr_json_file_one_language(file, "eng", list_of_slug_location_pair)
        file.write(",\n")
        generate_ocr_json_file_one_language(file, "spa", list_of_slug_location_pair)
        file.write(",\n")
        generate_ocr_json_file_one_language(file, "fra", list_of_slug_location_pair)
        file.write(",\n")
        generate_ocr_json_file_one_language(file, "ita", list_of_slug_location_pair)
        file.write(",\n")
        generate_ocr_json_file_one_language(file, "kor", list_of_slug_location_pair)
        file.write(",\n")
        generate_ocr_json_file_one_language(file, "jpn", list_of_slug_location_pair)
        file.write(",\n")
        generate_ocr_json_file_one_language(file, "chi_sim", list_of_slug_location_pair)
        file.write(",\n")
        generate_ocr_json_file_one_language(file, "chi_tra", list_of_slug_location_pair)
        file.write("\n}")


# start and end parameter so we only search certain portions of file.
# This helps with dealing with cases where the same English word is used by different parts of the game,
# but in a different language, different words may be used.
# e.g. "Close" in English can be "Nah" or "Schließen" in German.
def get_list_of_slug_location_pair(target_words_filename: str, text_dump_filename: str, start: int, end: int) -> list[tuple[str, tuple[int, int]]]:
    with open(target_words_filename, 'r', encoding='utf-8', errors='ignore') as file:
        list_of_slug_location_pair = []
        for line in file:
            target_word = line.strip()
            slug = get_slug_from_target_word(target_word)
            location = find_location_from_target_word(text_dump_filename, target_word, start, end)
            slug_location_pair = (slug, location)
            list_of_slug_location_pair.append(slug_location_pair)
    print("Done finding list_of_slug_location_pair")
    return list_of_slug_location_pair


def generate_ocr_json_file_one_language(file: TextIO, language: str, list_of_slug_location_pair: list[tuple[str, tuple[int, int]]]):
    text_dump_filename = get_text_dump_filename_from_language(language)
    file.write("\t" + "\"" + language + "\"" + ": " + "{" + "\n")
    for i, slug_location_pair in enumerate(list_of_slug_location_pair):
        slug = slug_location_pair[0]
        location = slug_location_pair[1]
        target_word = find_target_word_from_location(text_dump_filename, location)
        if i != 0:
            file.write(",\n")
        file.write("\t\t" + "\"" + slug + "\"" + ": " + "[ " + "\"" + target_word + "\"" + " ]")
    file.write("\n\t}")
    print("Done language: " + language)

def get_text_dump_filename_from_language(language: str) -> str:
    text_dump_filename = ""
    match language:
        case "deu":
            text_dump_filename = "Deutsch.txt"
        case "eng":
            text_dump_filename = "English.txt"
        case "spa":
            text_dump_filename = "Español (España).txt"
        case "fra":
            text_dump_filename = "Français.txt"
        case "ita":
            text_dump_filename = "Italiano.txt"
        case "kor":
            text_dump_filename = "한국어.txt"
        case "jpn":
            text_dump_filename = "日本語 (カタカナ).txt"
        case "chi_sim":
            text_dump_filename = "简体中文.txt"
        case "chi_tra":
            text_dump_filename = "繁體中文.txt"

    if text_dump_filename == "":
        raise Exception("Language not recognized")
    return text_dump_filename


generate_ocr_json_file("target_words.txt", start_line, end_line)



#!/usr/bin/env python3
"""
Convert a Pokemon name txt file to a JSON slug list.
Each line of the txt file is converted to a lowercase slug and validated against
the National Pokedex database.

The pokemon name txt file format is one line per pokemon name.
Its content can be downloaded from a site like Serebii.
"""

import argparse
import json
import re
import unicodedata
from pathlib import Path


def normalize_pokemon_name(name: str) -> str:
    """
    Convert a Pokemon name to a slug format.
    - Converts to lowercase
    - Removes accents/diacritics
    - Handles special characters
    """
    # Remove accents and normalize unicode characters
    # NFD = Canonical Decomposition
    nfd = unicodedata.normalize('NFD', name)
    # Filter out combining characters (accents)
    name_no_accents = ''.join(char for char in nfd if unicodedata.category(char) != 'Mn')

    # Convert to lowercase
    slug = name_no_accents.lower()

    # Handle special cases
    # Replace apostrophes with '-d' for Farfetch'd and Sirfetch'd
    slug = slug.replace("'d", "-d")

    # Replace periods and spaces with hyphens for Mr. Mime, Mime Jr., Mr. Rime
    slug = slug.replace(". ", "-")  # Handle "Mr. " -> "mr-"
    slug = slug.replace(" ", "-")   # Handle remaining spaces
    slug = slug.rstrip(".")         # Remove trailing periods (e.g., "mime-jr." -> "mime-jr")

    return slug


def main(input_txt: Path, database_json: Path, output_json: Path):
    """
    Convert Pokemon names from txt file to JSON slugs.

    Args:
        input_txt: Path to input text file with Pokemon names (one per line)
        database_json: Path to National Pokedex JSON database for validation
        output_json: Path to output JSON file
    """

    # Load the National Pokedex database
    with open(database_json, 'r', encoding='utf-8') as f:
        national_pokedex = json.load(f)

    # Convert to set for faster lookup
    national_pokedex_set = set(national_pokedex)

    # Read the input txt file and convert names
    pokemon_slugs = []
    not_found = []

    with open(input_txt, 'r', encoding='utf-8') as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue

            # Pokemon name is the entire line
            pokemon_name = line

            # Convert to slug
            slug = normalize_pokemon_name(pokemon_name)

            # Validate against database
            if slug in national_pokedex_set:
                pokemon_slugs.append(slug)
                print(f"✓ {pokemon_name} -> {slug}")
            else:
                not_found.append((pokemon_name, slug))
                print(f"✗ {pokemon_name} -> {slug} (NOT FOUND in database)")

    # Report results
    print(f"\n{'='*60}")
    print(f"Total Pokemon processed: {len(pokemon_slugs) + len(not_found)}")
    print(f"Successfully validated: {len(pokemon_slugs)}")
    print(f"Not found in database: {len(not_found)}")

    if not_found:
        print(f"\nPokemon not found in database:")
        for name, slug in not_found:
            print(f"  - {name} ({slug})")

    # Write output JSON
    with open(output_json, 'w', encoding='utf-8') as f:
        json.dump(pokemon_slugs, f, indent=4, ensure_ascii=False)

    print(f"\n✓ Output written to: {output_json}")
    print(f"  Total entries: {len(pokemon_slugs)}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Convert Pokemon name txt file to JSON slug list.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Example usage:
  %(prog)s input.txt Pokedex-National.json output.json
  %(prog)s Pokedex-Lumiose.txt Pokedex-National.json Pokedex-Lumiose.json
        """
    )

    parser.add_argument(
        'input_txt',
        type=Path,
        help='Input text file with Pokemon names (one per line)'
    )

    parser.add_argument(
        'database_json',
        type=Path,
        help='National Pokedex JSON database file for slug validation'
    )

    parser.add_argument(
        'output_json',
        type=Path,
        help='Output JSON file to save the slug list'
    )

    args = parser.parse_args()

    main(args.input_txt, args.database_json, args.output_json)

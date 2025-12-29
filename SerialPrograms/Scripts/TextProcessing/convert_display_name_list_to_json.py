#!/usr/bin/env python3
"""
Convert a display name txt file to a JSON slug list.
Each line of the txt file is converted to a lowercase slug format.

The display name txt file format is one line per item name.
"""

import argparse
import json
import re
import unicodedata
from pathlib import Path


def normalize_display_name(name: str) -> str:
    """
    Convert a display name to a slug format.
    - Converts to lowercase
    - Removes accents/diacritics
    - Non-alphanumeric characters become hyphens
    """
    # Remove accents and normalize unicode characters
    # NFD = Canonical Decomposition
    nfd = unicodedata.normalize('NFD', name)
    # Filter out combining characters (accents)
    name_no_accents = ''.join(char for char in nfd if unicodedata.category(char) != 'Mn')

    # Convert to lowercase
    slug = name_no_accents.lower()

    # Replace non-alphanumeric characters with hyphens
    # Keep only letters, numbers, and convert everything else to hyphen
    slug = re.sub(r'[^a-z0-9]+', '-', slug)

    # Remove leading/trailing hyphens
    slug = slug.strip('-')

    return slug


def main(input_txt: Path, output_json: Path):
    """
    Convert display names from txt file to JSON slugs.

    Args:
        input_txt: Path to input text file with display names (one per line)
        output_json: Path to output JSON file
    """

    # Read the input txt file and convert names
    display_name_slugs = []

    with open(input_txt, 'r', encoding='utf-8') as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue

            # Display name is the entire line
            display_name = line

            # Convert to slug
            slug = normalize_display_name(display_name)

            display_name_slugs.append(slug)
            print(f"{display_name} -> {slug}")

    # Report results
    print(f"\n{'='*60}")
    print(f"Total items processed: {len(display_name_slugs)}")

    # Write output JSON
    with open(output_json, 'w', encoding='utf-8') as f:
        json.dump(display_name_slugs, f, indent=4, ensure_ascii=False)

    print(f"\n✓ Output written to: {output_json}")
    print(f"  Total entries: {len(display_name_slugs)}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Convert display name txt file to JSON slug list.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Example usage:
  %(prog)s input.txt output.json
  %(prog)s items.txt items-slugs.json
        """
    )

    parser.add_argument(
        'input_txt',
        type=Path,
        help='Input text file with display names (one per line)'
    )

    parser.add_argument(
        'output_json',
        type=Path,
        help='Output JSON file to save the slug list'
    )

    args = parser.parse_args()

    main(args.input_txt, args.output_json)

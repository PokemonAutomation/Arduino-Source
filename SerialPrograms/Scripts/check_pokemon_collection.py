#!/usr/bin/env python3
"""
Script to check which Pokemon from a subset you've already collected.

Usage:
    python check_pokemon_collection.py --subset SUBSET_FILE --pages BOX_PAGE_FILE1 [BOX_PAGE_FILE2 ...] --trainers TRAINER_FILE [--national NATIONAL_DEX]

Example:
    python check_pokemon_collection.py \
        --subset /path/to/Pokedex-Lumiose.json \
        --pages /path/to/box_order_page_1.json /path/to/box_order_page_2.json \
        --trainers /path/to/my_ot.json \
        --national /path/to/Pokedex-National.json

    You can also use "*" when specifying pages:
    python check_pokemon_collection.py \
        --subset /path/to/Pokedex-Lumiose.json \
        --pages /path/to/box_order_page_*.json \
        --trainers /path/to/my_ot.json \
        --national /path/to/Pokedex-National.json
"""

import json
import argparse
from pathlib import Path
from typing import Dict, List, Set


def load_json(filepath: str) -> any:
    """Load a JSON file."""
    # Use utf-8-sig to handle BOM if present
    with open(filepath, 'r', encoding='utf-8-sig') as f:
        return json.load(f)


def load_national_pokedex(filepath: str) -> List[str]:
    """Load the national pokedex mapping (dex number -> pokemon name)."""
    pokedex = load_json(filepath)
    # The array is 0-indexed but national dex starts at 1
    # So index 0 = dex #1, index 1 = dex #2, etc.
    return pokedex


def load_subset(filepath: str) -> List[str]:
    """Load the subset of pokemon names we're looking for (preserving order)."""
    subset_list = load_json(filepath)
    return subset_list


def load_trainer_mapping(filepath: str) -> Dict[int, str]:
    """Load trainer ID to name mapping."""
    mapping_str = load_json(filepath)
    # Convert string keys to integers
    return {int(k): v for k, v in mapping_str.items()}


def load_box_pages(filepaths: List[str]) -> List[dict]:
    """Load all box page JSON files and combine them."""
    all_pokemon = []
    for page_idx, filepath in enumerate(filepaths):
        pokemon_list = load_json(filepath)
        for pokemon in pokemon_list:
            pokemon['page'] = page_idx
        all_pokemon.extend(pokemon_list)
    return all_pokemon


def get_pokemon_name(national_dex_number: int, national_pokedex: List[str]) -> str:
    """Get pokemon name from national dex number."""
    # national_dex_number 1 -> index 0
    index = national_dex_number - 1
    if 0 <= index < len(national_pokedex):
        return national_pokedex[index]
    return f"Unknown (#{national_dex_number})"


def get_trainer_name(ot_id: int, trainer_mapping: Dict[int, str]) -> str:
    """Get trainer name from OT ID, or return ID if not found."""
    return trainer_mapping.get(ot_id, str(ot_id))


def is_valid_pokemon(pokemon: dict) -> bool:
    """Check if a pokemon object has the minimum required fields (not an empty slot)."""
    required_fields = ['national_dex_number', 'ot_id']
    return all(field in pokemon for field in required_fields)


def format_gender(gender: str) -> str:
    """Format gender for display."""
    if gender == "Male":
        return "♂"
    elif gender == "Female":
        return "♀"
    else:
        return "-"


def format_ball(ball_slug: str) -> str:
    """Format pokeball name for display."""
    # Remove '-ball' suffix and capitalize
    if ball_slug.endswith('-ball'):
        ball_name = ball_slug[:-5]
    else:
        ball_name = ball_slug
    return ball_name.capitalize()


def main():
    parser = argparse.ArgumentParser(
        description='Check which Pokemon from a subset you have collected.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    parser.add_argument('--subset', required=True, help='Path to subset pokedex JSON (e.g., Pokedex-Lumiose.json)')
    parser.add_argument('--pages', nargs='+', required=True, help='Path(s) to one or more JSON files where each file stores a single page of box order')
    parser.add_argument('--trainers', required=True, help='Path to trainer ID mapping JSON (my_ot.json)')
    parser.add_argument('--national', help='Path to national pokedex JSON (default: auto-detect from subset location)')

    args = parser.parse_args()

    # Auto-detect national pokedex if not provided
    if args.national:
        national_path = args.national
    else:
        # Try to find Pokedex-National.json in the same directory as subset
        subset_dir = Path(args.subset).parent
        national_path = subset_dir / 'Pokedex-National.json'
        if not national_path.exists():
            # Try the Pokemon/Pokedex directory
            national_path = subset_dir.parent / 'Pokedex' / 'Pokedex-National.json'
        if not national_path.exists():
            print(f"Error: Could not find Pokedex-National.json. Please specify with --national")
            return 1

    # Load all data
    print(f"Loading national pokedex from: {national_path}")
    national_pokedex = load_national_pokedex(str(national_path))

    print(f"Loading subset from: {args.subset}")
    subset = load_subset(args.subset)

    print(f"Loading trainer mapping from: {args.trainers}")
    trainer_mapping = load_trainer_mapping(args.trainers)

    print(f"Loading {len(args.pages)} box page(s)...")
    all_pokemon = load_box_pages(args.pages)

    # Filter pokemon in subset
    print(f"\nSearching for {len(subset)} pokemon in {len(all_pokemon)} slots...")
    print("=" * 100)

    # Build a mapping from pokemon name to list of found instances
    found_pokemon_map = {}
    for pokemon in all_pokemon:
        if not is_valid_pokemon(pokemon):
            continue

        pokemon_name = get_pokemon_name(pokemon['national_dex_number'], national_pokedex)

        if pokemon_name in subset:
            if pokemon_name not in found_pokemon_map:
                found_pokemon_map[pokemon_name] = []

            found_pokemon_map[pokemon_name].append({
                'name': pokemon_name,
                'trainer': get_trainer_name(pokemon.get('ot_id'), trainer_mapping),
                'gender': pokemon.get('gender', 'Unknown'),
                'ball': pokemon.get('ball_slug', 'unknown'),
                'alpha': pokemon.get('alpha', False),
                'shiny': pokemon.get('shiny', False),
                'page': pokemon.get('page') + 1,   # make them 1-indexed
                'box': pokemon.get('box', -2) + 1,
                'row': pokemon.get('row', -2) + 1,
                'column': pokemon.get('column', -2) + 1,
            })

    # Flatten results in the order of the subset
    found_pokemon = []
    for pokemon_name in subset:
        if pokemon_name in found_pokemon_map:
            found_pokemon.extend(found_pokemon_map[pokemon_name])

    # Print results
    print(f"\n{'Pokemon':<20} {'Trainer':<15} {'Gender':<8} {'Ball':<15} {'Alpha':<7} {'Shiny':<7} {'Location'}")
    print("=" * 110)

    for p in found_pokemon:
        alpha_str = "✓" if p['alpha'] else "-"
        shiny_str = "✓" if p['shiny'] else "-"
        location = f"Page {p['page']} Box {p['box']}, Row {p['row']} Col {p['column']}"

        print(f"{p['name']:<20} {p['trainer']:<15} {format_gender(p['gender']):<8} "
              f"{format_ball(p['ball']):<15} {alpha_str:<7} {shiny_str:<7} {location}")

    # Summary
    print("=" * 110)
    # Count unique pokemon found
    unique_found = len(set(p['name'] for p in found_pokemon))
    print(f"\nFound {unique_found} / {len(subset)} unique pokemon from subset")
    print(f"Total instances: {len(found_pokemon)}")
    print(f"Missing {len(subset) - unique_found} pokemon")

    # Show missing pokemon
    if unique_found < len(subset):
        found_names = {p['name'] for p in found_pokemon}
        missing = [name for name in subset if name not in found_names]
        print(f"\nMissing pokemon:")
        for i, name in enumerate(missing, 1):
            print(f"  {i}. {name}")


if __name__ == '__main__':
    main()

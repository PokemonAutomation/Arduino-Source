#!/usr/bin/env python3
"""
Script to update wiki links from old format to new format.
Old format: Programs/PokemonSV/AutoStory.html
New format: Programs/PokemonSV/AutoStory.html
"""

import os
import re
import sys
from pathlib import Path


def update_wiki_links(content):
    """
    Update wiki links from old format to new format.

    Args:
        content: File content as string

    Returns:
        Updated content with new wiki links
    """
    # Pattern to match old wiki links
    # Matches: ComputerControl/blob/master/Wiki/ followed by path and .md extension
    pattern = r'ComputerControl/blob/master/Wiki/([^)\s]+)\.md'

    # Replace with new format: just the path with .html extension
    replacement = r'\1.html'

    updated_content = re.sub(pattern, replacement, content)

    return updated_content


def process_file(file_path):
    """
    Process a single file to update wiki links.

    Args:
        file_path: Path to the file to process

    Returns:
        True if file was modified, False otherwise
    """
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        updated_content = update_wiki_links(content)

        if content != updated_content:
            # Convert to Windows line endings (CRLF)
            updated_content = updated_content.replace('\r\n', '\n').replace('\n', '\r\n')

            with open(file_path, 'w', encoding='utf-8', newline='') as f:
                f.write(updated_content)
            return True

        return False
    except Exception as e:
        print(f"Error processing {file_path}: {e}", file=sys.stderr)
        return False


def find_files_with_wiki_links(root_dir, extensions=None):
    """
    Find all files that might contain wiki links.

    Args:
        root_dir: Root directory to search
        extensions: List of file extensions to search (default: code files only)

    Returns:
        List of file paths
    """
    if extensions is None:
        # Only process code files (.cpp, .h, .c, .hpp, .cc, .cxx)
        extensions = ['.cpp', '.h', '.c', '.hpp', '.cc', '.cxx', '.tpp']

    files_to_process = []

    for root, dirs, files in os.walk(root_dir):
        # Skip certain directories
        dirs[:] = [d for d in dirs if d not in ['.git', 'build', 'Build', '__pycache__']]

        for file in files:
            if any(file.endswith(ext) for ext in extensions):
                files_to_process.append(os.path.join(root, file))

    return files_to_process


def main():
    """Main function to update wiki links in the repository."""
    # Determine the root directory (SerialPrograms directory)
    script_dir = Path(__file__).parent
    serial_programs_dir = script_dir.parent

    print(f"Searching for wiki links in: {serial_programs_dir}")
    print("Looking for pattern: ComputerControl/blob/master/Wiki/.../*.md")
    print("Replacing with: .../....html")
    print("Target files: C/C++ code files (.cpp, .h, .c, .hpp, .cc, .cxx, .tpp)")
    print("Output: Windows line endings (CRLF)\n")

    # Find all relevant files
    files = find_files_with_wiki_links(serial_programs_dir)
    print(f"Found {len(files)} files to scan\n")

    # Process each file
    modified_count = 0
    modified_files = []

    for file_path in files:
        if process_file(file_path):
            modified_count += 1
            modified_files.append(file_path)
            print(f"Updated: {os.path.relpath(file_path, serial_programs_dir)}")

    print(f"\n{'='*60}")
    print(f"Summary: {modified_count} file(s) modified")

    if modified_files:
        print("\nModified files:")
        for file_path in modified_files:
            print(f"  - {os.path.relpath(file_path, serial_programs_dir)}")

    return 0 if modified_count >= 0 else 1


if __name__ == "__main__":
    sys.exit(main())

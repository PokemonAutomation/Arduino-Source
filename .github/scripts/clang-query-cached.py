#!/usr/bin/env python3
"""Run clang-query on every source file of a build, but skip the files whose result is known.

The "Run clang query" step in ``cpp-ci-serial-programs-base.yml`` runs clang-query on every
source file to find forbidden code.
If we run it on all files without a cache, parsing ~1500 files takes 11 to 24 minutes on a
4-core runner, but most changes touch only a few files.
So this script utilizes a cache called ``clang-query-passed.txt``. Each line of the file is
a hash computed from a source file and its dependencies. It tells us which files (or more
accurately, which file contents) already pass clang-query and don't need to be run on them
again.

If the source file content changes, the hash will change accordingly, so we know to run
clang-query on it.

To ensure correctness, the hash is computed by other data as well so we don't miss a file:

- The file that stores the forbidden code query rule (``query.txt``).
- This script.
- The clang-query version.
- The C++ standard library version.
- The compile command (include folders, defines, flags) of each entry of the file.
- The content of the file and every header it includes directly and indirectly.

Quick lecture of clang-query:
clang-query parses C++ code syntax so we can look for forbidden code pattern like building
a std::filesystem::path from a std::string.
clang-query runs the clang compiler preprocessor to expand all the macros and replace "#include"
with header file content. It then parses the code into a syntax tree (AST).
It takes about 2-4 sec to do the above on a source file.
To run clang-query:

    clang-query -p SerialPrograms/bin -f query.txt file1.cpp file2.cpp ... fileN.cpp

-p: folder where compile_commands.json is. clang-query reads it to know each file’s compile
  settings (include folders, defines, flags).
-f: the query file, the rules to match forbidden code.
- file1.cpp ... input source file list.

Usage::

    clang-query-cached.py --build-dir SerialPrograms/bin --query query.txt \\
        --cache clang-query-passed.txt --output clang-query-output.txt

--build-dir: the build folder where compile_commands.json is. clang-query reads it to know each
  file's compile settings (include folders, defines, flags).
--query: the query file holding rules to match forbidden code, to pass to clang-query.
--cache: the cache file to store which files already pass clang-query. It will be read by the
  script if it exists and then overwritten with the new list.
--output: the clang-query output goes there. It prints the matched files.

The script exits with 1 if a clang-query process failed, else 0. It also exits with 0 when
clang-query finds forbidden code, so the caller must look for "Match #" in the output file.
"""

# Keep the type hints as text, so that hints like ``list[str]`` and ``str | None`` also work on
# Python 3.8 and 3.9. This script supports Python 3.8 and newer.
from __future__ import annotations

import argparse
import concurrent.futures
import glob
import hashlib
import json
import os
import re
import shlex
import subprocess
import sys
from dataclasses import asdict, dataclass, fields
from typing import Any


# Files per clang-query process. clang-query parses all of its files before it matches, and
# keeps every AST in memory. With 10 files, 4 processes use about 7 GB at most.
BATCH_SIZE = 10

# List at most this many of the files to check in the log.
MAX_FILES_TO_LIST = 10

# One word of a make rule: anything but whitespace, where a backslash before a space or a "#"
# makes that character part of the word.
MAKE_WORD = re.compile(r"(?:\\[ #]|\S)+")


@dataclass
class CompileCommand:
    """Represents one entry of ``compile_commands.json``

    Each entry stores how the build compiled one source file for one target.
    """

    # Root folder the compiler ran in.
    directory: str
    # Source file, e.g. ``/src/SerialPrograms/Source/Foo.cpp``. It is an absolute path, or a
    # path relative to the root folder `directory`.
    file: str
    # Compile command as one string. CMake wrote this field.
    command: str | None = None
    # Compile command as a list of words. Some other tools wrote this field instead.
    arguments: list[str] | None = None
    # Object file, e.g. ``CMakeFiles/SerialProgramsLib.dir/Source/Foo.cpp.o``. The compiler
    # writes the ``.d`` file next to it.
    output: str | None = None


def make_global_key(query_path: str) -> str:
    """Return a hash for those that are the same for all files: the query file, this script,
    and the versions of clang-query and the C++ standard library.

    This global hash will be combined with file-specific hash to create the final hash (the
    key) of every source file.
    """
    hasher = hashlib.sha256()
    for path in [query_path, __file__]:
        with open(path, "rb") as file:
            hasher.update(file.read())
    version = subprocess.run(["clang-query", "--version"], capture_output=True, check=True)
    hasher.update(version.stdout)
    # Hash the names of the installed GCC and C++ standard library version folders, e.g.
    # ``/usr/include/c++/14``.
    for pattern in ["/usr/include/c++/*", "/usr/lib/gcc/*/*"]:
        hasher.update("\n".join(sorted(glob.glob(pattern))).encode())
    return hasher.hexdigest()


def split_make_words(line: str) -> list[str]:
    """Split one line of a make rule into words.

    A backslash before a space or a ``#`` keeps it in the word, and ``$$`` is a ``$``. For
    example, ``a\\ b.h c.h`` gives ``["a b.h", "c.h"]``.
    """
    return [
        re.sub(r"\\([ #])", r"\1", word).replace("$$", "$")
        for word in MAKE_WORD.findall(line)
    ]


def read_dependency_file(path: str) -> list[str] | None:
    """Return the files listed in the make-style dependency file at `path`, or None if the
    file does not exist.

    GCC and clang write these files with ``-MD``. They list every file that the compiler
    opened, at any include depth. For example::

        CMakeFiles/SerialProgramsLib.dir/Source/Foo.cpp.o: \\
          /src/SerialPrograms/Source/Foo.cpp \\
          /src/SerialPrograms/Source/Foo.h

    gives ``["/src/SerialPrograms/Source/Foo.cpp", "/src/SerialPrograms/Source/Foo.h"]``.
    """
    try:
        # A path can have bytes that are not UTF-8. "surrogateescape" keeps them as they are.
        with open(path, encoding="utf-8", errors="surrogateescape") as file:
            text = file.read()
    except FileNotFoundError:
        return None
    dependencies: list[str] = []
    # A backslash at the end of a line continues the rule on the next line.
    for line in text.replace("\\\n", " ").splitlines():
        words = split_make_words(line)
        # The word that ends with ":" is the target. The words after it are its dependencies.
        for index, word in enumerate(words):
            if word.endswith(":"):
                dependencies.extend(words[index + 1:])
                break
    return dependencies


class DependencyHasher:
    """Acts as a dependency file hash cache so each dependency file is hashed only once.

    Each source file has about 1000 "#include" dependencies, but most of them are headers that
    hundreds of source files share. So all source files together have only a few thousand
    different dependencies.
    """

    def __init__(self) -> None:
        # (folder of the compile command, path in the ``.d`` file) -> normalized path
        self.dependency_to_path: dict[tuple[str, str], str] = {}
        # normalized path -> key part of the file, or None if the file cannot be read
        self.path_to_key_part: dict[str, str | None] = {}

    def key_part(self, directory: str, dependency: str) -> str | None:
        """Return the part of a file key for one dependency, or None if it cannot be read.

        Args:
            directory: folder that the compile command ran in.
            dependency: path from a ``.d`` file, relative to `directory` if not absolute.

        Returns:
            the normalized path and the SHA-256 of the contents, as ``"<path>\\0<sha256>\\0"``.
        """
        path = self.dependency_to_path.get((directory, dependency))
        if path is None:
            path = os.path.normpath(os.path.join(directory, dependency))
            self.dependency_to_path[(directory, dependency)] = path
        if path not in self.path_to_key_part:
            try:
                with open(path, "rb") as file:
                    file_hash = hashlib.sha256(file.read()).hexdigest()
                self.path_to_key_part[path] = f"{path}\0{file_hash}\0"
            except OSError:
                self.path_to_key_part[path] = None
        return self.path_to_key_part[path]


def make_file_key(
    global_key: str, entries: list[CompileCommand], dependency_hasher: DependencyHasher
) -> str | None:
    """Return the key of one source file, or None if the key cannot be made.

    The key covers all `entries` of the file, because a file that is built into more than
    one target has one entry per target, and clang-query checks all of them. A file without
    a key always runs. That happens when an entry has no ``output``, when its ``.d`` file is
    missing, or when a file listed there cannot be read.
    """
    key = hashlib.sha256(global_key.encode())
    # Sort the entries, so that the key is the same in any entry order.
    for entry in sorted(entries, key=lambda e: json.dumps(asdict(e), sort_keys=True)):
        directory = entry.directory
        arguments = entry.arguments or shlex.split(entry.command or "")
        # add compiler command (include folders, defines, flags) to hash
        key.update(json.dumps([directory, arguments]).encode())

        if entry.output is None:
            return None
        dependencies = read_dependency_file(os.path.join(directory, entry.output + ".d"))
        if dependencies is None:
            return None
        key_parts: set[str] = set()
        for dependency in dependencies:
            key_part = dependency_hasher.key_part(directory, dependency)
            if key_part is None:
                return None
            key_parts.add(key_part)
        # add each dependency file hash to hash
        key.update("".join(sorted(key_parts)).encode())
    return key.hexdigest()


def read_compile_commands(build_dir: str) -> dict[str, list[CompileCommand]]:
    """Return the entries of ``<build_dir>/compile_commands.json``, grouped by source file.

    For example, ``{"/src/Foo.cpp": [<entry for target A>, <entry for target B>]}``.
    """
    with open(os.path.join(build_dir, "compile_commands.json")) as file:
        json_entries: list[dict[str, Any]] = json.load(file)
    # Keep only the fields that `CompileCommand` has. Some tools write more fields.
    field_names = {field.name for field in fields(CompileCommand)}
    # file path -> list of compile_commands.json entries tied to this file
    file_to_entries: dict[str, list[CompileCommand]] = {}
    for json_entry in json_entries:
        entry = CompileCommand(**{k: v for k, v in json_entry.items() if k in field_names})
        file_to_entries.setdefault(entry.file, []).append(entry)
    return file_to_entries


def read_passed_keys(cache_path: str) -> set[str]:
    """Read cache keys from the cache file at `cache_path`.

    Each line of the cache file is a hash as a cache key representing if a file already
    passes clang-query before.

    If no such file, return an empty set.
    """
    if not os.path.exists(cache_path):
        return set()
    with open(cache_path) as file:
        return set(file.read().split())


def run_clang_query(build_dir: str, query_file_path: str, files: list[str]) -> tuple[int, str]:
    """Run one clang-query process on `files`.

    Returns:
        the exit code, and the output with stdout and stderr together.
    """
    result = subprocess.run(
        ["clang-query", "-p", build_dir, "-f", query_file_path, *files],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        # The output quotes source code, which can have bytes that are not UTF-8.
        errors="replace",
    )
    return result.returncode, result.stdout


def check_files(
    build_dir: str, query_file_path: str, files_to_check: list[str],
    file_to_key: dict[str, str | None], output_path: str,
) -> tuple[set[str], bool]:
    """Run clang-query on `files_to_check` in batches, one batch per core at a time, and write
    all of its output to `output_path`.

    Returns:
        the keys of the files in the batches that passed, and True if a clang-query process
        failed.
    """
    batches = [
        files_to_check[i:i + BATCH_SIZE] for i in range(0, len(files_to_check), BATCH_SIZE)
    ]
    passed_keys: set[str] = set()
    failed = False
    with open(output_path, "w") as output, \
            concurrent.futures.ThreadPoolExecutor(max_workers=os.cpu_count()) as pool:
        future_to_batch = {
            pool.submit(run_clang_query, build_dir, query_file_path, batch): batch
            for batch in batches
        }
        for future in concurrent.futures.as_completed(future_to_batch):
            batch = future_to_batch[future]
            exit_code, batch_output = future.result()
            output.write(batch_output)
            if exit_code != 0:
                failed = True
            # A batch with an error or a match adds no keys, so all of its files run again
            # next time.
            elif "Match #" not in batch_output:
                batch_keys = [file_to_key[source_file] for source_file in batch]
                passed_keys.update(key for key in batch_keys if key is not None)
    return passed_keys, failed


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--build-dir", required=True, help="Folder with compile_commands.json.")
    parser.add_argument("--query", required=True, help="clang-query script to run on each file.")
    parser.add_argument("--cache", required=True, help="List of keys of files that passed.")
    parser.add_argument("--output", required=True, help="Where to write the clang-query output.")
    args = parser.parse_args()

    file_to_entries: dict[str, list[CompileCommand]] = read_compile_commands(args.build_dir)
    passed_keys_before: set[str] = read_passed_keys(args.cache)

    # make a global key to hash the dependencies that are the same for all the source files:
    # clang-query version, C++ standard library versions, query file and this script.
    global_key: str = make_global_key(args.query)
    dependency_hasher = DependencyHasher()
    file_to_key: dict[str, str | None] = {}
    passed_keys_now: set[str] = set()
    files_to_check: list[str] = []
    for source_file, entries in sorted(file_to_entries.items()):
        # compute cache key for each file
        key = make_file_key(global_key, entries, dependency_hasher)
        file_to_key[source_file] = key
        if key is not None and key in passed_keys_before:
            passed_keys_now.add(key)
        else:
            files_to_check.append(source_file)

    files_without_key = sum(1 for key in file_to_key.values() if key is None)
    print(
        f"clang-query cache: {len(file_to_entries)} files, {len(passed_keys_now)} passed "
        f"before and are skipped, {len(files_to_check)} to check ({files_without_key} of them "
        f"failed to compute a key)."
    )
    for source_file in files_to_check[:MAX_FILES_TO_LIST]:
        no_key_note = "" if file_to_key[source_file] else " (no key)"
        print(f"    {os.path.relpath(source_file)}{no_key_note}")
    # Show the summary in the CI log now. Checking the files can take many minutes.
    sys.stdout.flush()

    # check the other files, and save the new list of files that passed
    newly_passed_keys, failed = check_files(
        args.build_dir, args.query, files_to_check, file_to_key, args.output
    )
    passed_keys_now |= newly_passed_keys
    # write the file cache back:
    with open(args.cache, "w") as file:
        file.write("".join(key + "\n" for key in sorted(passed_keys_now)))
    print(f"clang-query cache: {len(passed_keys_now)} files in the new list of files that passed.")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())

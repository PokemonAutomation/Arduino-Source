#!/usr/bin/env bash
set -e

# This script will get the diff between the current branch and the main branch. 
# It will then find all files that depend on these diff files, by 
# using clang-scan-deps to generate the dependency graph.
# The clang-query is then run on these diffed files (and their dependants).
# One weakness of this is that this diff only compares the current branch and main branch,
# and so is mainly helpful for PRs. But doesn't help when committing directly to main, or
# after merging the PR. This isn't insurmountable, but I felt it was getting too messy.


# to get this script to work in Github CI:
# Update cpp-ci-serial-programs-base.yml: 
# - Set fetch-depth to 0 to fetch all commits for the diff. with blob:none so you're not downloading all that data
# with:
# 	path: 'Arduino-Source'
# 	submodules: 'recursive'
# 	fetch-depth: 0
# 	filter: blob:none
#
# - when installing clang-tools, specify a version. e.g. clang-tools-18
# sudo apt install clang-tools-18 libopencv-dev
#
# - under run clang query: set working directory. run this script
# name: Run clang query
#      if: inputs.run-clang-query
#      working-directory: ./Arduino-Source
#      run : bash ./.github/scripts/clang-query.sh
# - this script should be placed within the folder .github/scripts
#
# other files to consider updating
# .gitattributes: *.sh text eol=lf


# to get this script to work locally in Windows:
# Open Git Bash. cd to root of the repo (Arduino-Source). Run the following command:
# sh .github/scripts/clang-query.sh


SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
TMP_DIR="$REPO_ROOT/.ci_tmp"
mkdir -p "$TMP_DIR"

# Define the cleanup function
cleanup() {
    echo "Cleaning up temporary files..."
    rm -rf "$TMP_DIR"
}

# Register the trap: run cleanup on EXIT, plus common signals like INT (Ctrl+C) or TERM
# trap cleanup EXIT INT TERM


cd "$REPO_ROOT"

# find path to compile_commands.json
if [ -f "$REPO_ROOT/SerialPrograms/bin/compile_commands.json" ]; then
    DB_PATH="$REPO_ROOT/SerialPrograms/bin/compile_commands.json"
elif [ -f "$REPO_ROOT/build/RelWithDebInfo/compile_commands.json" ]; then
    DB_PATH="$REPO_ROOT/build/RelWithDebInfo/compile_commands.json"
else
    echo "Error: compile_commands.json not found!"
    exit 1
fi


echo "Generating clang-scan-deps experimental-full > deps.json."

# in ubuntu, the command is clang-scan-deps-18. in Windows, it is clang-scan-deps
SCAN_DEPS=$(command -v clang-scan-deps-18 || command -v clang-scan-deps)

# Safety check: Exit if the tool isn't found
if [ -z "$SCAN_DEPS" ]; then
  echo "Error: clang-scan-deps (or version -18) not found in PATH."
  exit 1
fi


# filter compile_commands.json, to remove .rc files, since clang-scan-deps doesn't recognize this format
jq '[.[] | select(.file | endswith(".rc") | not)]' "$DB_PATH" > "$TMP_DIR/compile_commands_filtered.json"

# get dependency graph
"$SCAN_DEPS" -compilation-database "$TMP_DIR/compile_commands_filtered.json" -format experimental-full > "$TMP_DIR/deps.json"

# normalize slashes
# sed 's|\\\\|/|g' deps.json > normalized_deps.json
sed -i 's|\\\\|/|g' "$TMP_DIR/deps.json"

# check if deps.json has the expected keys
# because we are relying on clang-scan-deps experimental-full, where the names of the keys can change.
TU_KEY="translation-units"
CMD_KEY="commands"
DEPS="file-deps"
INPUT="input-file"

JQ_SCRIPT=$(cat << 'EOF'
	# 1. Access the target object
  (.[$TU][0][$CMD][0]) as $target
  
  # 2. Define the required keys

  | [$DEPS, $INPUT] as $required

  | (
	if .[$TU] == null then 
		"Missing: \($TU). Keys found at top-level: \(keys_unsorted)"
	elif .[$TU][0] == null then 
		"Missing: \($TU)[0]"
	elif .[$TU][0].[$CMD] == null then 
		"Missing: \($TU)[0].\($CMD). Keys found from \($TU)[0]: \(.[$TU][0] | keys_unsorted)"
	elif $target == null then 
		"Missing: \($TU)[0].[$CMD][0]"
	elif ($required | all(. as $req | $target | has($req)) | not) then
	  "Missing: One or more required keys \($required). Found: \($target | keys_unsorted)"
	# elif (.[$TU][0].[$CMD][0] | keys_unsorted | any(. == [$DEPS] or . == [$INPUT]) | not) then
	#     "Missing: both \($DEPS) and \($INPUT). Found: \(.[$TU][0][$CMD][0] | keys_unsorted)"
	else 
	  "All keys \($required) found in \($TU)[0].\($CMD)[0]"
	end
  ) as $result

  | if ($result | type == "string" and startswith("Missing:")) then 
		("\($result). The keys within the experimental-full format from clang-scan-deps can change over time. Please fix the CI to use the correct keys.") | halt_error
		else $result end
EOF
)

echo "Checking keys in deps.json."
jq -r "$JQ_SCRIPT" \
	--arg TU "$TU_KEY" \
	--arg CMD "$CMD_KEY" \
	--arg DEPS "$DEPS" \
	--arg INPUT "$INPUT" \
	"$TMP_DIR/deps.json"

echo "Generating changed_files.txt from git diff."

# git diff with relative paths
git diff --name-only origin/main...HEAD > "$TMP_DIR/changed_files.txt"

echo "Generating files_to_query.txt, based on changed_files.txt and deps.json."

# for each line in changed_files_unix.txt, search deps.json to find all their dependants
jq -r --rawfile mod "$TMP_DIR/changed_files.txt" \
	--arg TU "$TU_KEY" \
	--arg CMD "$CMD_KEY" \
	--arg DEPS "$DEPS" \
	--arg INPUT "$INPUT" '
  # 1. Clean the list of changed files
  ($mod | split("\n") | map(select(length > 0))) as $changes |
  
  # 2. Access the translation-units array
  [ .[$TU][] | .[$CMD][] | 
	select(
	  # 3. Check "file-deps" for matches
	  .[$DEPS][] | . as $dp | 
	  any($changes[]; . as $c | $dp | endswith($c))
	) | 
	# 4. Get the source file path
	.[$INPUT] 
  ] | unique[]
' "$TMP_DIR/deps.json" | tr -d '\r' > "$TMP_DIR/files_to_query.txt"



cat << 'EOF' > "$TMP_DIR/query.txt"
set output dump
match invocation(
  isExpansionInFileMatching("SerialPrograms/"),
  hasDeclaration(cxxConstructorDecl(ofClass(hasName("std::filesystem::path")))),
  hasArgument(0, hasType(asString("std::string")))
)
EOF

echo "Running clang-query."

# files=$(jq -r '.[].file' "$DB_PATH")
DB_DIR=$(dirname "$DB_PATH")
#echo "$files" | xargs --max-args=150 clang-query -p "$DB_DIR" -f "$TMP_DIR/query.txt" >> output.txt

# jq -r '.[].file' "$DB_PATH" | sed 's/\\/\//g' | tr -d '\r'  | xargs -d '\n' --max-args=150 clang-query -p "$DB_DIR" -f "$TMP_DIR/query.txt" -- -Wno-unused-command-line-argument >> "$TMP_DIR/output.txt"

# this works
# jq -r '.[].file' "$DB_PATH" | sed 's/\\/\//g' | tr -d '\r'  | xargs -d '\n' --max-args=150 clang-query -p "$DB_DIR" -f "$TMP_DIR/query.txt" >> "$TMP_DIR/output.txt"
# jq -r '.[].file' "$DB_PATH" | tr -d '\r'  | xargs -d '\n' --max-args=150 clang-query -p "$DB_DIR" -f "$TMP_DIR/query.txt" >> "$TMP_DIR/output.txt"

# also works
# jq -r '.[].file' "$DB_PATH" | tr -d '\r' | xargs -d '\n' --max-args=150 \
# clang-query -p "$DB_DIR" \
# --extra-arg="-Wno-unused-command-line-argument" \
# -f "$TMP_DIR/query.txt" >> "$TMP_DIR/output.txt"

# also works
# jq -r '.[].file' "$DB_PATH" | tr -d '\r' | sed 's|\\|/|g' | \
# 	xargs -d '\n' --max-args=150 \
# 	clang-query -p "$DB_DIR" \
# 	--extra-arg="-Wno-unused-command-line-argument" \
# 	--extra-arg="-Wno-unused-function" \
# 	-f "$TMP_DIR/query.txt" >> "$TMP_DIR/output.txt"

# in ubuntu, the command is clang-query-18. in Windows, it is clang-query
CLANG_QUERY=$(command -v clang-query-18 || command -v clang-query)

if [ -z "$CLANG_QUERY" ]; then
    echo "Error: clang-query (or version -18) not found!"
    exit 1
fi

ONLY_CHECK_CHANGED_FILES=true
if [ "$ONLY_CHECK_CHANGED_FILES" = "true" ]; then
    LIST_FILE="$TMP_DIR/files_to_query.txt"
else # check all files
	LIST_FILE="$TMP_DIR/file_list.txt"
    jq -r '.[].file' "$DB_PATH" | tr -d '\r' | sed 's|\\|/|g' > "$LIST_FILE"
fi

> "$TMP_DIR/output.txt"

# Run clang-query using the list file
# check if LIST_FILE has any data to analyze
if [ ! -s "$LIST_FILE" ]; then
    echo "No files found to analyze. Skipping Clang-Query."
else
    xargs -d '\n' -a "$LIST_FILE" --max-args=150 \
		"$CLANG_QUERY" -p "$DB_DIR" \
		--extra-arg="-Wno-unused-command-line-argument" \
		--extra-arg="-Wno-unused-function" \
		-f "$TMP_DIR/query.txt" >> "$TMP_DIR/output.txt"
fi



cat "$TMP_DIR/output.txt"
if grep --silent "Match #" "$TMP_DIR/output.txt"; then
  echo "::error Forbidden std::filesystem::path construction detected!"
  exit 1
fi
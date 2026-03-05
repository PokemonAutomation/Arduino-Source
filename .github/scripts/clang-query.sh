#!/usr/bin/env bash
set -e

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
trap cleanup EXIT INT TERM


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


# git diff with relative paths
git diff --name-only origin/main...HEAD > "$TMP_DIR/changed_files.txt"

echo "Generating clang-scan-deps experimental-full > deps.json."

# filter compile_commands.json, to remove .rc files, since clang-scan-deps doesn't recognize this format
jq '[.[] | select(.file | endswith(".rc") | not)]' "$DB_PATH" > "$TMP_DIR/compile_commands_filtered.json"

# get dependency graph
clang-scan-deps -compilation-database "$TMP_DIR/compile_commands_filtered.json" -format experimental-full > "$TMP_DIR/deps.json"

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

echo "Generating files_to_query.txt."
# for each line in changed_files.txt, search deps.json to find all their dependants
# 
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
' "$TMP_DIR/deps.json" > "$TMP_DIR/files_to_query.txt"

cat << 'EOF' > "$TMP_DIR/query.txt"
set output dump
match invocation(
  isExpansionInFileMatching("SerialPrograms/"),
  hasDeclaration(cxxConstructorDecl(ofClass(hasName("std::filesystem::path")))),
  hasArgument(0, hasType(asString("std::string")))
)
EOF

echo "Running clang-query."

files=$(jq -r '.[].file' "$DB_PATH")
DB_DIR=$(dirname "$DB_PATH")
echo "$files" | xargs --max-args=150 clang-query -p "$DB_DIR" -f "$TMP_DIR/query.txt" >> output.txt
cat output.txt
if grep --silent "Match #" output.txt; then
  echo "::error Forbidden std::filesystem::path construction detected!"
  exit 1
fi
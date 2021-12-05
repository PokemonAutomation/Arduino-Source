#!/usr/bin/env bash

# change directory to library location
cd "$(dirname "$0")"

# define the MCU, which is the board type

if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="mac"
    FZF=$(which fzf)

    if [[ "$FZF" == "" ]]; then
        boxed_msg "${RED}ERROR: FZF is not detected on your system. Please install it through Homebrew ${RESET}"
        exit 1
    fi

elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="linux"
    FZF="../Scripts/fzf-${PLATFORM}"
else
    boxed_msg "${RED}${OSTYPE} is not a recognized platform. Please run only on Mac or Linux${RESET}"
    exit 1
fi

BOARD=$1

function run() {
    if [[ -z "$BOARD" ]]; then
        board_prompt
    fi

    # if the boards were given by integer, we can update them here


    # say what we're building for convenience
    echo "Ready to build for board: $BOARD"
    echo ""

    # and send off to build
    bash ../Scripts/BuildAllUnix.sh "$BOARD" #1> /dev/null

    # let them know we finished
    echo -e "\033[1mFinished building hex files for board: $BOARD \033[0m"
    echo "Please make sure the hex files were properly built or updated."
    echo "You can now close this window if you wish."
    echo ""
    # really done
}

function board_prompt() {
    BOARD=$(cat ../Scripts/Boards.txt | $FZF --height=15% --prompt="Choose your Board: ")
}

run

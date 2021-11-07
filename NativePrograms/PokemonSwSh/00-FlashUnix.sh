#!/usr/bin/env bash

# change directory to library location
cd "$(dirname "$0")" || exit

# Parse Command line arguments for MCU, Program and Device Path
BOARD=$1
PROGRAM=$2
DEVICE_PATH=$3


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

SUDO=""
BUILD_HEX=""
HEXFILE=""
PROGRAM_COMMAND="program_avrdude ${MCU} ${HEXFILE}" # Default to using avrdude
BOARDS=(ArduinoUnoR3 ProMicro Teensy2 TeensyPP2)

function run() {

    configure

    if [[ $BUILD_HEX == "y" ]]; then
        build_hex
    fi

    boxed_msg "Flashing ${PROGRAM}.hex ..."
    $PROGRAM_COMMAND

    boxed_msg "Finished flashing ${HEXFILE} to ${BOARD}!"
}

function configure() {
    boxed_msg "Configuration"

    [[ -f "${HOME}/.paconfig" ]] && source "${HOME}/.paconfig"
    [[ "$EUID" -ne 0 ]] && SUDO="sudo"
    [[ -z "${BOARD}" ]] && board_prompt || note "Using Preconfigured BOARD [${BOARD}]"
    [[ -z "${PROGRAM}" ]] && prog_prompt
    
    if [[ ! -f ${HEXFILE} ]]; then
        question "Build ${PROGRAM}? ${WHITE}[Y/n]${RESET}" "DOBUILD"
        DOBUILD="${DOBUILD:-y}"
        if [ "$DOBUILD" == "y" ]; then
            BUILD_HEX="y"
        else
            logmsg "Nothing to flash, exiting.."
            exit
        fi
    fi

    # Logic below to determine if we need to us dfu-programmer or not
    if [[ $PLATFORM == "mac" ]] && [[ ${MCU} == "atmega16u2" ]]; then
        which dfu-programmer >/dev/null 2>&1
        if [[ $? -eq 0 ]]; then
            question "Mac platform detected, UNO R3 configured. Connect your device and enter DFU mode, then press any key..." "use_dfu"
            PROGRAM_COMMAND="program_dfu ${MCU} ${HEXFILE}"
        else
            logmsg "dfu-programmer not installed, run \"brew install dfu-programmer\" then rerun this script"
	    exit 1
        fi
    fi
}

function build_hex {
        boxed_msg "Build ${PROGRAM}.c"

        note "Cleaning Build Environment"
        ./00-CleanupUnix.sh >/dev/null


        note "Running Build Script"
        sh ../Scripts/BuildOneUnix.sh "${BOARD}" "${PROGRAM}" > /dev/null 2>&1

        retVal=$?
        if [ $retVal -ne 0 ]; then
            boxed_msg "${RED}ERROR - CRITICAL: An error occured while building.\nPlease check logs and perhaps run the cleanup script before trying again.${RESET}"
            exit 1
        else 
            note "${PROGRAM}.hex successfully built!"
        fi
    
}

function detect_device() {
    local -r delay='0.75'
    local spinstr='\|/-'
    local temp
    while true; do
        temp="${spinstr#?}"
        printf "${YELLOW}Waiting for device... [%c]  ${RESET}" "${spinstr}"
        spinstr=${temp}${spinstr%"${temp}"}
        # shellcheck disable=SC2010
        found_device_path="$(ls /dev/tty* | grep -i 'acm\|usb')"
        if [[ -n "$found_device_path" ]]; then
            DEVICE_PATH="${found_device_path}"
            break
        fi
        sleep "${delay}"
        printf "%0.s\b" {1..28}
    done
    printf "    \b\b\b\b\n"
    boxed_msg "Device connected at [${found_device_path}]" "short"
}

function program_dfu {
    # start with clearing the BOARD
    ${SUDO} dfu-programmer "${MCU}" erase || true
    # then flash the hex to the BOARD
    ${SUDO} dfu-programmer "${MCU}" flash "${HEXFILE}"
    if [[ $? -ne 0 ]]; then
        boxed_msg "${RED}Flash Error: Check the output above for more info.${RESET}"
        exit 1
    fi
    # then reset the BOARD
    ${SUDO} dfu-programmer "${MCU}" reset
    if [[ $? -ne 0 ]]; then
        boxed_msg "${RED}Device Reset Error: Check the output above for more info.${RESET}"
        exit 1
    fi
}

function program_avrdude {
    if [[ $AUTODETECT_DEVICE == "y" ]]; then
        note "Autodetect enabled..."
        detect_device
    elif [[ -z $DEVICE_PATH ]] && [[ -z $autodetect_device ]]; then
        question "Would you like to attempt auto detecting your device? ${WHITE}[y/n]${RESET}" "autodetect_device"
        autodetect_device="${autodetect_device:-y}"
        if [[ $autodetect_device == "n" ]]; then
            question "Please enter the path to your device ${WHITE}(ex. /dev/ttyACM0)${RESET}" "DEVICE_PATH"        
        elif [[ $autodetect_device == "y" ]]; then
            detect_device 
        else
            note "Invalid input, defaulting to autodetection"
            detect_device
        fi
    fi
    
    note "Attempting to flash ${WHITE}${PROGRAM}.hex${RESET} to ${WHITE}${DEVICE_PATH}${RESET} now"
    ${SUDO} avrdude -q -p "${MCU}" -P "${DEVICE_PATH}" -c avr109 -U flash:w:"${HEXFILE}"
}

function boxed_msg() {
    msg=$1
    len=$2
    [[ $len == short ]] && div="\n===============\n" || div="\n=================================\n"
    echo -e "${div}${GREEN}${msg}${RESET}${div}"
}

function question() {
    echo -e "\n${YELLOW}${1}: ${RESET}"
    read -r "$2"
}

function logmsg {
    msg=$1
    echo -e "${LBLUE}${msg}${RESET}"
}

function note() {
    msg=$1
    div='** '
    echo -e "${YELLOW}${div}${msg}${RESET}"
}

function board_prompt() {
    BOARD=$(cat ../Scripts/Boards.txt | $FZF --height=15% --prompt="Choose your Board: ")
    case $BOARD in
        "ArduinoUnoR3")
            MCU=atmega16u2
            ;;
        "ArduinoLeonardo")
            MCU=atmega32u4
            ;;
        "ProMicro") 
            MCU=atmega32u4
            ;;
        "Teensy2") 
            MCU=atmega32u4
            ;;
        "TeensyPP2")
            MCU=at90usb1286
            ;;
        *)
            note "Invalid Board"
            exit 1
    esac
}

function prog_prompt() {
    PROGRAM=$(ls -1 *.c 2> /dev/null | sed 's/\.c//g' | $FZF --height=15% --prompt="Program to Flash: " )
    HEXFILE="${PROGRAM}.hex"
}

## Color Helpers
RED=$'\e[1;31m'
YELLOW=$'\e[1;33m'
GREEN=$'\e[1;32m'
LBLUE=$'\e[1;34m'
WHITE=$'\e[1;37m'
RESET=$'\e[0m'

# LEZZGOOO!
run


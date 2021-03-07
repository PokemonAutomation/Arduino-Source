#!/usr/bin/env bash
# NOTE: this script is a helper script that *should not be called directly*
# please use the scripts provided in the root of this directory

# get the MCU from the first argument
# define the MCU 
board=$1

if [ -z "$board" ]; then
      read -p 'No MCU Type given, please enter: ' board
fi

# get the program from the second argument
program=$2

if [ -z "$program" ]; then
      read -p 'No program given, please enter: ' program
fi

# delete the existing hex program if it exists
if [ -f "$program.hex" ]; then
    rm $program.hex
fi

# Allows running a cleanup. Adding for the MacOS port of the GUI
# while maintaining compatibility with the CLI scripts

gui=$3
if [[ $gui == "gui" ]]; then 
      obj_files=obj/*.o
      if [[ ! -z $obj_files ]]; then
            echo "Existing build found. Checking MCU compatibility."
            if [[ ! -f "obj/build-$MCU" ]]; then
                  echo "Incompatible MCU, cleaning..."
                  bash 00-CleanupUnix.sh
            else
                  echo "Existing build is compatible."
            fi
      else
            echo "No build found, continuing..."
      fi
fi

# then call make
make BOARD_TYPE="$board" TARGET="$program"

# then we can clean up the map, eep, bin, lss, sym, tmp
rm $program.map > /dev/null 2>&1
rm $program.eep > /dev/null 2>&1
rm $program.bin > /dev/null 2>&1
rm $program.lss > /dev/null 2>&1
rm $program.sym > /dev/null 2>&1
rm $program.elf > /dev/null 2>&1

# Also putting this behind the gui-wall
if [[ $gui == "gui" ]]; then 
      mv "$program.hex" "../$program-$board.hex"
fi
#!/usr/bin/env bash
# This bash script builds all of the different hex files for the programs included
# NOTE: this script is a helper script that *should not be called directly*
# please use the scripts provided in the root of this directory

board=$1

declare -a PROGRAMS=()

#PROGRAMS+=("FriendDelete")
#mapfile -t PROGRAMS < ProgramList.txt
while IFS="$IFS"$'\r' read -r line; do
    if [ -n "$line" ]; then
        if [ -f "$line.c" ]; then
            PROGRAMS+=("$line")
        fi
    fi
done < ProgramList.txt


echo "Now Checking the make command..."
echo ""

if ! command -v make -v &> /dev/null; then
    echo "make could not be found. Please install make"
    exit
fi

echo "Starting the build... This may take a while if your computer is slow"
echo ""


# check the obj directory and make it if it doesn't exist
if [ ! -d obj/ ]; then
    mkdir obj/
fi

if [ ! -d log/ ]; then
    mkdir log/
fi

# build one of them first to establish the library
for p in "${PROGRAMS[@]}"; do
    first="$p"
    echo "$first"

    echo "sh ../Scripts/BuildOneUnix.sh $board $p"
    sh ../Scripts/BuildOneUnix.sh $board $p

    retVal=$?
    if [ $retVal -ne 0 ]; then
        echo ""
        echo "WARNING: Something went wrong building $first" 1>&2
        echo "WARNING: The most likely cause is remnants of builds from different boards." 1>&2
        rm obj/*.d obj/*.o
        echo "WARNING: Attempting build again..." 1>&2

        sh ../Scripts/BuildOneUnix.sh $board $p
        retVal=$?

        # one last check, if it errors again, we'll exit
        if [ $retVal -ne 0 ]; then
            echo "ERROR - CRITICAL: Another error occured, so it was something else. Please run 'bash !CleanupUnix.sh' and try again."
            echo "IF THIS PERSISTS: PLEASE LOOK AT THE LOG FILE FOR $first IN logs/"
        else
            echo "The rebuild was successful! Continuing other programs!"
        fi
    fi

    # break out of the first value
    break
done

# then we can actually run everything in the programs
for p in "${PROGRAMS[@]}"; do
    if [ "$p" != "$first" ]; then
        # send it off and pipe it to a log file
        sh ../Scripts/BuildOneUnix.sh $board $p 2>&1 | tee "log/$p.log" &
    fi
done

FAIL=0

# run a check to make sure that we're done with each program
# this waiting ensures that we're finishing before we move on
for job in `jobs -p`
do
    wait $job || let "FAIL+=1"
done

echo "$FAIL jobs failed"

echo ""
echo "Done building! Double-check that the .hex files have been made or updated."

echo "You can close this window now!"

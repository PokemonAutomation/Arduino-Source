# Extract the Discord Social SDK from `DISCORD_ZIP` into `DISCORD_DIR` when DISCORD_DIR does
# not exist or is older than DISCORD_ZIP.
#
function(extract_discord_sdk DISCORD_ZIP DISCORD_DIR)
    if (NOT EXISTS "${DISCORD_ZIP}")
        message(FATAL_ERROR "Discord SDK archive is missing: ${DISCORD_ZIP}")
    endif()

    # Initialize the string for logging the reason for extracing the zip.
    set(EXTRACT_REASON "")

    # Directories are excluded by GLOB_RECURSE, so this is the list of real files.
    file(GLOB_RECURSE EXTRACTED_FILES "${DISCORD_DIR}/*")

    if (NOT EXISTS "${DISCORD_DIR}")
        set(EXTRACT_REASON "not extracted yet")
    elseif (NOT EXTRACTED_FILES)
        set(EXTRACT_REASON "extracted directory is empty")
    else()
        # Compare each file's timestamp against the zip file.
        file(TIMESTAMP "${DISCORD_ZIP}" ZIP_TIME "%s" UTC)
        set(OLDEST_TIME "")
        foreach (EXTRACTED_FILE IN LISTS EXTRACTED_FILES)
            file(TIMESTAMP "${EXTRACTED_FILE}" FILE_TIME "%s" UTC)
            # file(TIMESTAMP) yields "" for a file it cannot stat. Treat that as requiring
            # extraction to cover the case of broken files.
            if (FILE_TIME STREQUAL "")
                set(OLDEST_TIME "")
                break()
            endif()
            if (OLDEST_TIME STREQUAL "" OR FILE_TIME LESS OLDEST_TIME)
                set(OLDEST_TIME "${FILE_TIME}")
            endif()
        endforeach()
        if (OLDEST_TIME STREQUAL "")
            set(EXTRACT_REASON "extracted files are unreadable")
        elseif (ZIP_TIME STREQUAL "")
            # Unknown zip timestamp: extract anyway
            set(EXTRACT_REASON "zip timestamp is unreadable")
        elseif (ZIP_TIME GREATER_EQUAL OLDEST_TIME)
            # GREATER_EQUAL, not GREATER: file(TIMESTAMP) has whole-second resolution, so a
            # zip replaced within the same second as the extraction is indistinguishable from
            # one that predates it.
            set(EXTRACT_REASON "zip is newer than the extracted files")
        endif()
    endif()

    if (EXTRACT_REASON STREQUAL "")
        # No need to extract.
        return()
    endif()

    message(STATUS "Discord SDK ${EXTRACT_REASON}, extracting from ${DISCORD_ZIP}...")

    # Delete first so files dropped or renamed by the new SDK do not survive as leftovers.
    file(REMOVE_RECURSE "${DISCORD_DIR}")
    get_filename_component(DISCORD_PARENT_DIR "${DISCORD_DIR}" DIRECTORY)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xf "${DISCORD_ZIP}"
        WORKING_DIRECTORY "${DISCORD_PARENT_DIR}"
        RESULT_VARIABLE EXTRACT_RESULT
    )
    if (NOT EXTRACT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to extract ${DISCORD_ZIP} (error: ${EXTRACT_RESULT})")
    endif()

    file(GLOB_RECURSE EXTRACTED_FILES "${DISCORD_DIR}/*")
    if (NOT EXTRACTED_FILES)
        message(FATAL_ERROR "Extracting ${DISCORD_ZIP} produced no files in ${DISCORD_DIR}")
    endif()
    # Touch the files to ensure their timestamps are newer to avoid redundant extraction.
    file(TOUCH ${EXTRACTED_FILES})
endfunction()

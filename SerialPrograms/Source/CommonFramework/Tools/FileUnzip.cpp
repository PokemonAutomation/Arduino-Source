/*  File Unzip
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#endif
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

// #include "miniz-cpp/zip_file.hpp"

#include "miniz-3.1.1/miniz.h"
// #include "3rdParty/miniz-3.1.1/miniz.h"
#include "Common/Cpp/Exceptions.h"
#include "FileUnzip.h"
#include <filesystem>
#include <fstream>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

    namespace fs = std::filesystem;

    struct ProgressData {
        std::ofstream* out_file;
        uint64_t total_bytes;
        uint64_t processed_bytes;
        int last_percentage;
    };

    // Callback triggered for every chunk of decompressed data
    // pOpaque is an opaque pointer that actually represents ProgressData
    size_t write_callback(void* pOpaque, mz_uint64 file_ofs, const void* pBuf, size_t n) {
        ProgressData* data = static_cast<ProgressData*>(pOpaque);

        // 1. Check if we actually need to seek
        // tellp() returns the current 'put' position.  get the current position of the write pointer in an output stream.
        if (static_cast<mz_uint64>(data->out_file->tellp()) != file_ofs) {
            data->out_file->seekp(file_ofs);
        }
            
        // Write chunk to disk
        data->out_file->write(static_cast<const char*>(pBuf), n);
        
        // Update and display progress
        data->processed_bytes += n;
        double percent = (double)data->processed_bytes / data->total_bytes * 100.0;
        int current_percent = static_cast<int>(percent);

        // Only print if the integer value has changed
        if (current_percent > data->last_percentage) {
            data->last_percentage = current_percent;
            std::cout << "\rProgress: " << current_percent << "% (" 
                    << data->processed_bytes << "/" << data->total_bytes << " bytes)" << endl;
        }
                
        return n;
    }

    // ensure that entry_name is inside target_dir, to prevent path traversal attacks.
    bool is_safe(const std::string& target_dir, const std::string& entry_name) {
        try {
            // 1. Get absolute, normalized paths
            // handles symlinks. and resolves .. and . components. throws error if path doesn't exist
            fs::path base = fs::canonical(target_dir);
            // confirms that base is a directory, and not a file
            if (!fs::is_directory(base)) return false;
            
            // resolves .. and . components and returns an absolute path without requiring the final path to exist.
            fs::path target = fs::weakly_canonical(base / entry_name);

            // cout << base << endl;
            // cout << target << endl;

            // 2. Use lexically_relative to find the path from base to target
            fs::path rel = target.lexically_relative(base);

            // 3. Validation:
            // - If rel is empty, they are likely different roots
            // - If rel starts with "..", it escaped the base
            // - If rel is ".", it IS the base directory (usually safe)
            if (rel.empty() || *rel.begin() == "..") {
                return false;
            }

            return true;
        } catch (...) {
            cout << "target_dir path doesn't exist." << endl;
            return false;
        }
    }

    void unzip_file(const char* zip_path, const char* target_dir) {
        fs::path p{zip_path};
        if (!fs::exists(p)) {
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "unzip_all: Attempted to unzip a file that doesn't exist.");
        } 

        mz_zip_archive zip_archive;
        memset(&zip_archive, 0, sizeof(zip_archive));

        // Opens the ZIP file at zip_path
        // zip_archive holds the state and metadata of the ZIP archive.
        if (!mz_zip_reader_init_file(&zip_archive, zip_path, 0)){
            cout << "failed to run mz_zip_reader_init_file" << endl;
            cout << "mz_zip_error: " << mz_zip_get_last_error(&zip_archive) << endl;
            return;
        } 

        // Get total number of files in the archive
        int num_files = (int)mz_zip_reader_get_num_files(&zip_archive);

        // calculate total uncompressed size
        uint64_t total_uncompressed_size = 0;
        for (int i = 0; i < num_files; i++) {
            mz_zip_archive_file_stat file_stat; // holds info on the specific file

            // fills file_stat with the data for the current index
            if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) continue;

            cout << std::to_string(file_stat.m_uncomp_size) << endl;
            total_uncompressed_size += file_stat.m_uncomp_size;
        }

        uint64_t total_processed_bytes = 0;
        for (int i = 0; i < num_files; i++) {
            mz_zip_archive_file_stat file_stat; // holds info on the specific file

            // fills file_stat with the data for the current index
            if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) continue;

            // Checks if the current entry is a folder. Miniz treats folders as entries; 
            // this code skips them to avoid trying to "write" a folder as if it were a file.
            if (mz_zip_reader_is_file_a_directory(&zip_archive, i)) {
                continue;
            }

            // Construct your output path (e.g., target_dir + file_stat.m_filename)
            std::string out_path = std::string(target_dir) + "/" + file_stat.m_filename;
            fs::path const parent_dir{fs::path(out_path).parent_path()};

            // Create the entire directory, including intermediate directories for this file
            std::error_code ec{};
            fs::create_directories(parent_dir, ec);
            if (ec) {
                std::cerr << "Error creating " << parent_dir << ": " << ec.message() << std::endl;
                ec.clear(); 
            }

            // ensure that entry_name is inside target_dir. to prevent path traversal attacks.
            if (!is_safe(target_dir, file_stat.m_filename)){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "unzip_all: Attempted to unzip a file that was trying to leave its base directory. This is a security risk.");
            }
            
            std::ofstream out_file(out_path, std::ios::binary); // std::ios::binary is to prevent line-ending conversions.
            ProgressData progress = { &out_file, total_uncompressed_size, total_processed_bytes, -1 };

            // Extract using the callback
            // decompresses the file in chunks and repeatedly calls write_callback to save those chunks to the disk via the out_file
            mz_zip_reader_extract_to_callback(&zip_archive, i, write_callback, &progress, 0);
            std::cout << "\nFinished: " << file_stat.m_filename << std::endl;
            total_processed_bytes += file_stat.m_uncomp_size;
        }

        mz_zip_reader_end(&zip_archive);
    }

    // void unzip_file(const std::string& zip_path, const std::string& output_dir) {
    //     cout << "try to unzip the file." << endl;
    //     miniz_cpp::zip_file archive(zip_path);

    //     // create folder structure before extracting.
    //     // since miniz-cpp does not automatically create subdirectories if they exist within the zip archive
    //     std::vector<miniz_cpp::zip_info> const info_list = archive.infolist();
    //     auto const current_directory = std::filesystem::current_path();
    //     std::error_code ec{};
    //     for(miniz_cpp::zip_info const & info: info_list ){
    //         std::filesystem::path const p{(std::filesystem::path(output_dir) / info.filename).parent_path()};
    //         // Create the entire directory tree for this file
    //         std::filesystem::create_directories(p, ec);
            
    //         if (ec) {
    //             std::cerr << "Error creating " << p << ": " << ec.message() << std::endl;
    //             ec.clear(); 
    //         }
    //     }

    //     // Extract all files to the specified path
    //     archive.extractall(output_dir);

    //     cout << "done unzipping the file." << endl;

    // }


}

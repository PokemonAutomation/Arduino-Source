/*  File Unzip
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */



// #define _LARGEFILE64_SOURCE 1

#include "miniz-cpp/zip_file.hpp"
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
        std::ofstream* outFile;
        uint64_t totalBytes;
        uint64_t processedBytes;
    };

    // Callback triggered for every chunk of decompressed data
    // pOpaque is an opaque pointer that actually represents ProgressData
    size_t write_callback(void* pOpaque, mz_uint64 file_ofs, const void* pBuf, size_t n) {
        ProgressData* data = static_cast<ProgressData*>(pOpaque);

        // 1. Check if we actually need to seek
        // tellp() returns the current 'put' position.  get the current position of the write pointer in an output stream.
        if (static_cast<mz_uint64>(data->outFile->tellp()) != file_ofs) {
            data->outFile->seekp(file_ofs);
        }
            
        // Write chunk to disk
        data->outFile->write(static_cast<const char*>(pBuf), n);
        
        // Update and display progress
        data->processedBytes += n;
        double percent = (double)data->processedBytes / data->totalBytes * 100.0;
        std::cout << "\rProgress: " << percent << "% (" 
                << data->processedBytes << "/" << data->totalBytes << " bytes)" << std::flush;
                
        return n;
    }

    // void unzip_with_progress(const char* zip_path, const char* file_in_zip, const char* out_path) {
    //     mz_zip_archive zip_archive;
    //     memset(&zip_archive, 0, sizeof(zip_archive));

    //     if (!mz_zip_reader_init_file(&zip_archive, zip_path, 0)) return;

    //     int file_index = mz_zip_reader_locate_file(&zip_archive, file_in_zip, nullptr, 0);
    //     mz_zip_archive_file_stat stat;
    //     mz_zip_reader_file_stat(&zip_archive, file_index, &stat);

    //     std::ofstream outFile(out_path, std::ios::binary);
    //     ProgressData progress = { &outFile, (size_t)stat.m_uncomp_size, 0 };

    //     // Extract using the callback for byte-level tracking
    //     mz_zip_reader_extract_to_callback(&zip_archive, file_index, write_callback, &progress, 0);

    //     mz_zip_reader_end(&zip_archive);
    //     std::cout << "\nExtraction complete!" << std::endl;
    // }


    // ensure that entry_name is inside target_dir.
    // to prevent path traversal attacks.
    bool is_safe(const std::string& target_dir, const std::string& entry_name) {
        fs::path base = fs::absolute(target_dir);
        // Join paths and resolve all '../'. Unlike canonical, this works even if the file doesn't exist yet.
        fs::path final_path = fs::weakly_canonical(base / entry_name);
        
        // Check if the final path is still within the base directory
        // this compares base and final_path, trying to find the first mismatch
        // if the final_path is a descendent directory of base, 
        // then the position of the first mismatch should be the end of base.
        auto [root, relative] = std::mismatch(base.begin(), base.end(), final_path.begin());
        return root == base.end();
    }

    void unzip_all(const char* zip_path, const char* target_dir) {
        mz_zip_archive zip_archive;
        memset(&zip_archive, 0, sizeof(zip_archive));

        // Opens the ZIP file at zip_path
        // zip_archive holds the state and metadata of the ZIP archive.
        if (!mz_zip_reader_init_file(&zip_archive, zip_path, 0)) return;

        // Get total number of files in the archive
        int num_files = (int)mz_zip_reader_get_num_files(&zip_archive);

        // calculate total uncompressed size
        uint64_t total_uncompressed_size = 0;
        for (int i = 0; i < num_files; i++) {
            mz_zip_archive_file_stat file_stat; // holds info on the specific file

            // fills file_stat with the data for the current index
            if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) continue;

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

            // ensure that entry_name is inside target_dir. to prevent path traversal attacks.
            if (!is_safe(target_dir, file_stat.m_filename)){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "unzip_all: Attempted to unzip a file that was trying to leave its base directory. This is a security risk.");
            }

            // Create the entire directory, including intermediate directories for this file
            std::error_code ec{};
            fs::create_directories(parent_dir, ec);
            if (ec) {
                std::cerr << "Error creating " << parent_dir << ": " << ec.message() << std::endl;
                ec.clear(); 
            }
            
            std::ofstream outFile(out_path, std::ios::binary); // std::ios::binary is to prevent line-ending conversions.
            ProgressData progress = { &outFile, total_uncompressed_size, total_processed_bytes };

            // Extract using the callback
            // decompresses the file in chunks and repeatedly calls write_callback to save those chunks to the disk via the outFile
            mz_zip_reader_extract_to_callback(&zip_archive, i, write_callback, &progress, 0);
            std::cout << "\nFinished: " << file_stat.m_filename << std::endl;
            total_processed_bytes += file_stat.m_uncomp_size;
        }

        mz_zip_reader_end(&zip_archive);
    }

    void unzip_file(const std::string& zip_path, const std::string& output_dir) {
        cout << "try to unzip the file." << endl;
        miniz_cpp::zip_file archive(zip_path);

        // create folder structure before extracting.
        // since miniz-cpp does not automatically create subdirectories if they exist within the zip archive
        std::vector<miniz_cpp::zip_info> const info_list = archive.infolist();
        auto const current_directory = std::filesystem::current_path();
        std::error_code ec{};
        for(miniz_cpp::zip_info const & info: info_list ){
            std::filesystem::path const p{(std::filesystem::path(output_dir) / info.filename).parent_path()};
            // Create the entire directory tree for this file
            std::filesystem::create_directories(p, ec);
            
            if (ec) {
                std::cerr << "Error creating " << p << ": " << ec.message() << std::endl;
                ec.clear(); 
            }
        }

        // Extract all files to the specified path
        archive.extractall(output_dir);

        cout << "done unzipping the file." << endl;

    }


}

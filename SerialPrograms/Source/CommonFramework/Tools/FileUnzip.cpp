/*  File Unzip
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "miniz-cpp/zip_file.hpp"
#include "FileUnzip.h"
#include <filesystem>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

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

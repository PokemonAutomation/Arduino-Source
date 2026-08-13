/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <unistd.h>
#include <limits.h>
#include <vector>
#include <sstream>
#include <fstream>
#include "Common/Cpp/Strings/StringTools.h"
#include "FilePath.h"
#include "Filesystem.h"

namespace PokemonAutomation{
namespace Filesystem{


Path application_binary_path(){
    char buffer[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buffer, PATH_MAX);
    if (count != -1) {
        return Path(std::string(buffer, count)).parent_path();
    }
    return Path();
}
Path application_install_path(){
    // Check for AppImage environment variables to find the root directory, if running as an AppImage.
    // PA_APPIMAGE_DIR is set by Azure via a patched AppRun script.
    {
        char* dir = std::getenv("PA_APPIMAGE_DIR");
        if (dir != nullptr){
            return dir;
        }
    }
    {
        char* path = std::getenv("APPIMAGE");
        if (path != nullptr){
            return std::filesystem::absolute(path).parent_path().lexically_normal();
        }
    }
    {
        char* app_dir = std::getenv("APPDIR");
        if (app_dir != nullptr){
            std::ifstream mount_info(Filesystem::Path("/proc/self/mountinfo").stdpath());
            if (mount_info.is_open()){
                std::string line;
                while (std::getline(mount_info, line)){
                    size_t dash_sep = line.find(" - ");
                    if (dash_sep == std::string::npos) {
                        continue;
                    }

                    // 1. Extract and tokenize the "pre" segment (before " - ")
                    std::vector<std::string> pre;
                    {
                        std::string pre_str = line.substr(0, dash_sep);
                        std::stringstream ss(pre_str);
                        std::string token;
                        while (ss >> token) {
                            pre.push_back(token);
                        }
                    }

                    // 2. Extract and tokenize the "post" segment (after " - ")
                    std::vector<std::string> post;
                    {
                        std::string post_str = line.substr(dash_sep + 3);
                        std::stringstream ss(post_str);
                        std::string token;
                        while (ss >> token) {
                            post.push_back(token);
                        }
                    }

                    // 3. Validation bounds check
                    if (pre.size() < 5 || post.size() < 2){
                        continue;
                    }

                    // 4. Clean out octal space encodings using standard string operations
                    std::string mount_point = StringTools::replace(pre[4], "\\040", " ");
                    std::string source = StringTools::replace(post[1], "\\040", " ");

                    if (mount_point == app_dir && source.ends_with(".AppImage")){
                        Filesystem::Path dir_path = Filesystem::Path(source).parent_path();
                        Filesystem::Path abs_path = Filesystem::absolute(dir_path);
                        return abs_path.lexically_normal();
                    }
                }
            }
        }
    }

    //  Fallback
    return application_binary_path();
}
Path application_scratch_path(){
    return current_path();
}



}
}

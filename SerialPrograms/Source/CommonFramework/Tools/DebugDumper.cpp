/*  Debug Dumper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QDir>
#include "DebugDumper.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Logging/Logger.h"

namespace PokemonAutomation{

namespace {
const char* debug_dump_folder_name = "DebugDumps";
}

// create a folder with path ./DebugDumps/`folder_path`.
// The function will create all parent folders necessary to create the folder.
void create_debug_folder(const std::string& folder_path){
    QDir().mkdir(debug_dump_folder_name);
    QDir(debug_dump_folder_name).mkpath(folder_path.c_str());
}

std::string dump_debug_image(
    Logger& logger,
    const std::string& path,
    const std::string& label,
    const ImageViewRGB32& image
){
    create_debug_folder(path);
    std::string full_path = debug_dump_folder_name;
    full_path += "/" + path + "/" + now_to_filestring() + "-" + label + ".jpg";
    logger.log("Saving debug image to: " + full_path, COLOR_YELLOW);
    image.save(full_path);
    return full_path;
}


}

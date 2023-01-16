/*  New Version Check
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QMessageBox>
#include "Common/Cpp/Json/JsonObject.h"
#include "Options/Environment/ThemeSelectorOption.h"
#include "Tools/FileDownloader.h"
#include "Globals.h"
#include "NewVersionCheck.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


bool compare_version(Logger& logger, const JsonObject* json){
    if (json == nullptr){
        logger.log("Invalid version JSON.", COLOR_RED);
        return false;
    }

    int major;
    if (!json->read_integer(major, "VersionMajor")){
        logger.log("Invalid version JSON: Missing major version.", COLOR_RED);
        return false;
    }
    if (major > PROGRAM_VERSION_MAJOR){
        return true;
    }
    if (major < PROGRAM_VERSION_MAJOR){
        return false;
    }

    int minor;
    if (!json->read_integer(minor, "VersionMinor")){
        logger.log("Invalid version JSON: Missing minor version.", COLOR_RED);
        return false;
    }
    if (minor > PROGRAM_VERSION_MINOR){
        return true;
    }
    if (minor < PROGRAM_VERSION_MINOR){
        return false;
    }

    int patch;
    if (!json->read_integer(patch, "VersionPatch")){
        logger.log("Invalid version JSON: Missing patch #.", COLOR_RED);
        return false;
    }
    if (patch > PROGRAM_VERSION_PATCH){
        return true;
    }

    return false;
}
void check_new_version(Logger& logger){
    logger.log("Checking for new version...");
    JsonValue json;
    try{
        json = FileDownloader::download_json_file(
            logger,
            "https://raw.githubusercontent.com/PokemonAutomation/ComputerControl/master/LatestVersion.json"
        );
    }catch (OperationFailedException&){
        return;
    }

//    cout << json.dump() << endl;

    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        logger.log("Invalid version JSON.", COLOR_RED);
        return;
    }

    const JsonObject* release = obj->get_object("Release");
    if (compare_version(logger, release)){
        QMessageBox box;
        box.setTextFormat(Qt::RichText);
        box.information(
            nullptr,
            "New Version Available!",
            QString::fromStdString(
                "A new version is available!<br>" +
                make_text_url("https://github.com/PokemonAutomation/ComputerControl/releases", "Download here.")
            )
        );
        return;
    }

    if (!IS_BETA_VERSION){
        return;
    }

    const JsonObject* beta = obj->get_object("Beta");
    if (compare_version(logger, beta)){
        QMessageBox box;
        box.setTextFormat(Qt::RichText);
        box.information(
            nullptr,
            "New Beta Available!",
            QString::fromStdString(
                "A new beta is available!<br>" +
                make_text_url("https://discord.com/channels/695809740428673034/732736538965704726", "Download from our Discord!")
            )
        );
        return;
    }
}





}

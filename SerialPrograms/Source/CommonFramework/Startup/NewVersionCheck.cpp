/*  New Version Check
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QMessageBox>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Options/CheckForUpdatesOption.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/Tools/FileDownloader.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "NewVersionCheck.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


static auto CHECK_FOR_UPDATES_PERIOD = std::chrono::days(1);


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
std::string get_changes(const JsonObject* json){
    const std::string* changes = json->get_string("Changes");
    if (changes == nullptr){
        return "";
    }
    return "<br><br>" + *changes;
}

void check_new_version(Logger& logger){
    bool check_release = GlobalSettings::instance().CHECK_FOR_UPDATES->RELEASE;
    bool check_beta = GlobalSettings::instance().CHECK_FOR_UPDATES->BETA;
    bool check_private_beta = GlobalSettings::instance().CHECK_FOR_UPDATES->PRIVATE_BETA;
    bool updates_enabled = false;
    updates_enabled |= check_release;
    updates_enabled |= check_beta;
    updates_enabled |= check_private_beta;

    if (!updates_enabled){
        return;
    }

    static WallClock last_version_check = WallClock::min();
    WallClock now = current_time();
    if (last_version_check != WallClock::min() && now - last_version_check < CHECK_FOR_UPDATES_PERIOD){
        return;
    }
    last_version_check = now;

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
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        logger.log("Invalid version JSON.", COLOR_RED);
        return;
    }

//    cout << json.dump() << endl;

    if (check_private_beta){
        const JsonObject* node = obj->get_object("PrivateBeta");
        if (compare_version(logger, node)){
            QMessageBox box;
            box.setTextFormat(Qt::RichText);
            std::string text = "A new private beta is available!<br>";
            text += make_text_url(
                "https://discord.com/channels/695809740428673034/732736538965704726",
                "Download from our Discord!"
            );
            text += get_changes(node);
            box.information(
                nullptr,
                "A new private beta is available!",
                QString::fromStdString(text)
            );
            return;
        }
    }
    if (check_beta){
        const JsonObject* node = obj->get_object("Beta");
        if (compare_version(logger, node)){
            QMessageBox box;
            box.setTextFormat(Qt::RichText);
            std::string text = "A new beta is available!<br>";
            text += make_text_url(
                "https://github.com/PokemonAutomation/ComputerControl/releases",
                "Download here."
            );
            text += get_changes(node);
            box.information(
                nullptr,
                "A new beta is available!",
                QString::fromStdString(text)
            );
            return;
        }
    }
    if (check_release){
        const JsonObject* node = obj->get_object("Release");
        if (compare_version(logger, node)){
            QMessageBox box;
            box.setTextFormat(Qt::RichText);
            std::string text = "A new version is available!<br>";
            text += make_text_url(
                "https://github.com/PokemonAutomation/ComputerControl/releases",
                "Download here."
            );
            text += get_changes(node);
            box.information(
                nullptr,
                "New Version Available!",
                QString::fromStdString(text)
            );
            return;
        }
    }

}





}

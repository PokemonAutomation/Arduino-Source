/*  New Version Check
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QPushButton>
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


struct ProgramVersion{
    int major;
    int minor;
    int patch;
    std::string version_text;

    ProgramVersion(Logger& logger, const JsonObject* json){
        if (!json->read_integer(major, "VersionMajor")){
            throw_and_log<ParseException>(logger, "Invalid version JSON: Missing major version.");
        }
        if (!json->read_integer(minor, "VersionMinor")){
            throw_and_log<ParseException>(logger, "Invalid version JSON: Missing minor version.");
        }
        if (!json->read_integer(patch, "VersionPatch")){
            throw_and_log<ParseException>(logger, "Invalid version JSON: Missing patch #.");
        }
        version_text = std::to_string(major)
            + "." + std::to_string(minor)
            + "." + std::to_string(patch);
    }

    bool is_newer() const{
        if (major > PROGRAM_VERSION_MAJOR){
            return true;
        }
        if (major < PROGRAM_VERSION_MAJOR){
            return false;
        }

        if (minor > PROGRAM_VERSION_MINOR){
            return true;
        }
        if (minor < PROGRAM_VERSION_MINOR){
            return false;
        }

        if (patch > PROGRAM_VERSION_PATCH){
            return true;
        }

        return false;
    }

    bool show_update_nag() const{
        if (!is_newer()){
            return false;
        }
        std::string skip_version = GlobalSettings::instance().CHECK_FOR_UPDATES->SKIP_VERSION;
        return version_text != skip_version;
    }
};

bool compare_version(Logger& logger, const JsonObject* json){
    if (json == nullptr){
        logger.log("Invalid version JSON.", COLOR_RED);
        return false;
    }

    int major;
    int minor;
    int patch;
    if (!json->read_integer(major, "VersionMajor")){
        logger.log("Invalid version JSON: Missing major version.", COLOR_RED);
        return false;
    }
    if (!json->read_integer(minor, "VersionMinor")){
        logger.log("Invalid version JSON: Missing minor version.", COLOR_RED);
        return false;
    }
    if (!json->read_integer(patch, "VersionPatch")){
        logger.log("Invalid version JSON: Missing patch #.", COLOR_RED);
        return false;
    }
    std::string full_version = std::to_string(major)
        + "." + std::to_string(minor)
        + "." + std::to_string(patch);
    std::string skip_version = GlobalSettings::instance().CHECK_FOR_UPDATES->SKIP_VERSION;
    if (full_version == skip_version){
        return false;
    }

    if (major > PROGRAM_VERSION_MAJOR){
        return true;
    }
    if (major < PROGRAM_VERSION_MAJOR){
        return false;
    }

    if (minor > PROGRAM_VERSION_MINOR){
        return true;
    }
    if (minor < PROGRAM_VERSION_MINOR){
        return false;
    }

    if (patch > PROGRAM_VERSION_PATCH){
        return true;
    }

    return false;
}
std::string get_changes(const JsonObject& json){
    const std::string* changes = json.get_string("Changes");
    if (changes == nullptr){
        return "";
    }
    return "<br><br>" + *changes;
}
void show_update_box(
    const std::string& title,
    const std::string& link_text,
    const std::string& link_url,
    const std::string& header,
    const ProgramVersion& version,
    const JsonObject& node
){
    QMessageBox box;
    QPushButton* ok = box.addButton(QMessageBox::Ok);
    QPushButton* skip = box.addButton("Skip this Version", QMessageBox::NoRole);
    box.setEscapeButton(ok);
//    cout << "ok = " << ok << endl;
//    cout << "skip = " << skip << endl;

    box.setTextFormat(Qt::RichText);
    std::string text = header + "<br>";
    text += make_text_url(link_url, link_text);
    text += get_changes(node);


    box.setWindowTitle(QString::fromStdString(title));
    box.setText(QString::fromStdString(text));

//    box.open();

    box.exec();

    QAbstractButton* clicked = box.clickedButton();
//    cout << "clicked = " << clicked << endl;
    if (clicked == ok){
        return;
    }
    if (clicked == skip){
        GlobalSettings::instance().CHECK_FOR_UPDATES->SKIP_VERSION.set(version.version_text);
        return;
    }


//    box.information(
//        nullptr,
//        QString::fromStdString(title),
//        QString::fromStdString(text)
//    );
}


void check_new_version(Logger& logger){
    bool check_release = GlobalSettings::instance().CHECK_FOR_UPDATES->RELEASE0;
    bool check_beta = GlobalSettings::instance().CHECK_FOR_UPDATES->PUBLIC_BETA0;
    bool check_private_beta = GlobalSettings::instance().CHECK_FOR_UPDATES->PRIVATE_BETA0;
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
        ProgramVersion latest(logger, node);
        if (latest.show_update_nag()){
            show_update_box(
                "New Private Beta Available!",
                "Download from our Discord!",
                "https://discord.com/channels/695809740428673034/732736538965704726",
                "A new private beta is available!",
                latest, *node
            );
            return;
        }
    }
    if (check_beta){
        const JsonObject* node = obj->get_object("Beta");
        ProgramVersion latest(logger, node);
        if (latest.show_update_nag()){
            show_update_box(
                "New Beta Available!",
                "Download here.",
                "https://github.com/PokemonAutomation/ComputerControl/releases",
                "A new beta is available!",
                latest, *node
            );
            return;
        }
    }
    if (check_release){
        const JsonObject* node = obj->get_object("Release");
        ProgramVersion latest(logger, node);
        if (latest.show_update_nag()){
            show_update_box(
                "New Version Available!",
                "Download here.",
                "https://github.com/PokemonAutomation/ComputerControl/releases",
                "A new version is available!",
                latest, *node
            );
            return;
        }
    }

}





}

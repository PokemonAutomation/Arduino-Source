/*  Setup Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QFile>
#include <QMessageBox>
#include "Options/Environment/ThemeSelectorOption.h"
#include "Globals.h"
#include "SetupSettings.h"

namespace PokemonAutomation{

void setup_settings(Logger& logger, std::string file_name){
    QFile root_file(QString::fromStdString(file_name));
    QFile folder_file(QString::fromStdString(SETTINGS_PATH + file_name));

    logger.log("Checking settings configuration...");

    if (!root_file.exists() && !folder_file.exists()){
        logger.log("Clean install, nothing to do.");
        return;
    }

    if (!root_file.exists() && folder_file.exists()){
        logger.log("Migrated, nothing to do.");
        return;
    }

    if (root_file.exists() && !folder_file.exists()){
        logger.log("Migrating root file to the folder...");
        root_file.copy(folder_file.fileName());
        logger.log("Renaming root file as backup...");
        root_file.rename(root_file.fileName() + ".bak");
        logger.log("Migrated.");
        return;
    }

    if (root_file.exists() && folder_file.exists()){
        logger.log("Two configs detected, showing prompt...");
        QMessageBox box;
        box.setTextFormat(Qt::RichText);
        box.information(
            nullptr,
            "Two settings files detected!",
            QString::fromStdString(
                "We messed this up, we're sorry!<br>We detected conflicting settings files in two locations:<br>" +
                root_file.fileName().toStdString() + "<br>" +
                folder_file.fileName().toStdString() +
                "<br>Please either delete or rename the file you do not want to use."
            )
        );
    }
}




}

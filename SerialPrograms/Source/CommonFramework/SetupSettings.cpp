/*  Setup Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QFile>
#include <QMessageBox>
#include "Globals.h"
#include "GlobalSettingsPanel.h"
#include "SetupSettings.h"

namespace PokemonAutomation{

bool migrate_settings(Logger& logger, std::string file_name){
    QFile root_file(QString::fromStdString(file_name));
    QFile folder_file(QString::fromStdString(SETTINGS_PATH + file_name));

    logger.log("Checking settings configuration...");

    if (!root_file.exists() && !folder_file.exists()){
        logger.log("Clean install, nothing to do.");
        return true;
    }

    if (!root_file.exists() && folder_file.exists()){
        logger.log("Migrated, nothing to do.");
        return true;
    }

    if (root_file.exists() && !folder_file.exists()){
        logger.log("Migrating root file to the folder...");
        root_file.copy(folder_file.fileName());
        logger.log("Renaming root file as backup...");
        root_file.rename(root_file.fileName() + ".bak");
        logger.log("Migrated.");
        QMessageBox box;
        box.setTextFormat(Qt::RichText);
        box.information(
            nullptr,
            "Settings Migrated!",
            QString::fromStdString(
                "Detected a settings file at the old location used by version 0.29 and earlier.<br>"
                "It has been automatically moved into the \"" + SETTINGS_PATH + "\" folder."
            )
        );
        return true;
    }

    if (root_file.exists() && folder_file.exists()){
        logger.log("Two configs detected, showing prompt...");
        QMessageBox box;
        box.setTextFormat(Qt::RichText);
        box.critical(
            nullptr,
            "Two settings files detected!",
            QString::fromStdString(
                "Detected two settings files at these locations:<br><br>" +
                root_file.fileName().toStdString() + " (versions 0.29 and older)<br>" +
                folder_file.fileName().toStdString() + " (version 0.30)" +
                "<br><br>This probably happened because you used an early beta of v0.30.x which created a new settings file at the new location. "
                "Please either delete or rename the file you do not want to use."
            )
        );
        return false;
    }

    return true;
}

bool migrate_stats(Logger& logger){
    logger.log("Checking if stats should be migrated...");

    std::string path = GlobalSettings::instance().STATS_FILE;
    if (path != "PA-Stats.txt"){
        logger.log("Stats file not at old default location. Will not attempt to migrate...");
        return true;
    }

    const std::string new_path = GlobalSettings::instance().STATS_FILE.default_value();

    QFile root_file(QString::fromStdString(path));
    QFile folder_file(QString::fromStdString(new_path));

    if (!root_file.exists() && !folder_file.exists()){
        logger.log("Clean install, nothing to do.");
        return true;
    }

    if (!root_file.exists() && folder_file.exists()){
        logger.log("File migrated. Stats path in settings not updated. Updating...");
        GlobalSettings::instance().STATS_FILE.restore_defaults();
        return true;
    }

    if (root_file.exists() && !folder_file.exists()){
        logger.log("Migrating root file to the folder...");
        root_file.copy(folder_file.fileName());
        logger.log("Renaming root file as backup...");
        root_file.rename(root_file.fileName() + ".bak");
        GlobalSettings::instance().STATS_FILE.restore_defaults();
        logger.log("Migrated.");
        QMessageBox box;
        box.setTextFormat(Qt::RichText);
        box.information(
            nullptr,
            "Settings Migrated!",
            QString::fromStdString(
                "Detected a stats file at the old location used by version 0.29 and earlier.<br>"
                "It has been automatically moved into the \"" + SETTINGS_PATH + "\" folder."
            )
        );
        return true;
    }

    if (root_file.exists() && folder_file.exists()){
        logger.log("Two stats files detected, showing prompt...");
        QMessageBox box;
        box.setTextFormat(Qt::RichText);
        box.critical(
            nullptr,
            "Two settings files detected!",
            QString::fromStdString(
                "Detected two stats files at these locations:<br><br>" +
                root_file.fileName().toStdString() + " (versions 0.29 and older)<br>" +
                folder_file.fileName().toStdString() + " (version 0.30)" +
                "<br><br>This probably happened because you used an early beta of v0.30.x which created a new stats file at the new location. "
                "Please either delete or rename the file you do not want to use."
            )
        );
        return false;
    }

    return true;
}






}

/*  Setup Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#if QT_CONFIG(permissions)
#include <QPermissions>
#endif
#endif
#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "SetupSettings.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

bool migrate_settings(Logger& logger, const std::string& file_name){
    QFile cur_dir_file(QString::fromStdString(file_name));
    QFile folder_file(QString::fromStdString(SETTINGS_PATH() + file_name));

    logger.log("Checking if user setting JSON file need to be migrated...");

    if (!cur_dir_file.exists() && !folder_file.exists()){
        logger.log("Clean install, nothing to migrate.");
        return true;
    }

    if (!cur_dir_file.exists() && folder_file.exists()){
        logger.log("Migrated, nothing to do.");
        return true;
    }

    if (cur_dir_file.exists() && !folder_file.exists()){
        logger.log("Migrating old setting file to the new folder...");
        cur_dir_file.copy(folder_file.fileName());
        logger.log("Renaming old setting file as backup...");
        cur_dir_file.rename(cur_dir_file.fileName() + ".bak");
        logger.log("Migrated.");
        QMessageBox box;
        box.setTextFormat(Qt::RichText);
        box.information(
            nullptr,
            "Settings Migrated!",
            QString::fromStdString(
                "Detected a settings file at the old location used by version 0.29 and earlier.<br>"
                "It has been automatically moved into the \"" + SETTINGS_PATH() + "\" folder."
            )
        );
        return true;
    }

    // last case: cur_dir_file.exists() && folder_file.exists()
    logger.log("Two configs detected, showing prompt...");
    QMessageBox box;
    box.setTextFormat(Qt::RichText);
    box.critical(
        nullptr,
        "Two settings files detected!",
        QString::fromStdString(
            "Detected two settings files at these locations:<br><br>" +
            cur_dir_file.fileName().toStdString() + " (versions 0.29 and older)<br>" +
            folder_file.fileName().toStdString() + " (version 0.30)" +
            "<br><br>This probably happened because you used an early beta of v0.30.x which created a new "
            "settings file at the new location. Please either delete or rename the file you do not want to use."
        )
    );
    return false;
}

bool migrate_stats(Logger& logger){
    logger.log("Checking if stats file should be migrated...");

    std::string path = GlobalSettings::instance().STATS_FILE;
    if (path != "PA-Stats.txt"){
        logger.log("Stats file not at old default location. Will not attempt to migrate...");
        return true;
    }

    const std::string new_path = GlobalSettings::instance().STATS_FILE.default_value();

    // old location: current working directory
    QFile cur_dir_file(QString::fromStdString(path));
    // new location: 
    QFile folder_file(QString::fromStdString(new_path));

    if (!cur_dir_file.exists() && !folder_file.exists()){
        logger.log("Clean install, nothing to migrate.");
        return true;
    }

    if (!cur_dir_file.exists() && folder_file.exists()){
        logger.log("File migrated. Stats path in settings not updated. Updating...");
        GlobalSettings::instance().STATS_FILE.restore_defaults();
        return true;
    }

    if (cur_dir_file.exists() && !folder_file.exists()){
        logger.log("Migrating old stats file to the folder...");
        cur_dir_file.copy(folder_file.fileName());
        logger.log("Renaming old stats file as backup...");
        cur_dir_file.rename(cur_dir_file.fileName() + ".bak");
        GlobalSettings::instance().STATS_FILE.restore_defaults();
        logger.log("Migrated.");
        QMessageBox box;
        box.setTextFormat(Qt::RichText);
        box.information(
            nullptr,
            "Settings Migrated!",
            QString::fromStdString(
                "Detected a stats file at the old location used by version 0.29 and earlier.<br>"
                "It has been automatically moved into the \"" + SETTINGS_PATH() + "\" folder."
            )
        );
        return true;
    }

    // last case: cur_dir_file.exists() && folder_file.exists()
    logger.log("Two stats files detected, showing prompt...");
    QMessageBox box;
    box.setTextFormat(Qt::RichText);
    box.critical(
        nullptr,
        "Two settings files detected!",
        QString::fromStdString(
            "Detected two stats files at these locations:<br><br>" +
            cur_dir_file.fileName().toStdString() + " (versions 0.29 and older)<br>" +
            folder_file.fileName().toStdString() + " (version 0.30)" +
            "<br><br>This probably happened because you used an early beta of v0.30.x which created a new stats file at the new location. "
            "Please either delete or rename the file you do not want to use."
        )
    );
    return false;
}


void set_permissions(QObject& object){
#if defined(__APPLE__)
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#if QT_CONFIG(permissions)
    cout << "Checking MacOS Permissions..." << endl;
    QCameraPermission camera_permission;
    switch(qApp->checkPermission(camera_permission)){
    case Qt::PermissionStatus::Undetermined:
        cout << "Camera permission undetermined" << endl;
        qApp->requestPermission(camera_permission, &object, [](const QPermission &permission){
            cout << "camera request is ready!" << endl;
        });
        cout << "Requested Camera permission" << endl;
        break;
    case Qt::PermissionStatus::Denied:
        cout << "No camera permission found" << endl;
        qApp->requestPermission(camera_permission, &object, [](const QPermission &permission){
            cout << "camera request is ready!" << endl;
        });
        cout << "Requested Camera permission" << endl;
        break;
    case Qt::PermissionStatus::Granted:
        cout << "Camera permission granted" << endl;
        break;
    }

    QMicrophonePermission microphone_permission;
    switch(qApp->checkPermission(microphone_permission)){
    case Qt::PermissionStatus::Undetermined:
        cout << "Microphone permission undetermined" << endl;
        qApp->requestPermission(microphone_permission, &object, [](const QPermission &permission){
            cout << "Microphone request is ready!" << endl;
        });
        cout << "Requested microphone permission" << endl;
        break;
    case Qt::PermissionStatus::Denied:
        cout << "No microphone permission found" << endl;
        qApp->requestPermission(microphone_permission, &object, [](const QPermission &permission){
            cout << "Microphone request is ready!" << endl;
        });
        cout << "Requested microphone permission" << endl;
        break;
    case Qt::PermissionStatus::Granted:
        cout << "Microphone permission granted" << endl;
        break;
    }
#endif
#endif
#endif
}


bool check_resource_folder(Logger& logger){
    QDir resources_folder(QString::fromStdString(RESOURCE_PATH()));
    if (resources_folder.exists()){
        logger.log("Found Resource folder at " + RESOURCE_PATH());
        return true;
    }
    logger.log("No Resources/ folder at " + RESOURCE_PATH());
    QMessageBox box;
    box.setTextFormat(Qt::RichText);
    box.critical(
        nullptr,
        "No Resources Folder!",
        QString::fromStdString(
            "No Resources Folder:<br><br>"
            "Make sure you download Resources folder from latest release at<br>"
            "https://github.com/PokemonAutomation/ComputerControl/releases/<br><br>"
            "The Resources folder is part of PA-SerialPrograms-[VERSION]-[DATE].zip<br><br>"
            "Extract the Resources folder from the zip file and place it at following path:<br><br>" + RESOURCE_PATH()
        )
    );
    return false;
}



}

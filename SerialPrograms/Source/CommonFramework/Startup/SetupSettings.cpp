/*  Setup Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include <QMessageBox>
#include <QApplication>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#if QT_CONFIG(permissions)
#include <QPermissions>
#endif
#endif
#include "Common/Compiler.h"
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "SetupSettings.h"

using std::cout;
using std::endl;

namespace PokemonAutomation{

void set_permissions(QObject& object){
    (void)object;
#if defined(__APPLE__)
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#if QT_CONFIG(permissions)
    cout << "Checking MacOS Permissions..." << endl;
    QCameraPermission camera_permission;
    switch(qApp->checkPermission(camera_permission)){
    case Qt::PermissionStatus::Undetermined:
        cout << "Camera permission undetermined" << endl;
        qApp->requestPermission(camera_permission, &object, [](const QPermission& permission){
            cout << "camera request is ready!" << endl;
        });
        cout << "Requested Camera permission" << endl;
        break;
    case Qt::PermissionStatus::Denied:
        cout << "No camera permission found" << endl;
        qApp->requestPermission(camera_permission, &object, [](const QPermission& permission){
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
        qApp->requestPermission(microphone_permission, &object, [](const QPermission& permission){
            cout << "Microphone request is ready!" << endl;
        });
        cout << "Requested microphone permission" << endl;
        break;
    case Qt::PermissionStatus::Denied:
        cout << "No microphone permission found" << endl;
        qApp->requestPermission(microphone_permission, &object, [](const QPermission& permission){
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
    if (Filesystem::exists(Filesystem::Path(RESOURCE_PATH()))){
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

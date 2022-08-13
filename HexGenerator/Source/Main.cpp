
#include <memory>
#include <vector>
#include <iostream>
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include "Tools/PersistentSettings.h"
#include "UI/MainWindow.h"

//using std::cout;
//using std::endl;

//#include <QFile>

namespace PokemonAutomation{
namespace HexGenerator{

std::unique_ptr<QApplication> application;

}
}

using namespace PokemonAutomation;
using namespace HexGenerator;

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    application.reset(new QApplication(argc, argv));

    settings.load();

    if (!QDir(QString::fromStdString(settings.path + CONFIG_FOLDER_NAME)).exists()){
        QMessageBox box;
        box.critical(
            nullptr, "Error",
            "Unable to find source directory.\r\nPlease unzip the package if you haven't already."
        );
    }

    for (QChar ch : settings.path){
        if (ch.unicode() >= 128){
            QMessageBox box;
            box.critical(
                nullptr, "Error",
                QString::fromStdString(
                    "Unicode characters found in the path name. Please move the folder to a place with only English characters.\r\n"
                    "GNU Make does not work with filepaths containing non-ASCII characters.\r\n"
                    "\r\n"
                    "Path: " + settings.path
                )
            );
            break;
        }
    }

    MainWindow w;
    w.show();
    int ret = application->exec();
    settings.write();
    return ret;
}


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

QApplication* application = nullptr;

}
}

using namespace PokemonAutomation;
using namespace HexGenerator;

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    auto app = std::make_unique<QApplication>(argc, argv);
    application = app.get();

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
    int ret = app->exec();
    settings.write();
    return ret;
}

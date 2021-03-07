
#include <memory>
#include <vector>
#include <iostream>
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include "Common/Qt/QtJsonTools.h"
#include "Tools/PersistentSettings.h"
#include "UI/MainWindow.h"

//using std::cout;
//using std::endl;

//#include <QFile>

namespace PokemonAutomation{

std::unique_ptr<QApplication> application;

}

using namespace PokemonAutomation;

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    application.reset(new QApplication(argc, argv));

    settings.load();

    if (!QDir(settings.path + CONFIG_FOLDER_NAME).exists()){
        QMessageBox box;
        box.critical(nullptr, "Error", "Unable to find source directory.\r\nPlease unzip the package if you haven't already.");
    }

    MainWindow w;
    w.show();
    int ret = application->exec();
    settings.write();
    return ret;
}

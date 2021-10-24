
#include <QApplication>
#include "Common/Cpp/Exception.h"
#include "PersistentSettings.h"
#include "CrashDump.h"
#include "Tools/StatsDatabase.h"
#include "Tools/Logger.h"
#include "Integrations/SleepyDiscordRunner.h"
#include "Windows/MainWindow.h"

#include <iostream>
using std::cout;
using std::endl;


using namespace PokemonAutomation;

Q_DECLARE_METATYPE(std::string)

int main(int argc, char *argv[]){
    setup_crash_handler();

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication application(argc, argv);

    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<std::string>();

    try{
        PERSISTENT_SETTINGS().read();
    }catch (const StringException& error){
        global_logger_tagged().log(error.what(), "red");
    }


    int ret;
    {
        MainWindow w;
        w.show();
        ret = application.exec();
    }
    PERSISTENT_SETTINGS().write();
    Integration::SleepyDiscordRunner::sleepy_terminate();
    return ret;
}

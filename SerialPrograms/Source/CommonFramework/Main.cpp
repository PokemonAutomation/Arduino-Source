
#include <QApplication>
#include "Common/Cpp/Exception.h"
#include "PersistentSettings.h"
#include "CrashDump.h"
#include "Tools/StatsDatabase.h"
#include "Windows/MainWindow.h"

#include <iostream>
using std::cout;
using std::endl;


using namespace PokemonAutomation;


int main(int argc, char *argv[]){
    setup_crash_handler();

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication application(argc, argv);

    try{
        PERSISTENT_SETTINGS().read();
    }catch (const StringException& error){
        cout << error.what() << endl;
    }

#if 0
    {
        StatSet stats;
        stats.open_from_file(PERSISTENT_SETTINGS().stats_file);
        stats.save_to_file(PERSISTENT_SETTINGS().stats_file);
    }
#endif

    int ret;
    {
        MainWindow w;
        w.show();
        ret = application.exec();
    }
    PERSISTENT_SETTINGS().write();
    return ret;
}

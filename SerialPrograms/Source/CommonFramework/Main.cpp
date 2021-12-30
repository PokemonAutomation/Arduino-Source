
#include <QApplication>
#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "PersistentSettings.h"
#include "CrashDump.h"
#include "Environment/HardwareValidation.h"
#include "Logging/Logger.h"
#include "Logging/OutputRedirector.h"
#include "Tools/StatsDatabase.h"
#include "Integrations/SleepyDiscordRunner.h"
#include "Windows/MainWindow.h"

#include <iostream>
using std::cout;
using std::endl;


using namespace PokemonAutomation;

Q_DECLARE_METATYPE(std::string)





int main(int argc, char *argv[]){
    setup_crash_handler();

#if QT_VERSION_MAJOR == 5 // AA_EnableHighDpiScaling is deprecated in Qt6
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication application(argc, argv);

    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<std::string>();

    OutputRedirector redirect_stdout(std::cout, "stdout", Color());
    OutputRedirector redirect_stderr(std::cerr, "stderr", COLOR_RED);

    if (!check_hardware()){
        return 1;
    }

    try{
        PERSISTENT_SETTINGS().read();
    }catch (const StringException& error){
        global_logger_tagged().log(error.what(), COLOR_RED);
    }


    int ret;
    {
        MainWindow w;
        w.show();
        ret = application.exec();
    }
    PERSISTENT_SETTINGS().write();

#ifdef PA_SLEEPY
    Integration::SleepyDiscordRunner::sleepy_terminate();
#endif

    return ret;
}

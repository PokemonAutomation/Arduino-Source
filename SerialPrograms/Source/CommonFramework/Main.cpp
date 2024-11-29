
#include <QDir>
#include <QApplication>
#include <QFileInfo>
//#include <QTextStream>
#include <QMessageBox>
#include <dpp/DPP_SilenceWarnings.h>
#include <Integrations/DppIntegration/DppClient.h>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/ImageResolution.h"
#include "PersistentSettings.h"
#include "Tests/CommandLineTests.h"
#include "ErrorReports/ProgramDumper.h"
#include "Environment/HardwareValidation.h"
#include "Logging/Logger.h"
#include "Logging/OutputRedirector.h"
//#include "Tools/StatsDatabase.h"
#include "Integrations/SleepyDiscordRunner.h"
#include "Globals.h"
#include "GlobalSettingsPanel.h"
//#include "Windows/DpiScaler.h"
#include "SetupSettings.h"
#include "NewVersionCheck.h"
#include "Windows/MainWindow.h"


#include <iostream>
using std::cout;
using std::endl;


using namespace PokemonAutomation;

Q_DECLARE_METATYPE(std::string)

void set_working_directory(){
    QString application_dir_path = qApp->applicationDirPath();
    if (application_dir_path.endsWith(".app/Contents/MacOS")){
        // a macOS bundle. Change working directory to the folder that hosts the .app folder.
        QString app_bundle_path = application_dir_path.chopped(15);
        QString base_folder_path = QFileInfo(app_bundle_path).dir().absolutePath();
        QDir::setCurrent(base_folder_path);
    }
}

int main(int argc, char *argv[]){
    setup_crash_handler();

//#if QT_VERSION_MAJOR == 5 // AA_EnableHighDpiScaling is deprecated in Qt6
//    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//#endif
    QApplication application(argc, argv);

    global_logger_tagged().log("================================================================================");
    global_logger_tagged().log("Starting Program...");

    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<Resolution>("Resolution");

    OutputRedirector redirect_stdout(std::cout, "stdout", Color());
    OutputRedirector redirect_stderr(std::cerr, "stderr", COLOR_RED);

    QDir().mkpath(QString::fromStdString(SETTINGS_PATH()));
    QDir().mkpath(QString::fromStdString(SCREENSHOTS_PATH()));

    //  Read program settings from json file: SerialPrograms-Settings.json.
    try{
        if (!migrate_settings(global_logger_tagged(), application.applicationName().toStdString() + "-Settings.json")){
            return 1;
        }

        PERSISTENT_SETTINGS().read();

        if (!migrate_stats(global_logger_tagged())){
            return 1;
        }
    }catch (const FileException& error){
        global_logger_tagged().log(error.message(), COLOR_RED);
    }catch (const ParseException& error){
        global_logger_tagged().log(error.message(), COLOR_RED);
    }

    if (GlobalSettings::instance().COMMAND_LINE_TEST_MODE){
        return run_command_line_tests();
    }

    //  Check whether the hardware is powerful enough to run this program.
    if (!check_hardware()){
        return 1;
    }

    check_new_version(global_logger_tagged());

    Integration::DiscordIntegrationSettingsOption& discord_settings = GlobalSettings::instance().DISCORD.integration;
    if (discord_settings.run_on_start){
#ifdef PA_SLEEPY
        if (discord_settings.library0 == Integration::DiscordIntegrationSettingsOption::Library::SleepyDiscord){
            Integration::SleepyDiscordRunner::sleepy_connect();
        }
#endif
#ifdef PA_DPP
        if (discord_settings.library0 == Integration::DiscordIntegrationSettingsOption::Library::DPP){
            Integration::DppClient::Client::instance().connect();
        }
#endif
        discord_settings.value_changed(nullptr);
    }

    set_working_directory();
    SendableErrorReport::send_all_unsent_reports(global_logger_tagged());
    
    int ret = 0;
    {
        MainWindow w;
        w.show();
        w.raise(); // bring the window to front on macOS
        set_permissions(w);
        ret = application.exec();
    }

    // Write program settings back to the json file.
    PERSISTENT_SETTINGS().write();

#ifdef PA_SLEEPY
    Integration::SleepyDiscordRunner::sleepy_terminate();
#endif

#ifdef PA_DPP
    Integration::DppClient::Client::instance().disconnect();
#endif

    return ret;
}


#include <QDir>
#include <QApplication>
#include <QFileInfo>
//#include <QTextStream>
#include <QMessageBox>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/ImageResolution.h"
#include "PersistentSettings.h"
#include "Tests/CommandLineTests.h"
#include "ErrorReports/ProgramDumper.h"
#include "ErrorReports/ErrorReports.h"
#include "Environment/HardwareValidation.h"
#include "Integrations/DiscordSettingsOption.h"
#include "Integrations/DiscordSocial/DiscordSocial.h"
#include "Integrations/DppIntegration/DppClient.h"
#include "Logging/Logger.h"
#include "Logging/OutputRedirector.h"
//#include "Tools/StatsDatabase.h"
#include "Globals.h"
#include "GlobalSettingsPanel.h"
//#include "Windows/DpiScaler.h"
#include "Startup/SetupSettings.h"
#include "Startup/NewVersionCheck.h"
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

#if defined(__linux) || defined(__APPLE__)
    // By default Qt uses native menubar but this only works on Windows.
    // We use menubar in our ButtonDiagram window to choose which controller's button mapping image to show.
    // So we fix it by don't using native menubar on non-Windows OS.
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
#endif

//#if QT_VERSION_MAJOR == 5 // AA_EnableHighDpiScaling is deprecated in Qt6
//    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//#endif
    QApplication application(argc, argv);

    Logger& logger = global_logger_tagged();

    logger.log("================================================================================");
    logger.log("Starting Program...");

    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<Resolution>("Resolution");

    OutputRedirector redirect_stdout(std::cout, "stdout", Color());
    OutputRedirector redirect_stderr(std::cerr, "stderr", COLOR_RED);

    QDir().mkpath(QString::fromStdString(SETTINGS_PATH()));
    QDir().mkpath(QString::fromStdString(SCREENSHOTS_PATH()));

    //  Several novice developers struggled to build and run the program due to missing Resources folder.
    //  Add this check to pop a message box when Resources folder is missing.
    if (!check_resource_folder(logger)){
        return 1;
    }

    //  Read program settings from json file: SerialPrograms-Settings.json.
    try{
        if (!migrate_settings(logger, application.applicationName().toStdString() + "-Settings.json")){
            return 1;
        }

        PERSISTENT_SETTINGS().read();

        if (!migrate_stats(logger)){
            return 1;
        }
    }catch (const FileException& error){
        logger.log(error.message(), COLOR_RED);
    }catch (const ParseException& error){
        logger.log(error.message(), COLOR_RED);
    }

    if (GlobalSettings::instance().COMMAND_LINE_TEST_MODE){
        return run_command_line_tests();
    }

    //  Check whether the hardware is powerful enough to run this program.
    if (!check_hardware()){
        return 1;
    }

    check_new_version(logger);

    Integration::DiscordIntegrationSettingsOption& discord_settings = GlobalSettings::instance().DISCORD->integration;
    if (discord_settings.run_on_start){
#ifdef PA_DPP
        Integration::DppClient::Client::instance().connect();
#endif
        discord_settings.on_config_value_changed(nullptr);
    }

#ifdef PA_SOCIAL_SDK
    if (GlobalSettings::instance().RICH_PRESENCE){
        Integration::DiscordSocialSDK::DiscordSocial::instance().run();
    }
#endif

    set_working_directory();

    //  Run this asynchronously to we don't block startup.
    std::unique_ptr<AsyncTask> task = send_all_unsent_reports(logger, true);

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

#ifdef PA_DPP
    Integration::DppClient::Client::instance().disconnect();
#endif

    return ret;
}


#include <iostream>
//#include <QTextStream>
#include <QApplication>
#include "Common/Cpp/Logging/LastLogTracker.h"
#include "Common/Cpp/Logging/FileLogger.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "Common/Cpp/Logging/MultiOutputLogger.h"
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/Filesystem/FileIO.h"
#include "Common/Cpp/Concurrency/Qt6.9ThreadBugWorkaround.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/FireForgetDispatcher.h"
#include "Common/Cpp/Concurrency/Watchdog.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/ImageResolution.h"
#include "Common/Cpp/ScopeExit.h"
#include "Common/Qt/GlobalThreadPoolsQt.h"
#include "StaticRegistration.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/Panels/OptionsPanel.h"
#include "CommonFramework/VideoPipeline/Backends/CameraImplementations.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "Tests/CommandLineTests.h"
#include "ControllerInput/ControllerInput.h"
#include "Controllers/SerialPort/SerialPortPollerQt.h"
#include "Integrations/DiscordSettingsOption.h"
#include "Integrations/DiscordSocial/DiscordSocial.h"
#include "Integrations/DppIntegration/DppClient.h"
#include "Integrations/DiscordWebhook.h"
#include "VideoPipeline/Backends/MediaServicesQt6.h"
#include "ErrorReports/ProgramDumper.h"
#include "ErrorReports/ErrorReports.h"
#include "Environment/HardwareValidation.h"
//#include "Tools/StatsDatabase.h"
//#include "Windows/DpiScaler.h"
#include "Logging/Logger.h"
#include "Logging/OutputRedirector.h"
#include "Startup/SetupSettings.h"
#include "Startup/NewVersionCheck.h"
#include "Windows/MainWindow.h"
#include "GlobalSettingsPanel.h"
#include "GlobalServices.h"
#include "PersistentSettings.h"
#include "NintendoSwitch/NintendoSwitch_SettingsPanel.h"

using namespace PokemonAutomation;

namespace PokemonAutomation{
    bool USE_QT_UI = true;
}

Q_DECLARE_METATYPE(std::string)

void set_working_directory(){
    const std::string app_dir = Filesystem::application_binary_directory().string_slash_normalized();
    const std::string bundle_dir = "/Contents/MacOS";
    const std::string bundle_suffix = ".app" + bundle_dir;
    if (app_dir.size() > bundle_suffix.size() &&
        app_dir.compare(app_dir.size() - bundle_suffix.size(), bundle_suffix.size(), bundle_suffix) == 0){
        // a macOS bundle. Change working directory to the folder that hosts the .app folder.
        const std::string app_bundle_path = app_dir.substr(0, app_dir.size() - bundle_dir.size());
        const Filesystem::Path base_folder_path = Filesystem::Path(app_bundle_path).parent_path();
        std::error_code ec;
        Filesystem::set_current_path(base_folder_path, ec);
    }
}

namespace PokemonAutomation{



FileLogger& global_file_logger(){
    static FileLogger logger(
        GlobalThreadPools::unlimited_normal(),
        FileLoggerConfig{
            .file_path = USER_FILE_PATH() + Filesystem::application_binary_name().stem().string() + ".log",
        }
    );
    return logger;
}

}



int run_program(int argc, char *argv[]){
//    qputenv("QT_RHI_BACKEND", "opengl");

#if defined(__APPLE__)
    Filesystem::set_startup_profile(argc, argv);
    QApplication application(argc, argv);
#else
    QApplication application(argc, argv);
#endif

#ifdef PA_DPP
    // Configure OpenSSL before DPP or background tasks create a TLS context.
    std::string dpp_cert_message;
    bool dpp_cert_error = false;
    {
        const std::string cert_env = qgetenv("SSL_CERT_FILE").toStdString();
        const bool use_bundled_cert = cert_env.empty();
        const std::string cert_path = use_bundled_cert
            ? (Filesystem::application_binary_directory() / Filesystem::Path("dpp-cacert.pem")).string_slash_normalized()
            : cert_env;
        FileIO cert_file;
        if (!cert_file.open(Filesystem::Path(cert_path), FileMode::READ | FileMode::BINARY)){
            const std::string error = std::error_code(errno, std::generic_category()).message();
            dpp_cert_error = true;
            dpp_cert_message = "DPP CA certificate bundle cannot be read: " + cert_path +
                " (" + error + "). Discord bot connections may fail.";
        }else{
            if (use_bundled_cert){
                dpp_cert_error = !qputenv("SSL_CERT_FILE", cert_path.c_str());
#ifdef _WIN32
                // OpenSSL uses the release CRT even when Qt uses the debug CRT.
                // Set both environment copies so OpenSSL can find the bundle.
                const HMODULE crt = LoadLibraryExW(L"ucrtbase.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
                ScopeExit unload_crt([crt]{
                    if (crt){
                        FreeLibrary(crt);
                    }
                });
                using PutEnv = int (__cdecl *)(const char*, const char*);
                const PutEnv openssl_putenv = crt
                    ? reinterpret_cast<PutEnv>(GetProcAddress(crt, "_putenv_s"))
                    : nullptr;
                dpp_cert_error = dpp_cert_error ||
                    !openssl_putenv || openssl_putenv("SSL_CERT_FILE", cert_path.c_str()) != 0;
#endif
            }
            dpp_cert_message = dpp_cert_error
                ? "Failed to set SSL_CERT_FILE for DPP: " + cert_path
                : "DPP CA certificate bundle: " + cert_path;
        }
    }
#endif

    {
        const std::string qml_import_path =
            (Filesystem::application_binary_directory() / Filesystem::Path("qml")).string_slash_normalized();
        qputenv("QML_IMPORT_PATH", qml_import_path.c_str());
    }

    GlobalOutputRedirector redirect_stdout(std::cout, "stdout", Color());
    GlobalOutputRedirector redirect_stderr(std::cerr, "stderr", COLOR_RED);

    {
        MultiOutputLogger& logger = global_multi_logger();
        logger.add_listener(global_last_log_history());
        logger.add_listener(global_file_logger());
    }

    Logger& logger = global_logger_tagged();

    logger.log("================================================================================");
    logger.log("Starting Program...");
    logger.log("Current path: " + Filesystem::current_path().string_slash_normalized());
    logger.log("Executable path: " + Filesystem::application_binary_directory().string_slash_normalized());
    logger.log("Program setting folder: " + SETTINGS_PATH());
    logger.log("Program resources folder: " + RESOURCE_PATH());
#ifdef PA_DPP
    logger.log(dpp_cert_message, dpp_cert_error ? COLOR_RED : Color());
#endif

    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<Resolution>("Resolution");

#if defined(__linux) || defined(__APPLE__)
    // By default Qt uses native menubar but this only works on Windows.
    // We use menubar in our ButtonDiagram window to choose which controller's button mapping image to show.
    // So we fix it by don't using native menubar on non-Windows OS.
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
#endif

//#if QT_VERSION_MAJOR == 5 // AA_EnableHighDpiScaling is deprecated in Qt6
//    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//#endif

    Filesystem::create_directories(Filesystem::Path(SETTINGS_PATH()));
    Filesystem::create_directories(Filesystem::Path(SCREENSHOTS_PATH()));



    ScopeExit cleanup([]{
        SerialPortPoller::instance().stop();
        GlobalMediaServices::instance().stop();
    });

    //  Several novice developers struggled to build and run the program due to missing Resources folder.
    //  Add this check to pop a message box when Resources folder is missing.
    if (!check_resource_folder(logger)){
        return 1;
    }

    //  Read program settings from json file: SerialPrograms-Settings.json.
    try{
        PERSISTENT_SETTINGS().read();
    }catch (const FileException& error){
        logger.log(error.message(), COLOR_RED);
    }catch (const ParseException& error){
        logger.log(error.message(), COLOR_RED);
    }

    //  Load the Switch framework settings now, not only when their panel is opened.
    OptionsPanelWrapper<NintendoSwitch::ConsoleSettingsPanel>().make_panel()->load_json_from_global();

    //  Preload a bunch of stuff now so they are ready later.
    SerialPortPoller::instance().ports();
    get_all_cameras();

    //  Force all the Qt thread pools to be constructed now on the main thread.
    GlobalThreadPools::qt_worker_threadpool();
    GlobalThreadPools::qt_event_threadpool();

    bool augment_test_color_distortions = false;
    for (size_t i = 0; i < argc; i++){
        constexpr const char* force_run_tests = "--command-line-test-mode";
        constexpr const char* distort_test_images = "--augment-color-distortions";

        if (strcmp(argv[i], force_run_tests) == 0){
            GlobalSettings::instance().COMMAND_LINE_TEST_MODE = true;
        }
        if (strcmp(argv[i], distort_test_images) == 0){
            augment_test_color_distortions = true;
        }
    }

    if (GlobalSettings::instance().COMMAND_LINE_TEST_MODE){
        return run_command_line_tests(augment_test_color_distortions);
    }

    //  Check whether the hardware is powerful enough to run this program.
    if (!check_hardware()){
        return 1;
    }

    check_new_version(logger);

    set_working_directory();

    //  Run this asynchronously to we don't block startup.
    AsyncTask task = send_all_unsent_reports(logger, true);



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


    MainWindow w;
    w.show();
    w.raise(); // bring the window to front on macOS
    set_permissions(w);

    int ret = application.exec();

    //  Write program settings back to the json file.
    std::cout << "Saving Settings..." << std::endl;
    PERSISTENT_SETTINGS().write();

    return ret;
}


int main(int argc, char *argv[]){
#if 0
    // Retrieve and store program name
    set_program_path(argv[0]);
#endif

#if defined(__linux__)
    // Qt multimedia, default to gstreamer to prevent flickering
    // Easier than the alternative which is compiling qt6multimedia with QT_DEFAULT_MEDIA_BACKEND
    // See: https://doc.qt.io/qt-6.5/qtmultimedia-index.html
    if (qEnvironmentVariableIsEmpty("QT_MEDIA_BACKEND")){
        qputenv("QT_MEDIA_BACKEND", "gstreamer");
    }
#endif

    //  So far, this is only needed on Mac where static initialization is fucked up.
    PokemonAutomation::register_all_statics();

    setup_crash_handler();

    int ret = run_program(argc, argv);


#ifdef PA_SOCIAL_SDK
    Integration::DiscordSocialSDK::DiscordSocial::instance().stop();
#endif

#ifdef PA_DPP
    Integration::DppClient::Client::instance().stop();
#endif

    //  We must clear the OCR cache or it will crash on Linux when the library
    //  unloads before the cache is destructed from static memory.
    OCR::clear_ocr_cache();

    //  Stop the controllers.
    global_input_stop();

    //  Stop misc. services.
    Integration::DiscordWebhook::DiscordWebhookSender::instance().stop();
    SystemSleepController::instance().stop();
    global_periodic_runner().stop();
    global_watchdog().stop();
    global_file_logger().stop();

//
//  Workaround Qt 6.9 thread-adoption bug on Windows.
//      https://github.com/PokemonAutomation/Arduino-Source/issues/570
//      https://bugreports.qt.io/browse/QTBUG-131892
//
//  Joining threads on Qt 6.9+ Windows will hang!
//
#ifdef PA_ENABLE_QT_ADOPTION_WORKAROUND
    std::cout << "Qt6.9 bug workaround: std::quick_exit() to avoid thread join hangs." << std::endl;
    std::quick_exit(ret);
#endif

    //  Force stop the thread pools.
    //  This is where all the threads in the program are joined.
    PokemonAutomation::GlobalThreadPools::computation_realtime().stop();
    PokemonAutomation::GlobalThreadPools::computation_normal().stop();
    PokemonAutomation::GlobalThreadPools::unlimited_realtime().stop();
    PokemonAutomation::GlobalThreadPools::unlimited_normal().stop();
    PokemonAutomation::global_dispatcher.stop();

    GlobalThreadPools::qt_worker_threadpool().stop();
    GlobalThreadPools::qt_event_threadpool().stop();

    std::cout << "Exiting main()..." << std::endl;

    return ret;
}

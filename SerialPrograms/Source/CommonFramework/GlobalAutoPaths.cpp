/*  Globals Auto-Paths
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Filesystem/Filesystem.h"
#include "GlobalAutoPaths.h"

#include <atomic>

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


namespace{


std::string get_resource_path(){
    //  Find the resource directory.
    Filesystem::Path base = Filesystem::application_install_path();
    Filesystem::Path path = base;
    for (size_t c = 0; c < 5; c++){
        Filesystem::Path try_path = path / "Resources/";
        if (exists(try_path)){
            return try_path.string_slash_normalized();
        }
        path = path / "..";
    }
    return (base / "Resources/").string_slash_normalized();
}
std::string get_unittest_resource_path(){
    //  Find the unit test resource directory.

    //  Try the intended folder name first.
    Filesystem::Path base = Filesystem::application_install_path();
    Filesystem::Path path = base;
    for (size_t c = 0; c < 5; c++){
        Filesystem::Path try_path = path / "UnitTestResources/";
        if (exists(try_path)){
            return try_path.string_slash_normalized();
        }
        path = path / "..";
    }

    //  Now try with the old command-line folder.
    path = base;
    for (size_t c = 0; c < 5; c++){
        Filesystem::Path try_path = path / "CommandLineTests/";
        if (exists(try_path)){
            return try_path.string_slash_normalized();
        }
        path = path / "..";
    }

    return (base / "UnitTestResources/").string_slash_normalized();
}

std::string get_training_path(){
    //  Find the training data directory.
    Filesystem::Path base = Filesystem::application_install_path();
    Filesystem::Path path = base;
    for (size_t c = 0; c < 5; c++){
        Filesystem::Path try_path = path / "TrainingData/";
        if (exists(try_path)){
            return try_path.string_slash_normalized();
        }
        path = path / "..";
    }

    return (base / "TrainingData/").string_slash_normalized();
}

std::string get_debug_path(){
    return RUNTIME_BASE_PATH() + "DebugDumps/";
}
std::string get_user_file_path(){
    return RUNTIME_BASE_PATH();
}

UserDataStorageLocation get_default_storage_location(){
#if defined(__APPLE__)
    return UserDataStorageLocation::USER_APP;
#else
    return UserDataStorageLocation::LOCAL;
#endif
}

std::string get_local_data_base_path(){
#if defined(__APPLE__)
    //  On macOS, the "local" user data folder is the folder that hosts the
    //  .app bundle. This matches where the program's working directory is set
    //  to at startup.
    return Filesystem::application_install_path().string_slash_normalized() + "/";
#else
    //  On Windows and Linux, the "local" user data folder is the current
    //  working directory (the folder the program runs from).
    return Filesystem::current_path().string_slash_normalized() + "/";
#endif
}

std::string get_user_app_data_base_path(){
    std::string path = Filesystem::application_user_data_path().string_slash_normalized();
    if (path.empty() || path.back() != '/'){
        path += '/';
    }
    return path;
}

std::string get_program_setting_json_file_name(){
    return Filesystem::application_binary_name().replace_extension().string() + "-Settings.json";
}

class UserDataStorageState{
public:
    UserDataStorageState()
        : m_settings(get_default_storage_location())
        , m_screenshots(get_default_storage_location())
    {}

    UserDataStorageLocation settings() const{
        return m_settings.load(std::memory_order_relaxed);
    }
    void set_settings(UserDataStorageLocation location){
        m_settings.store(location, std::memory_order_relaxed);
    }
    UserDataStorageLocation screenshots() const{
        return m_screenshots.load(std::memory_order_relaxed);
    }
    void set_screenshots(UserDataStorageLocation location){
        m_screenshots.store(location, std::memory_order_relaxed);
    }

private:
    std::atomic<UserDataStorageLocation> m_settings;
    std::atomic<UserDataStorageLocation> m_screenshots;
};

UserDataStorageState& user_data_storage_state(){
    static UserDataStorageState state;
    return state;
}

} // anonymous namespace




const std::string& RUNTIME_BASE_PATH(){
    static std::string path = Filesystem::application_scratch_path().string();
    return path;
}

UserDataStorageLocation DEFAULT_USER_DATA_STORAGE_LOCATION(){
    return get_default_storage_location();
}
const std::string& USER_DATA_STORAGE_LOCATION_DISPLAY_NAME(UserDataStorageLocation location){
    static const std::string user_app_name = "User App";
    static const std::string local_name = "Local";
    return location == UserDataStorageLocation::USER_APP ? user_app_name : local_name;
}
const std::string& LOCAL_DATA_BASE_PATH(){
    static const std::string path = get_local_data_base_path();
    return path;
}
const std::string& USER_APP_DATA_BASE_PATH(){
    static const std::string path = get_user_app_data_base_path();
    return path;
}
UserDataStorageLocation SETTINGS_STORAGE_LOCATION(){
    return user_data_storage_state().settings();
}
void SET_SETTINGS_STORAGE_LOCATION(UserDataStorageLocation location){
    UserDataStorageState& state = user_data_storage_state();
    if (state.settings() != location){
        state.set_settings(location);
        //  Create the setting folder at the new location so the user data can
        //  be saved there immediately. (Error report folders are created when
        //  they are first used.)
        Filesystem::create_directories(SETTINGS_PATH());
    }
}
UserDataStorageLocation SCREENSHOTS_STORAGE_LOCATION(){
    return user_data_storage_state().screenshots();
}
void SET_SCREENSHOTS_STORAGE_LOCATION(UserDataStorageLocation location){
    UserDataStorageState& state = user_data_storage_state();
    if (state.screenshots() != location){
        state.set_screenshots(location);
        //  Create the screenshots folder at the new location so screenshots
        //  can be saved there immediately.
        Filesystem::create_directories(SCREENSHOTS_PATH());
    }
}
const std::string& USER_DATA_BASE_PATH(){
    return SETTINGS_STORAGE_LOCATION() == UserDataStorageLocation::LOCAL
        ? LOCAL_DATA_BASE_PATH()
        : USER_APP_DATA_BASE_PATH();
}
const std::string& SCREENSHOTS_BASE_PATH(){
    return SCREENSHOTS_STORAGE_LOCATION() == UserDataStorageLocation::LOCAL
        ? LOCAL_DATA_BASE_PATH()
        : USER_APP_DATA_BASE_PATH();
}
void DETECT_SETTINGS_STORAGE_LOCATION(){
    const std::string file_name = get_program_setting_json_file_name();
    const std::string local_json = get_local_data_base_path() + "UserSettings/" + file_name;
    const std::string user_app_json = get_user_app_data_base_path() + "UserSettings/" + file_name;

    const bool local_exists = Filesystem::exists(local_json);
    const bool user_app_exists = Filesystem::exists(user_app_json);

    UserDataStorageLocation location = get_default_storage_location();
    if (local_exists && !user_app_exists){
        location = UserDataStorageLocation::LOCAL;
    }else if (user_app_exists && !local_exists){
        location = UserDataStorageLocation::USER_APP;
    }else if (local_exists && user_app_exists){
        //  Both candidate files exist (e.g. the user switched the storage
        //  location, and the older file was left behind). Use the one that
        //  was saved most recently.
        const auto local_time = Filesystem::last_write_time(local_json);
        const auto user_app_time = Filesystem::last_write_time(user_app_json);
        if (local_time > user_app_time){
            location = UserDataStorageLocation::LOCAL;
        }else if (user_app_time > local_time){
            location = UserDataStorageLocation::USER_APP;
        }
        //  If both have the same modification time, keep the platform default.
    }
    SET_SETTINGS_STORAGE_LOCATION(location);
}

const std::string& SETTINGS_PATH(){
    static const std::string local_path = LOCAL_DATA_BASE_PATH() + "UserSettings/";
    static const std::string user_app_path = USER_APP_DATA_BASE_PATH() + "UserSettings/";
    return SETTINGS_STORAGE_LOCATION() == UserDataStorageLocation::LOCAL ? local_path : user_app_path;
}
const std::string& PROGRAM_SETTING_JSON_FILE_NAME(){
    static const std::string name = get_program_setting_json_file_name();
    return name;
}
const std::string& PROGRAM_SETTING_JSON_PATH(){
    static const std::string local_path = LOCAL_DATA_BASE_PATH() + "UserSettings/" + PROGRAM_SETTING_JSON_FILE_NAME();
    static const std::string user_app_path = USER_APP_DATA_BASE_PATH() + "UserSettings/" + PROGRAM_SETTING_JSON_FILE_NAME();
    return SETTINGS_STORAGE_LOCATION() == UserDataStorageLocation::LOCAL ? local_path : user_app_path;
}
const std::string& SCREENSHOTS_PATH(){
    static const std::string local_path = LOCAL_DATA_BASE_PATH() + "Screenshots/";
    static const std::string user_app_path = USER_APP_DATA_BASE_PATH() + "Screenshots/";
    return SCREENSHOTS_STORAGE_LOCATION() == UserDataStorageLocation::LOCAL ? local_path : user_app_path;
}
const std::string& DEBUG_PATH(){
    static const std::string path = get_debug_path();
    return path;
}
const std::string& ERROR_PATH(){
    static const std::string local_path = LOCAL_DATA_BASE_PATH() + "ErrorDumps/";
    static const std::string user_app_path = USER_APP_DATA_BASE_PATH() + "ErrorDumps/";
    return SETTINGS_STORAGE_LOCATION() == UserDataStorageLocation::LOCAL ? local_path : user_app_path;
}
const std::string& UNSENT_ERROR_REPORTS_PATH(){
    //  The folder name "ErrorReportsLocal" matches ERROR_PATH_UNSENT in
    //  ErrorReports/ErrorReports.h.
    static const std::string local_path = LOCAL_DATA_BASE_PATH() + "ErrorReportsLocal/";
    static const std::string user_app_path = USER_APP_DATA_BASE_PATH() + "ErrorReportsLocal/";
    return SETTINGS_STORAGE_LOCATION() == UserDataStorageLocation::LOCAL ? local_path : user_app_path;
}
const std::string& SENT_ERROR_REPORTS_PATH(){
    //  The folder name "ErrorReportsSent" matches ERROR_PATH_SENT in
    //  ErrorReports/ErrorReports.h.
    static const std::string local_path = LOCAL_DATA_BASE_PATH() + "ErrorReportsSent/";
    static const std::string user_app_path = USER_APP_DATA_BASE_PATH() + "ErrorReportsSent/";
    return SETTINGS_STORAGE_LOCATION() == UserDataStorageLocation::LOCAL ? local_path : user_app_path;
}
const std::string& USER_FILE_PATH(){
    static std::string path = get_user_file_path();
    return path;
}
const std::string& RESOURCE_PATH(){
    static std::string path = get_resource_path();
    return path;
}
const std::string& DOWNLOADED_RESOURCE_PATH(){
    static std::string path = RUNTIME_BASE_PATH() + "DownloadedResources/";
    return path;
}
const std::string& UNIT_TEST_RESOURCE_PATH(){
    static std::string path = get_unittest_resource_path();
    return path;
}





const std::string& TRAINING_PATH(){
    static std::string path = get_training_path();
    return path;
}
const std::string& ML_ANNOTATION_PATH(){
    static const std::string path = RUNTIME_BASE_PATH() + "DataAnnotation/";
    return path;
}
const std::string& ML_MODEL_CACHE_PATH(){
    static const std::string path = RUNTIME_BASE_PATH() + "ModelCache/";
    return path;
}




}

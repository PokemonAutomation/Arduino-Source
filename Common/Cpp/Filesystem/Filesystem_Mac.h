/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <limits.h>
#include <vector>
#include <unistd.h>
#include <libproc.h>
#include <mach-o/dyld.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFBundle.h>
#include "FilePath.h"
#include "Filesystem.h"

namespace PokemonAutomation{
namespace Filesystem{



static std::string g_startup_profile;

void set_startup_profile(int& argc, char* argv[]){
    for (int i = 1; i + 1 < argc; i++){
        if (strcmp(argv[i], "--profile") == 0){
            std::string profile = argv[i + 1];
            for (char c : profile){
                if (!(std::isalpha(c) || std::isdigit(c)) && c != u'_' && c != u'-') c = u'_';
            }
            g_startup_profile = std::move(profile);
            // Shift everything after --profile <name> down by 2.
            for (int j = i; j + 2 < argc; j++){
                argv[j] = argv[j + 2];
            }
            argc -= 2;
            return;
        }
    }
}

const std::string& STARTUP_PROFILE(){
    return g_startup_profile;
}


// Helper function to safely get the current executable's name on macOS
std::string get_macos_executable_name() {
    char path_buffer[PROC_PIDPATHINFO_MAXSIZE];
    pid_t pid = getpid();

    if (proc_pidpath(pid, path_buffer, sizeof(path_buffer)) > 0) {
        Filesystem::Path full_path(path_buffer);
        return full_path.filename().string(); // Returns the binary name (e.g., "SerialPrograms")
    }

    return "UnknownApp"; // Worst-case fallback
}









Path application_binary_path(){
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        return std::filesystem::canonical(std::filesystem::path(buffer));
    }
    return Path();
}



std::filesystem::path CFString_to_path(CFStringRef cfString) {
    if (!cfString) {
        return {};
    }

    // 1. Get the exact buffer size needed for the file system representation
    CFIndex maxLength = CFStringGetMaximumSizeOfFileSystemRepresentation(cfString);

    // 2. Allocate a buffer to store the raw file system bytes
    std::vector<char> buffer(maxLength);

    // 3. Extract the file system representation directly into the buffer
    if (CFStringGetFileSystemRepresentation(cfString, buffer.data(), buffer.size())) {
        return std::filesystem::path(buffer.data());
    }

    // Fallback: If file-system specific translation failed, try standard UTF-8 text extraction
    CFIndex length = CFStringGetLength(cfString);
    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    std::vector<char> fallbackBuffer(maxSize);

    if (CFStringGetCString(cfString, fallbackBuffer.data(), fallbackBuffer.size(), kCFStringEncodingUTF8)) {
        return std::filesystem::path(fallbackBuffer.data());
    }

    return {};
}
Path application_install_path(){
    //  Use CFBundle to find the .app bundle path.
    //  Change working directory to the folder that hosts the .app bundle.
    CFURLRef bundleURL = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    if (bundleURL){
        CFStringRef cfPath = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);
        CFRelease(bundleURL);
        if (cfPath){
            std::filesystem::path bundlePath = CFString_to_path(cfPath);
            CFRelease(cfPath);
            if (bundlePath.extension() == ".app"){
                return std::filesystem::absolute(bundlePath).parent_path().lexically_normal();
            }
        }
    }

    //  Fallback
    return application_binary_directory();
}
Path application_scratch_path(){
    // 1. Replicate Home folder resolution (~/)
    const char* home_env = std::getenv("HOME");
    Filesystem::Path app_support_path;

    if (home_env != nullptr) {
        app_support_path = Filesystem::Path(home_env);
    } else {
        app_support_path = Filesystem::current_path();
    }

    // 2. Replicate standard macOS AppDataLocation structure programmatically
    std::string app_name = get_macos_executable_name();
    app_support_path = app_support_path / "Library" / "Application Support" / app_name;

    // 3. Append your profile path
    if (!g_startup_profile.empty()) {
        app_support_path /= "Profiles";
        app_support_path /= g_startup_profile;
    }

    // 4. Create directory structure
    Filesystem::create_directories(app_support_path);

    return app_support_path.string() + "/";
}



}
}

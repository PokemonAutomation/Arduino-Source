/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <limits.h>
#include <mach-o/dyld.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFBundle.h>
#include "FilePath.h"

namespace PokemonAutomation{
namespace Filesystem{


Path application_binary_path(){
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        return std::filesystem::canonical(std::filesystem::path(buffer)).parent_path();
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
Path application_working_path(){
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
                return std::filesystem::absolute(bundlePath).lexically_normal();
            }
        }
    }

    //  Fallback
    return application_binary_path();
}



}
}

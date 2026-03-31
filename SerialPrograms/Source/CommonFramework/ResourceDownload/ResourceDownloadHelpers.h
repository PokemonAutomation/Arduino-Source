/*  Resource Download Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadHelpers_H
#define PokemonAutomation_ResourceDownloadHelpers_H

#include <string>
#include <optional>


namespace PokemonAutomation{

namespace Filesystem{
    class Path;
}


enum class ResourceType{
    ZIP_FILE,
};

struct DownloadedResourceMetadata{
    std::string resource_name;
    std::optional<uint16_t> version_num;
    ResourceType resource_type;
    size_t size_compressed_bytes;
    size_t size_decompressed_bytes;
    std::string url;
};

enum class ResourceVersionStatus{
    CURRENT,
    OUTDATED, // still used, but newer version available
    FUTURE_VERSION, // current version number is greater than the expected version number
    NOT_APPLICABLE, // resource not downloaded locally, so can't get its version
    // RETIRED, // no longer used
    // BLANK, // not yet fetched version info from remote
};

enum class RemoteMetadataStatus{
    UNINITIALIZED,
    NOT_AVAILABLE,
    AVAILABLE,
};
struct RemoteMetadata {
    RemoteMetadataStatus status = RemoteMetadataStatus::UNINITIALIZED;
    DownloadedResourceMetadata metadata;
};


const std::vector<DownloadedResourceMetadata>& local_resource_download_list();
const std::vector<DownloadedResourceMetadata>& remote_resource_download_list();
std::optional<uint16_t> get_resource_version_num(Filesystem::Path folder_path);
ResourceVersionStatus get_version_status(uint16_t expected_version_num, std::optional<uint16_t> current_version_num);

}
#endif

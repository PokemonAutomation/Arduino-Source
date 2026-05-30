/*  Resource Download Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadHelpers_H
#define PokemonAutomation_ResourceDownloadHelpers_H

#include <string>
#include <vector>
#include <unordered_set>
#include <optional>


namespace PokemonAutomation{

namespace Filesystem{
    class Path;
}



struct DownloadedResourceMetadata{
    std::string resource_name;
    std::optional<uint16_t> version_num;
    uint64_t size_compressed_bytes;
    uint64_t size_decompressed_bytes;
    std::string url;
    std::string sha256;
};

enum class ResourceVersionStatus{
    CURRENT,
    OUTDATED, // still used, but newer version available
    FUTURE_VERSION, // current version number is greater than the expected version number
    NOT_APPLICABLE, // resource not downloaded locally, so can't get its version
    // RETIRED, // no longer used
};


// there are three lists:
// - local_resource_download_list(): List of resources with the version numbers that the programs expect. from the local ResourceDownloadList.json
// - list of resources downloaded locally. use get_version_status() to determine ResourceVersionStatus relative to the expected resource version number in the local list.
// - remote_resource_download_list(): list of remote resources. the remote version numbers may or may not match the local list. from the remote ResourceDownloadList.json

const std::vector<DownloadedResourceMetadata>& local_resource_download_list();
const std::vector<DownloadedResourceMetadata>& remote_resource_download_list();
std::optional<uint16_t> get_resource_version_num(Filesystem::Path folder_path);

// for the given expected_resource, it tries to find its corresponding file downloaded locally.
// it then returns the ResourceVersionStatus, which is the result of comparing the expected version number
// (from the expected_resource) to the actual version number (from the downloaded file).
// expected_resource is one of the items from local_resource_download_list(), which is a list of 
// resources with the version numbers that the programs expect (from the local ResourceDownloadList.json).
ResourceVersionStatus get_version_status(DownloadedResourceMetadata& expected_resource);
ResourceVersionStatus compare_version_num(uint16_t expected_version_num, std::optional<uint16_t> current_version_num);


// ASSUMES: given resource_list has every resource_type within it
DownloadedResourceMetadata get_resource_metadata_from_resource_type(const std::string& target_resource_type, const std::vector<DownloadedResourceMetadata>& resource_list);


bool is_resource_ready_in_queue(uint16_t max_concurrent_downloads, const std::string& resource_slug, const std::vector<std::string>& download_queue);

const std::unordered_set<std::string>& all_resource_names();

}
#endif

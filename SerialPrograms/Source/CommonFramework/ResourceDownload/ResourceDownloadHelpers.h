/*  Resource Download Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadHelpers_H
#define PokemonAutomation_ResourceDownloadHelpers_H

#include <optional>
#include <string>
#include <vector>
#include <unordered_set>


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


struct ResourceVersionInfo{
    bool is_downloaded;
    ResourceVersionStatus version_status;
    std::optional<uint16_t> current_version_num;
};

// there are three lists:
// - expected_resource_download_list(): List of resources with the version numbers that the programs expect. from the local ResourceDownloadList.json
// - list of resources downloaded locally. use get_local_version_info() to determine ResourceVersionStatus relative to the expected resource version number.
// - remote_resource_download_list(): list of remote resources. the remote version numbers may or may not match the local list. from the remote ResourceDownloadList.json

const std::vector<DownloadedResourceMetadata>& expected_resource_download_list();

const std::vector<DownloadedResourceMetadata>& remote_resource_download_list();

// - This returns the version information for a resource that has been downloaded locally.
// - This returns a struct containing a boolean representing whether the resource has been downloaded, 
// the resource's version status (e.g. CURRENT, OUTDATED, NOT_APPLICABLE etc.), and its version number
// - for the version status, it compares the version of the locally downloaded file to the 
// corresponding expected_resource from expected_resource_download_list()
ResourceVersionInfo get_local_version_info(const std::string& target_resource_slug);


// - throws OperationFailedException if target_resource_slug isn't found within remote_resource_download_list
// this would indicate that CC is out of date.
// - also throws OperationFailedException if Internet is not turned on.
DownloadedResourceMetadata get_remote_resource_metadata_from_resource_slug(const std::string& target_resource_slug);

// ASSUMES: given target_resource_slug is listed within expected_resource_download_list().
// PanelInstance::validate_resource_list() should ensure that target_resource_slug is valid.
DownloadedResourceMetadata get_expected_resource_metadata_from_resource_slug(const std::string& target_resource_slug);


const std::unordered_set<std::string>& all_resource_names();

}
#endif

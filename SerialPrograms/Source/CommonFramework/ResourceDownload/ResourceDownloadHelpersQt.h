/*  Resource Download Helpers using Qt headers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadHelpersQt_H
#define PokemonAutomation_ResourceDownloadHelpersQt_H

#include <optional>
#include <string>
#include <vector>
#include <unordered_set>
#include "ResourceDownloadHelpers.h"


namespace PokemonAutomation{


const std::vector<DownloadedResourceMetadata>& remote_resource_download_list();


// - throws OperationFailedException if target_resource_slug isn't found within remote_resource_download_list
// this would indicate that CC is out of date.
// - also throws OperationFailedException if Internet is not turned on.
DownloadedResourceMetadata get_remote_resource_metadata_from_resource_slug(const std::string& target_resource_slug);


}
#endif

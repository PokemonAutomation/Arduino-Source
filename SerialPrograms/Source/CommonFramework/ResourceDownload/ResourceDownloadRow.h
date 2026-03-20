/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadRow_H
#define PokemonAutomation_ResourceDownloadRow_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/StaticTableOption.h"
#include <optional>


namespace PokemonAutomation{

class ResourceDownloadRow;

class ResourceDownloadButton : public ConfigOptionImpl<ResourceDownloadButton>{
public:
    ResourceDownloadButton(ResourceDownloadRow& p_row);

    ResourceDownloadRow& row;
};

class ResourceDeleteButton : public ConfigOptionImpl<ResourceDeleteButton>{
public:
    ResourceDeleteButton(ResourceDownloadRow& p_row);

    ResourceDownloadRow& row;
};

enum class ResourceVersionStatus{
    CURRENT,
    OUTDATED, // still used, but newer version available
    FUTURE_VERSION, // current version number is greater than the expected version number
    NOT_APPLICABLE, // resource not downloaded locally, so can't get its version
    // RETIRED, // no longer used
    // BLANK, // not yet fetched version info from remote
};

class ResourceDownloadRow : public StaticTableRow{

public:
    ~ResourceDownloadRow();
    ResourceDownloadRow(
        std::string&& resource_name,
        size_t file_size,
        bool is_downloaded,
        std::optional<uint16_t> version_num,
        ResourceVersionStatus version_status
    );

public:
    struct Data;
    Pimpl<Data> m_data;

    ResourceDownloadButton m_download_button;
    ResourceDeleteButton m_delete_button;

};

}
#endif

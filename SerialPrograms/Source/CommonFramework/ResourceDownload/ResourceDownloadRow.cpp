/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */



#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Options/LabelCellOption.h"
#include "ResourceDownloadRow.h"


// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{


ResourceDownloadButton::ResourceDownloadButton(ResourceDownloadRow& p_row)
    : ConfigOptionImpl<ResourceDownloadButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
{}

ResourceDeleteButton::ResourceDeleteButton(ResourceDownloadRow& p_row)
    : ConfigOptionImpl<ResourceDeleteButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
{}

std::string resource_version_to_string(ResourceVersionStatus version){
    switch(version){
    case ResourceVersionStatus::CURRENT:
        return "Current";
    case ResourceVersionStatus::OUTDATED:
        return "Outdated";
    case ResourceVersionStatus::NOT_APPLICABLE:
        return "--";
    // case ResourceVersionStatus::BLANK:
    //     return "";
    case ResourceVersionStatus::FUTURE_VERSION:
        return "Unsupported future version.<br>Please update the Computer Control program.";
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "resource_version_to_string: Unknown enum.");  
    }
}

struct ResourceDownloadRow::Data{
    Data(
        std::string&& resource_name,
        size_t file_size,
        bool is_downloaded,
        std::optional<uint16_t> version_num,
        ResourceVersionStatus version_status
    )
        : m_resource_name(LockMode::LOCK_WHILE_RUNNING, resource_name)
        , m_file_size(file_size)
        , m_file_size_label(LockMode::LOCK_WHILE_RUNNING, std::to_string(file_size))
        , m_is_downloaded(is_downloaded)
        , m_is_downloaded_label(LockMode::LOCK_WHILE_RUNNING, is_downloaded ? "Yes" : "--")
        , m_version_num(version_num)
        , m_version_status(version_status)
        , m_version_status_label(LockMode::LOCK_WHILE_RUNNING, resource_version_to_string(version_status))
    {}

    LabelCellOption m_resource_name;

    size_t m_file_size;
    LabelCellOption m_file_size_label;

    bool m_is_downloaded;
    LabelCellOption m_is_downloaded_label;

    std::optional<uint16_t> m_version_num;
    ResourceVersionStatus m_version_status;
    LabelCellOption m_version_status_label;


};


ResourceDownloadRow::~ResourceDownloadRow(){}
ResourceDownloadRow::ResourceDownloadRow(
    std::string&& resource_name,
    size_t file_size,
    bool is_downloaded,
    std::optional<uint16_t> version_num,
    ResourceVersionStatus version_status
)
    : StaticTableRow(resource_name)
    , m_data(CONSTRUCT_TOKEN, std::move(resource_name), file_size, is_downloaded, version_num, version_status)
    , m_download_button(*this)
    , m_delete_button(*this)
{
    PA_ADD_STATIC(m_data->m_resource_name);
    PA_ADD_STATIC(m_data->m_file_size_label);
    PA_ADD_STATIC(m_data->m_is_downloaded_label);
    PA_ADD_STATIC(m_data->m_version_status_label);

    PA_ADD_STATIC(m_download_button);
    PA_ADD_STATIC(m_delete_button);
}


}

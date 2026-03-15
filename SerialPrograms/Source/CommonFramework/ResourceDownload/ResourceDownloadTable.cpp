/*  Resource Download Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "ResourceDownloadTable.h"

namespace PokemonAutomation{

DownloadButton::DownloadButton(ResourceDownloadRow& p_row)
    : ConfigOptionImpl<DownloadButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , option(p_row)
{}    


ResourceDownloadRow::ResourceDownloadRow(
    std::string&& resource_name,
    size_t file_size
)
    : StaticTableRow(resource_name)
    , m_resource_name(LockMode::LOCK_WHILE_RUNNING, resource_name)
    , m_file_size(file_size)
    , m_file_size_label(LockMode::LOCK_WHILE_RUNNING, std::to_string(file_size))
    , m_download_button(*this)
{
    PA_ADD_STATIC(m_resource_name);
    PA_ADD_STATIC(m_file_size_label);
    // PA_ADD_STATIC(m_download_button);
}

ResourceDownloadTable::ResourceDownloadTable()
    : StaticTableOption("<b>Resource Downloading:</b><br>Download resources not included in the initial download of the program.", LockMode::LOCK_WHILE_RUNNING, false)
{
    add_row(std::make_unique<ResourceDownloadRow>("PaddleOCR", 1000000));

    finish_construction();
}
std::vector<std::string> ResourceDownloadTable::make_header() const{
    std::vector<std::string> ret{
        "Resource",
        "Size"
    };
    return ret;
}




}

/*  Resource Download Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadTable_H
#define PokemonAutomation_ResourceDownloadTable_H

#include "CommonFramework/Options/LabelCellOption.h"
#include "Common/Cpp/Options/StaticTableOption.h"

namespace PokemonAutomation{


class ResourceDownloadRow;
class ResourceDownloadButton : public ConfigOptionImpl<ResourceDownloadButton>{
public:
    ResourceDownloadButton(ResourceDownloadRow& p_row);

    ResourceDownloadRow& option;
};

class ResourceDeleteButton : public ConfigOptionImpl<ResourceDeleteButton>{
public:
    ResourceDeleteButton(ResourceDownloadRow& p_row);

    ResourceDownloadRow& option;
};

class ResourceDownloadRow : public StaticTableRow{

public:
    // ~ResourceDownloadRow();
    ResourceDownloadRow(
        std::string&& resource_name,
        bool is_downloaded,
        size_t file_size
    );

    LabelCellOption m_resource_name;
    bool m_is_downloaded;
    LabelCellOption m_is_downloaded_label;
    size_t m_file_size;
    LabelCellOption m_file_size_label;
    ResourceDownloadButton m_download_button;
    ResourceDeleteButton m_delete_button;


};


enum class ResourceType{
    ZIP_FILE,
};
struct DownloadedResource{
    std::string resource_name;
    ResourceType resource_type;
    size_t size_compressed_bytes;
    size_t size_decompressed_bytes;
    std::string url;
};

class ResourceDownloadTable : public StaticTableOption{
public:
    ResourceDownloadTable();

    virtual std::vector<std::string> make_header() const override;

    std::vector<DownloadedResource> deserialize_resource_list_json();
    std::vector<std::unique_ptr<ResourceDownloadRow>> get_resource_download_rows();
    void add_resource_download_rows();


private:
    std::vector<DownloadedResource> m_resources;

    // we need to keep a handle on each Row, so that we can edit m_is_downloaded_label later on.
    std::vector<std::unique_ptr<ResourceDownloadRow>> m_resource_rows;

};






}
#endif

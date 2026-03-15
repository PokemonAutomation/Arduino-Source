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
        size_t file_size
    );

    LabelCellOption m_resource_name;
    size_t m_file_size;
    LabelCellOption m_file_size_label;
    ResourceDownloadButton m_download_button;
    ResourceDeleteButton m_delete_button;


};


class ResourceDownloadTable : public StaticTableOption{
public:
    ResourceDownloadTable();

    virtual std::vector<std::string> make_header() const override;

};




}
#endif

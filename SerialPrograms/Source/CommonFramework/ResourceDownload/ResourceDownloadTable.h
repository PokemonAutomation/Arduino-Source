/*  Resource Download Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadTable_H
#define PokemonAutomation_ResourceDownloadTable_H

#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/StaticTableOption.h"

namespace PokemonAutomation{

class JsonValue;
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
    RETIRED, // no longer used
    NOT_APPLICABLE, // resource not downloaded locally, so can't get its version
    BLANK, // not yet fetched version info from remote
};

class ResourceDownloadRow : public StaticTableRow{

public:
    ~ResourceDownloadRow();
    ResourceDownloadRow(
        std::string&& resource_name,
        size_t file_size,
        bool is_downloaded,
        size_t version_num
    );

private:
    static std::string resource_version_to_string(ResourceVersionStatus version);

public:
    struct Data;
    Pimpl<Data> m_data;

    ResourceDownloadButton m_download_button;
    ResourceDeleteButton m_delete_button;

};


enum class ResourceType{
    ZIP_FILE,
};
struct DownloadedResource{
    std::string resource_name;
    size_t version_num;
    ResourceType resource_type;
    size_t size_compressed_bytes;
    size_t size_decompressed_bytes;
    std::string url;
};

class ResourceDownloadTable : public StaticTableOption{
public:
    ~ResourceDownloadTable();
    ResourceDownloadTable();

    virtual std::vector<std::string> make_header() const override;
    virtual UiWrapper make_UiComponent(void* params) override;

private:  
    std::vector<DownloadedResource> deserialize_resource_list_json(const JsonValue& json);
    std::vector<std::unique_ptr<ResourceDownloadRow>> get_resource_download_rows();
    void add_resource_download_rows();

    void check_all_resource_versions();


private:
    std::vector<DownloadedResource> m_resources;

    // we need to keep a handle on each Row, so that we can edit m_is_downloaded_label later on.
    std::vector<std::unique_ptr<ResourceDownloadRow>> m_resource_rows;

    AsyncTask m_worker;

};






}
#endif

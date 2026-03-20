/*  Resource Download Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadTable_H
#define PokemonAutomation_ResourceDownloadTable_H

#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "ResourceDownloadRow.h"

namespace PokemonAutomation{


enum class ResourceType{
    ZIP_FILE,
};
struct DownloadedResource{
    std::string resource_name;
    std::optional<uint16_t> version_num;
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
    // virtual UiWrapper make_UiComponent(void* params) override;

private:  
    void add_resource_download_rows();


private:
    // we need to keep a handle on each Row, so that we can edit m_is_downloaded_label later on.
    std::vector<std::unique_ptr<ResourceDownloadRow>> m_resource_rows;

    AsyncTask m_worker;

};






}
#endif

/*  Resource Download Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadTable_H
#define PokemonAutomation_ResourceDownloadTable_H

#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Options/StaticTableOption.h"
// #include "ResourceDownloadRow.h"

namespace PokemonAutomation{

class ResourceDownloadRow;

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

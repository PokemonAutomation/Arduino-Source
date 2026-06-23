/*  Resource Download Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadTable_H
#define PokemonAutomation_ResourceDownloadTable_H

#include <deque>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Options/StaticTableOption.h"
// #include "SettingsResourceDownloadRow.h"

namespace PokemonAutomation{

class SettingsResourceDownloadRow;
class ResourceDownload;

class SettingsResourceDownloadTable : public StaticTableOption{
public:
    ~SettingsResourceDownloadTable();
    SettingsResourceDownloadTable();

    virtual std::vector<std::string> make_header() const override;

    void connect_row_with_download(const std::string& resource_slug, std::shared_ptr<ResourceDownload>& download_ptr);

private:  
    std::vector<std::unique_ptr<SettingsResourceDownloadRow>> get_resource_download_rows();
    void add_resource_download_rows();


private:
    // we need to keep a handle on each Row, so that we can edit m_is_downloaded_label later on.
    std::vector<std::unique_ptr<SettingsResourceDownloadRow>> m_resource_rows;

};



}
#endif

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
// #include "ResourceDownloadRow.h"

namespace PokemonAutomation{

class ResourceDownloadRow;

class ResourceDownloadTable : public StaticTableOption{
public:
    ~ResourceDownloadTable();
    ResourceDownloadTable();

    virtual std::vector<std::string> make_header() const override;
    // virtual UiWrapper make_UiComponent(void* params) override;

    void add_row_to_download_list(const std::string& resource_slug);
    void remove_row_from_download_list(const std::string& resource_slug);

    // return true if given resource_slug's position in m_download_queue is less than MAX_CONCURRENT_DOWNLOADS
    // ASSUMES: the calling thread holds the m_lock. therefore, this function doesn't lock the mutex when accessing m_download_queue.
    bool is_download_ready_to_start(const std::string& resource_slug);

    void update_row_status(const std::string& target_slug, bool success);

private:  
    std::vector<std::unique_ptr<ResourceDownloadRow>> get_resource_download_rows();
    void add_resource_download_rows();


private:
    // we need to keep a handle on each Row, so that we can edit m_is_downloaded_label later on.
    std::vector<std::unique_ptr<ResourceDownloadRow>> m_resource_rows;

    // queue of downloads
    // each download is represented by its slug, 
    std::vector<std::string> m_download_queue;

    Mutex m_lock;
    ConditionVariable m_cv;

    // AsyncTask m_worker;

};



}
#endif

/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadRow_H
#define PokemonAutomation_ResourceDownloadRow_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
// #include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/LifetimeSanitizer.h"
// #include "CommonFramework/Tools/GlobalThreadPools.h"
#include "Common/Cpp/Options/StaticTableOption.h"
#include "ResourceDownloadHelpers.h"
#include "DownloadThread.h"
#include "ResourceDownloadOptions.h"
#include "ResourceDownloadTable.h"
// #include <mutex>

namespace PokemonAutomation{


enum class ActionState{
    PRE_DOWNLOAD,
    DOWNLOADING,
    PRE_DELETE,
    DELETING,
    PRE_CANCEL,
    CANCELLING,
    READY,
};
class ResourceDownloadRow : public StaticTableRow{
public:
    ~ResourceDownloadRow();
    ResourceDownloadRow(
        ResourceDownloadTable& parent_table,
        Mutex& lock,
        ConditionVariable& cv,
        std::string resource_slug,
        DownloadedResourceMetadata local_metadata,
        bool is_downloaded,
        std::optional<uint16_t> version_num,
        ResourceVersionStatus version_status
    );

public:
    struct DownloadListener{
        virtual void on_download_progress(uint64_t bytes_done, uint64_t total_bytes){}
        virtual void on_unzip_progress(uint64_t bytes_done, uint64_t total_bytes){}
        virtual void on_hash_progress(uint64_t bytes_done, uint64_t total_bytes){}

        virtual void on_metadata_fetch_finished(const std::string& popup_message){}
        virtual void on_exception_caught(const std::string& function_name){}
        virtual void on_download_failed(){}

        virtual void on_action_state_updated(){}
    };

    void add_listener(DownloadListener& listener);
    void remove_listener(DownloadListener& listener);

    void report_download_progress(uint64_t bytes_done, uint64_t total_bytes);
    void report_unzip_progress(uint64_t bytes_done, uint64_t total_bytes);
    void report_hash_progress(uint64_t bytes_done, uint64_t total_bytes);

    void report_metadata_fetch_finished(const std::string& popup_message);
    void report_exception_caught(const std::string& function_name);
    void report_download_failed();

    void report_action_state_updated();


public:
    std::string get_resource_slug(){ return m_resource_slug; }
    void set_version_status(ResourceVersionStatus version_status);
    void set_is_downloaded(bool is_downloaded);
    void update_table_label(bool success);

    void ensure_remote_metadata_loaded();
    std::string predownload_warning_summary(const DownloadedResourceMetadata& remote_metadata);
    // get the DownloadedResourceMetadata from the remote JSON, that corresponds to this button/row
    const DownloadedResourceMetadata& fetch_remote_metadata();
    // DownloadedResourceMetadata initialize_local_metadata();

    void start_download();
    
    void on_download_finished();

    void start_delete();

    // READY: can come from any state
    // PRE_DOWNLOAD, PRE_DELETE, PRE_CANCEL: can only come from READY
    // DELETING, CANCELLING: can only come from their respective PRE state
    // DOWNLOADING: can come from either PRE_DOWNLOAD or PRE_CANCEL
    void update_action_state(ActionState state);

    ActionState get_action_state();

    bool is_download_ready_to_start();
    void remove_self_from_download_queue();    

    bool is_given_action_state(ActionState state);

    void cancel_download_thread();

private:
    ResourceDownloadTable& m_parent_table;
    Mutex& m_download_lock;
    ConditionVariable& m_download_cv;

    ActionState m_action_state;
    std::string m_resource_slug;
    DownloadedResourceMetadata m_local_metadata;
    struct Data;
    Pimpl<Data> m_data;

    ResourceDownloadButton m_download_button;
    ResourceDeleteButton m_delete_button;
    ResourceCancelButton m_cancel_button;
    ResourceProgressBar m_progress_bar;

    AsyncTask m_pre_download_thread;
    AsyncTask m_delete_thread;

    std::optional<DownloadedResourceMetadata> m_cached_metadata;

    std::unique_ptr<DownloadThread> m_download_thread;

    Mutex m_action_state_lock;

    LifetimeSanitizer m_lifetime_sanitizer;




};

}
#endif

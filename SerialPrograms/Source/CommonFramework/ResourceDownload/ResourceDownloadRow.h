/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadRow_H
#define PokemonAutomation_ResourceDownloadRow_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/LifetimeSanitizer.h"
// #include "CommonFramework/Tools/GlobalThreadPools.h"
#include "Common/Cpp/Options/StaticTableOption.h"
#include "ResourceDownloadHelpers.h"
#include "ResourceDownloadOptions.h"
#include <mutex>

namespace PokemonAutomation{


// must be initialized as shared_ptr, so that `shared_from_this` can work
// so you're forced to use the factory method `create`
class DownloadThread : public CancellableScope, public std::enable_shared_from_this<DownloadThread> {

private:
    struct ConstructorKey { 
        explicit ConstructorKey() = default; 
    };

public:
    ~DownloadThread();
    DownloadThread(ConstructorKey, ResourceDownloadRow& row);

public:
    // factor method to initialize DownloadThread, since it must be a shared_ptr
    static std::shared_ptr<DownloadThread> create(ResourceDownloadRow& row);
    
    void start_download_thread();

    // throws OperationCancelledException if the user cancels the action
    void run_download(DownloadedResourceMetadata resource_metadata);

    

private:
    ResourceDownloadRow& m_row;
    AsyncTask m_worker;
};

enum class ButtonState{
    DOWNLOAD,
    DELETE,
    CANCEL,
    READY,
};
class ResourceDownloadRow : public StaticTableRow{
public:
    ~ResourceDownloadRow();
    ResourceDownloadRow(
        DownloadedResourceMetadata local_metadata,
        bool is_downloaded,
        std::optional<uint16_t> version_num,
        ResourceVersionStatus version_status
    );

public:
    struct DownloadListener{
        virtual void on_download_progress(int percentage){};
        virtual void on_unzip_progress(int percentage){};
        virtual void on_hash_progress(int percentage){};

        virtual void on_metadata_fetch_finished(const std::string& popup_message){};
        virtual void on_exception_caught(const std::string& function_name){};
        virtual void on_download_failed(){};

        virtual void on_button_state_updated(){};
    };

    void add_listener(DownloadListener& listener);
    void remove_listener(DownloadListener& listener);

    void report_download_progress(int percentage);
    void report_unzip_progress(int percentage);
    void report_hash_progress(int percentage);

    void report_metadata_fetch_finished(const std::string& popup_message);
    void report_exception_caught(const std::string& function_name);
    void report_download_failed();

    void report_button_state_updated();


public:
    void set_version_status(ResourceVersionStatus version_status);
    void set_is_downloaded(bool is_downloaded);
    void update_table_label(bool success);

    void ensure_remote_metadata_loaded();
    std::string predownload_warning_summary(RemoteMetadata& remote_metadata);
    // get the DownloadedResourceMetadata from the remote JSON, that corresponds to this button/row
    void initialize_remote_metadata();
    RemoteMetadata& fetch_remote_metadata();
    // DownloadedResourceMetadata initialize_local_metadata();

    void start_download();
    
    void on_download_finished();

    void start_delete();

    void update_button_state(ButtonState state);

    inline ButtonState get_button_state(){ return m_button_state; }
    

private:
    std::once_flag init_flag;
    std::unique_ptr<RemoteMetadata> m_remote_metadata;

    ButtonState m_button_state;
    DownloadedResourceMetadata m_local_metadata;
    struct Data;
    Pimpl<Data> m_data;

    ResourceDownloadButton m_download_button;
    ResourceDeleteButton m_delete_button;
    ResourceCancelButton m_cancel_button;
    ResourceProgressBar m_progress_bar;

    AsyncTask m_worker1;
    AsyncTask m_worker2;

    std::shared_ptr<DownloadThread> m_download_thread;

    std::mutex m_thread_mutex;

    LifetimeSanitizer m_lifetime_sanitizer;




};

}
#endif

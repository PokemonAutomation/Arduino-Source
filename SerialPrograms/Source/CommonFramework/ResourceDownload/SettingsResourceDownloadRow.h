/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadRow_H
#define PokemonAutomation_ResourceDownloadRow_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/Cpp/Options/StaticTableOption.h"
#include "ResourceDownload.h"
#include "SettingsResourceDownloadOptions.h"

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
class SettingsResourceDownloadRow : public StaticTableRow, public ResourceDownload::Listener{
public:
    ~SettingsResourceDownloadRow();
    SettingsResourceDownloadRow(
        DownloadedResourceMetadata local_metadata,
        bool is_downloaded,
        std::optional<uint16_t> version_num,
        ResourceVersionStatus version_status
    );

public:
    struct Listener{
        virtual void on_download_progress(uint64_t bytes_done, uint64_t total_bytes){}
        virtual void on_unzip_progress(uint64_t bytes_done, uint64_t total_bytes){}
        virtual void on_hash_progress(uint64_t bytes_done, uint64_t total_bytes){}

        virtual void on_metadata_fetch_finished(const std::string& popup_message){}

        virtual void on_action_state_updated(){}
    };

    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);

    void report_download_progress(uint64_t bytes_done, uint64_t total_bytes);
    void report_unzip_progress(uint64_t bytes_done, uint64_t total_bytes);
    void report_hash_progress(uint64_t bytes_done, uint64_t total_bytes);

    void report_metadata_fetch_finished(const std::string& popup_message);

    void report_action_state_updated();

public: // ResourceDownload::Listener
    virtual void on_download_progress(uint64_t bytes_done, uint64_t total_bytes) override;
    virtual void on_unzip_progress(uint64_t bytes_done, uint64_t total_bytes) override;
    virtual void on_hash_progress(uint64_t bytes_done, uint64_t total_bytes) override;

    // NOTE: this runs regardless of success or failure
    virtual void on_download_finished(bool success, const std::string& resource_slug) override;

public:
    std::string get_resource_slug();
    void set_version_status(ResourceVersionStatus version_status);
    void set_is_downloaded(bool is_downloaded);
    void update_table_label(bool success);

    void ensure_remote_metadata_loaded();
    std::string predownload_warning_summary(const DownloadedResourceMetadata& remote_metadata);
    // get the DownloadedResourceMetadata from the remote JSON, that corresponds to this button/row
    const DownloadedResourceMetadata& fetch_remote_metadata();
    // DownloadedResourceMetadata initialize_local_metadata();

    void start_download();
    
    void start_delete();

    // READY: can come from any state
    // PRE_DOWNLOAD, PRE_DELETE, PRE_CANCEL: can only come from READY
    // DELETING, CANCELLING: can only come from their respective PRE state
    // DOWNLOADING: can come from either PRE_DOWNLOAD or PRE_CANCEL
    void update_action_state(ActionState state);

    ActionState get_action_state();

    bool is_given_action_state(ActionState state);

    void cancel_download_thread();

    // - updates m_download_ptr to point to the given download_ptr
    // - adds this object as a listener to download_ptr
    void connect_with_download(std::shared_ptr<ResourceDownload> download_ptr);

private:
    struct Data;
    Pimpl<Data> m_data;

    SettingsResourceDownloadButton m_download_button;
    SettingsResourceDeleteButton m_delete_button;
    SettingsResourceCancelButton m_cancel_button;
    SettingsResourceProgressBar m_progress_bar;

    LifetimeSanitizer m_lifetime_sanitizer;




};

}
#endif

/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadRow_H
#define PokemonAutomation_ResourceDownloadRow_H

#include <QObject>
#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
// #include "CommonFramework/Tools/GlobalThreadPools.h"
#include "Common/Cpp/Options/StaticTableOption.h"
#include "ResourceDownloadHelpers.h"
#include "ResourceDownloadOptions.h"


namespace PokemonAutomation{


class ResourceDownloadRow : public QObject, public StaticTableRow{
    Q_OBJECT
public:
    ~ResourceDownloadRow();
    ResourceDownloadRow(
        DownloadedResourceMetadata local_metadata,
        bool is_downloaded,
        std::optional<uint16_t> version_num,
        ResourceVersionStatus version_status
    );

signals:
    void download_progress(int percentage);
    void unzip_progress(int percentage);

    void metadata_fetch_finished(std::string popup_message);
    void exception_caught(std::string function_name);
    void download_failed();
    void download_finished();


public:
    void set_version_status(ResourceVersionStatus version_status);
    void set_is_downloaded(bool is_downloaded);
    void set_cancel_action(bool cancel_action);

    void actions_done_reenable_buttons();

    void ensure_remote_metadata_loaded();
    std::string predownload_warning_summary(RemoteMetadata& remote_metadata);
    // get the DownloadedResourceMetadata from the remote JSON, that corresponds to this button/row
    void initialize_remote_metadata();
    RemoteMetadata& fetch_remote_metadata();
    // DownloadedResourceMetadata initialize_local_metadata();

    void start_download();
    // throws OperationCancelledException if the user cancels the action
    void run_download(DownloadedResourceMetadata resource_metadata);
    

public:
    DownloadedResourceMetadata m_local_metadata;

private:
    std::once_flag init_flag;
    std::unique_ptr<RemoteMetadata> m_remote_metadata;

    struct Data;
    Pimpl<Data> m_data;

    ResourceDownloadButton m_download_button;
    ResourceDeleteButton m_delete_button;
    ResourceCancelButton m_cancel_button;
    ResourceProgressBar m_progress_bar;

    AsyncTask m_worker1;
    AsyncTask m_worker2;




};

}
#endif

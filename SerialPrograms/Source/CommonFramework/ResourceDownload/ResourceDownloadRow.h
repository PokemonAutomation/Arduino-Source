/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadRow_H
#define PokemonAutomation_ResourceDownloadRow_H

#include <QObject>
#include "Common/Cpp/Containers/Pimpl.h"
// #include "Common/Cpp/Concurrency/AsyncTask.h"
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
        std::string&& resource_name,
        size_t file_size,
        bool is_downloaded,
        std::optional<uint16_t> version_num,
        ResourceVersionStatus version_status
    );

signals:
    void download_progress(int percentage);


public:
    // get the DownloadedResourceMetadata from the remote JSON, that corresponds to this button/row
    void initialize_remote_metadata();
    RemoteMetadata& fetch_remote_metadata();
    DownloadedResourceMetadata initialize_local_metadata();

    void run_download(DownloadedResourceMetadata resource_metadata);

public:
    std::once_flag init_flag;

    struct Data;
    Pimpl<Data> m_data;

    ResourceDownloadButton m_download_button;
    ResourceDeleteButton m_delete_button;
    ResourceProgressBar m_progress_bar;
    DownloadedResourceMetadata m_local_metadata;

    std::unique_ptr<RemoteMetadata> m_remote_metadata;



};

}
#endif

/*  DownloadThread
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "Common/Cpp/Filesystem.h"
#include "CommonFramework/Tools/FileDownloader.h"
#include "CommonFramework/Tools/FileUnzip.h"
#include "CommonFramework/Tools/FileHash.h"
#include "CommonFramework/Logging/Logger.h"

#include "DownloadThread.h"


#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

namespace fs = std::filesystem;




DownloadThread::~DownloadThread(){
    // cout << "~DownloadThread" << endl;
    this->cancel();
    m_worker.wait_and_ignore_exceptions();
}
DownloadThread::DownloadThread(Hooks hooks, const DownloadedResourceMetadata& remote_metadata, Mutex& lock, ConditionVariable& cv)
    : m_hooks(std::move(hooks))
    , m_remote_metadata(remote_metadata)
    , m_download_lock(lock)
    , m_download_cv(cv)
{}

void DownloadThread::start_download_thread(){
    m_worker = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
    [this]{

        bool success = false;
        {
            std::unique_lock<Mutex> lock(m_download_lock);
            m_download_cv.wait(lock, [this] { return m_hooks.is_ready_to_start(); });

        }
        
        // runs when lambda is finished
        // updates action state, removes self from download queue
        struct ScopeGuard {
            DownloadThread* thread_ptr;
            bool& success_ref;
            ~ScopeGuard() {
                thread_ptr->m_hooks.on_finished(success_ref);
            }
        } guard{this, success};

        try {
            // std::this_thread::sleep_for(std::chrono::seconds(7));

            run_download(m_remote_metadata);

            cout << "Done Download" << endl;
            success = true;

        }catch(OperationCancelledException&){
            // user cancelled action
            success = false;

        }catch(OperationFailedException&){
            success = false;
            m_hooks.report_failed();
        }catch (const std::exception& e) {
            std::cout << "Standard exception: " << e.what() << std::endl;
            success = false;
            m_hooks.report_exception_caught("ResourceDownloadButton::start_download");
        } catch(...){
            success = false;
            m_hooks.report_exception_caught("ResourceDownloadButton::start_download");
        }

    }
    );

}

void DownloadThread::run_download(DownloadedResourceMetadata resource_metadata){
    Logger& logger = global_logger_tagged();
    // std::this_thread::sleep_for(std::chrono::seconds(5));

    std::string url = resource_metadata.url;
    std::string resource_name = resource_metadata.resource_name;
    uint64_t expected_size = resource_metadata.size_compressed_bytes;

    std::string resource_directory = DOWNLOADED_RESOURCE_PATH() + resource_name;
    try{

        // delete directory and the old resource
        fs::remove_all(Filesystem::Path(resource_directory));

        // download
        std::string zip_path = resource_directory + "/temp.zip";
        FileDownloader::download_file_to_disk(
            *this,
            logger, 
            url, 
            zip_path,
            expected_size,
            [this](uint64_t bytes_done, uint64_t total_bytes){
                m_hooks.report_download_progress(bytes_done, total_bytes);
            }
        );

        // hash
        std::string hash = 
            hash_file(
                *this,
                zip_path,
                [this](uint64_t bytes_done, uint64_t total_bytes){
                    m_hooks.report_hash_progress(bytes_done, total_bytes);
                }
            );
        std::string expected_hash = resource_metadata.sha256;
        if (hash != expected_hash){
            std::cerr << "current hash: " << hash << endl;
            throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, 
                "Downloaded file failed verification. SHA 256 hash did not match the expected value.");
        }

        // Filesystem::Path p{zip_path};
        // cout << "File size: " << std::filesystem::file_size(p) << endl;

        // unzip
        unzip_file(
            *this,
            zip_path.c_str(), 
            resource_directory.c_str(),
            [this](uint64_t bytes_done, uint64_t total_bytes){
                m_hooks.report_unzip_progress(bytes_done, total_bytes);
            }
        );

        // delete old zip file
        fs::remove(Filesystem::Path(zip_path));

        throw_if_cancelled();

    }catch(OperationCancelledException&){
        // delete directory and the resource
        fs::remove_all(Filesystem::Path(resource_directory));

        throw;
    }catch(OperationFailedException&){
        // delete directory and the resource
        fs::remove_all(Filesystem::Path(resource_directory));

        throw;
    }catch(...){
        // delete directory and the resource
        fs::remove_all(Filesystem::Path(resource_directory));

        throw;
    }

}
}

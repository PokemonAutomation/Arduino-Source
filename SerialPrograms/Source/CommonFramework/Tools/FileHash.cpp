/*  File Hash
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include <QFile>
#include <QCryptographicHash>
#include <QDebug>
#include "FileHash.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



std::string hash_file(
    CancellableScope& scope,
    const std::string& file_path,
    std::function<void(uint64_t bytes_done, uint64_t total_bytes)> hash_progress
){
    QFile file(QString::fromStdString(file_path));
    if (!file.open(QIODevice::ReadOnly)){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "hash_file: Could not open file.");
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    qint64 file_size = file.size();
    qint64 total_bytes_read = 0;

    QByteArray buffer(1024 * 1024, 0); // Pre-allocate 1MB once
    while (!file.atEnd()){
        scope.throw_if_cancelled();
        
        qint64 num_bytes_in_chunk = file.read(buffer.data(), buffer.size());
        if (num_bytes_in_chunk == -1){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "hash_file: Read error:" + file.errorString().toStdString());
        }

        hash.addData(QByteArrayView(buffer.data(), num_bytes_in_chunk));
        total_bytes_read += num_bytes_in_chunk;

        hash_progress(total_bytes_read, file_size);
    }

    return hash.result().toHex().toStdString(); 
}

}

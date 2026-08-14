/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QCoreApplication>
#include "FilePath.h"
#include "Filesystem.h"

namespace PokemonAutomation{
namespace Filesystem{


Path application_binary_path(){
    QString application_dir_path = qApp->applicationFilePath();
    return Path(application_dir_path.toStdString());
}
Path application_install_path(){
    return application_binary_directory();
}
Path application_scratch_path(){
    return current_path().string_slash_normalized() + "/";
}



}
}

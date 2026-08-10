/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QCoreApplication>
#include "FilePath.h"

namespace PokemonAutomation{
namespace Filesystem{


Path application_directory(){
    QString application_dir_path = qApp->applicationDirPath();
    return Path(application_dir_path.toStdString());
}



}
}

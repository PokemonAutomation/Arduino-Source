/*  Message Attachment
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QDir>
#include <QFile>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "MessageAttachment.h"

namespace PokemonAutomation{



ImageAttachment::ImageAttachment(
    const ImageViewRGB32& p_image,
    ImageAttachmentMode p_mode,
    bool p_keep_file
)
    : image(p_image)
    , mode(p_mode)
    , keep_file(p_keep_file)
{}



PendingFileSend::~PendingFileSend(){
    if (m_filepath.empty()){
        return;
    }
    if (m_keep_file){
        return;
    }

    if (m_extend_lifetime.load(std::memory_order_acquire)){
        return;
    }

    QFile file(QString::fromStdString(m_filepath));
    file.remove();
}
PendingFileSend::PendingFileSend(const std::string& file, bool keep_file)
    : m_keep_file(keep_file)
    , m_extend_lifetime(false)
    , m_filepath(file)
{
    QFileInfo info(QString::fromStdString(file));
    m_filename = info.fileName().toStdString();
}
PendingFileSend::PendingFileSend(Logger& logger, const ImageAttachment& image)
    : m_keep_file(image.keep_file)
    , m_extend_lifetime(false)
{
    if (image.mode == ImageAttachmentMode::NO_SCREENSHOT){
        return;
    }
    if (!image.image){
        logger.log("Screenshot is null.", COLOR_ORANGE);
        return;
    }

    std::string format;
    switch (image.mode){
    case ImageAttachmentMode::NO_SCREENSHOT:
        break;
    case ImageAttachmentMode::JPG:
        format = ".jpg";
        break;
    case ImageAttachmentMode::PNG:
        format = ".png";
        break;
    }

    m_filename = now_to_filestring() + format;

    if (image.keep_file){
        m_filepath = SCREENSHOTS_PATH() + m_filename;
    }else{
        m_filepath = GlobalSettings::instance().TEMP_FOLDER;
        QDir().mkdir(QString::fromStdString(m_filepath));
        m_filepath += m_filename;
    }

    if (image.image.save(m_filepath)){
        logger.log("Saved image to: " + m_filepath, COLOR_BLUE);
    }else{
        logger.log("Unable to save screenshot to: " + m_filepath, COLOR_RED);
        m_filename.clear();
        m_filepath.clear();
    }
}
void PendingFileSend::extend_lifetime(){
    m_extend_lifetime.store(true, std::memory_order_release);
}





}

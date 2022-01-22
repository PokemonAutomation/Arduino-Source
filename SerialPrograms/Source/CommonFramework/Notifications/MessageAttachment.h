/*  Message Attachment
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_MessageAttachment_H
#define PokemonAutomation_MessageAttachment_H

#include <atomic>
//#include <QFile>
#include <QImage>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/ScreenshotFormatOption.h"

namespace PokemonAutomation{


struct ImageAttachment{
    QImage image;
    ImageAttachmentMode mode = ImageAttachmentMode::NO_SCREENSHOT;
    bool keep_file = false;

    ImageAttachment() = default;
    ImageAttachment(
        QImage p_image,
        ImageAttachmentMode p_mode,
        bool p_keep_file = false
    );
};



//  Represents a file that's in the process of being sent.
//  If (keep_file = false), the file is automatically deleted after being sent.
class PendingFileSend{
public:
    ~PendingFileSend();

    PendingFileSend(const QString& file, bool keep_file);
    PendingFileSend(Logger& logger, const ImageAttachment& image);

    const QString& filename() const{ return m_filename; }
    const QString& filepath() const{ return m_filepath; }
    bool keep_file() const{ return m_keep_file; }

    //  Work around bug in Sleepy that destroys file before it's not needed anymore.
    void extend_lifetime();

private:
    bool m_keep_file;
    std::atomic<bool> m_extend_lifetime;
//    QFile m_file;
    QString m_filename;
    QString m_filepath;
};



}
#endif

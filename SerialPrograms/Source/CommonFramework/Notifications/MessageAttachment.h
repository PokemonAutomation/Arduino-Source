/*  Message Attachment
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MessageAttachment_H
#define PokemonAutomation_MessageAttachment_H

#include <atomic>
#include <memory>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"

namespace PokemonAutomation{


struct ImageAttachment{
    ImageViewRGB32 image;
    ImageAttachmentMode mode = ImageAttachmentMode::NO_SCREENSHOT;
    bool keep_file = false;

    ImageAttachment() = default;
    ImageAttachment(
        const ImageViewRGB32& p_image,
        ImageAttachmentMode p_mode,
        bool p_keep_file = false
    );
};



//  Represents a file that's in the process of being sent.
//  If (keep_file = false), the file is automatically deleted after being sent.
class PendingFileSend{
public:
    ~PendingFileSend();

    PendingFileSend(const std::string& file, bool keep_file);
//    PendingFileSend(Logger& logger, const std::string& text_attachment);
    PendingFileSend(Logger& logger, const ImageAttachment& image);

    const std::string& filename() const{ return m_filename; }
    const std::string& filepath() const{ return m_filepath; }
    bool keep_file() const{ return m_keep_file; }

    //  Work around bug in Sleepy that destroys file before it's not needed anymore.
    void extend_lifetime();

private:
    bool m_keep_file;
    std::atomic<bool> m_extend_lifetime;
//    QFile m_file;
    std::string m_filename;
    std::string m_filepath;
};



}
#endif

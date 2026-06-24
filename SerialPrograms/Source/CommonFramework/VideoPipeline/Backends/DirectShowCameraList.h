/*  DirectShow Camera List (Windows-only)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      Enumerates DirectShow video capture devices on Windows.
 *  Some capture cards (e.g. AVerMedia GC550) only expose DirectShow
 *  drivers and are not visible to Qt6's WMF-based QMediaDevices.
 *  This module discovers those devices so they can be offered in the UI.
 *
 */

#ifndef PokemonAutomation_VideoPipeline_DirectShowCameraList_H
#define PokemonAutomation_VideoPipeline_DirectShowCameraList_H

#ifdef _WIN32

#include <string>
#include <vector>
#include "CommonFramework/VideoPipeline/CameraInfo.h"

namespace PokemonAutomation{


struct DirectShowDeviceInfo{
    int index;              //  Device index for cv::VideoCapture(index, CAP_DSHOW)
    std::string name;       //  Friendly name
    std::string device_path;//  Device path (unique identifier)
};

//  Enumerate all DirectShow video capture devices.
std::vector<DirectShowDeviceInfo> enumerate_directshow_devices();

//  Build CameraInfo list for DirectShow devices not already present
//  in the Qt camera list (matched by friendly name).
std::vector<CameraInfo> get_directshow_only_cameras(
    const std::vector<std::string>& qt_camera_names
);

//  CameraInfo device_name format: "dshow:<index>:<friendly_name>"
constexpr const char DSHOW_PREFIX[] = "dshow:";

//  Check whether a CameraInfo represents a DirectShow-only device.
bool is_directshow_device(const CameraInfo& info);

//  Extract the device index from a dshow CameraInfo.
int get_directshow_device_index(const CameraInfo& info);

//  Extract the display name from a dshow CameraInfo.
std::string get_directshow_display_name(const CameraInfo& info);


//  Enumerate all DirectShow audio capture devices.
std::vector<DirectShowDeviceInfo> enumerate_directshow_audio_devices();

//  Return friendly names of DirectShow audio capture devices that are
//  not found (by fuzzy name match) in the given Qt audio input list.
std::vector<std::string> get_directshow_only_audio_devices(
    const std::vector<std::string>& qt_audio_names
);


//  Return friendly names of all DirectShow video capture devices
//  (which may have embedded audio pins for HDMI capture cards).
//  These appear in the audio dropdown so users can capture audio
//  from the same device as the video.
std::vector<std::string> get_video_devices_with_audio_pins();


}

#endif // _WIN32
#endif

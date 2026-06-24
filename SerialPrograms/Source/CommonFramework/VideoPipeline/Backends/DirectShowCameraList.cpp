/*  DirectShow Camera List (Windows-only)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef _WIN32

#include <algorithm>
#include <Windows.h>
#include <dshow.h>
#include "CommonFramework/Logging/Logger.h"
#include "DirectShowCameraList.h"

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ole32.lib")

namespace PokemonAutomation{

namespace{
std::string wchar_to_utf8(const wchar_t* wstr){
    if (!wstr || wstr[0] == L'\0'){
        return {};
    }
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0){
        return {};
    }
    std::string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result.data(), len, nullptr, nullptr);
    return result;
}
}


static std::vector<DirectShowDeviceInfo> enumerate_directshow_category(const CLSID& category){
    std::vector<DirectShowDeviceInfo> devices;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool com_initialized = SUCCEEDED(hr);

    ICreateDevEnum* dev_enum = nullptr;
    hr = CoCreateInstance(
        CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum, reinterpret_cast<void**>(&dev_enum)
    );
    if (FAILED(hr) || !dev_enum){
        if (com_initialized) CoUninitialize();
        return devices;
    }

    IEnumMoniker* enum_moniker = nullptr;
    hr = dev_enum->CreateClassEnumerator(category, &enum_moniker, 0);
    if (hr == S_OK && enum_moniker){
        IMoniker* moniker = nullptr;
        int index = 0;
        while (enum_moniker->Next(1, &moniker, nullptr) == S_OK){
            IPropertyBag* prop_bag = nullptr;
            hr = moniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag,
                                        reinterpret_cast<void**>(&prop_bag));
            if (SUCCEEDED(hr) && prop_bag){
                DirectShowDeviceInfo info;
                info.index = index;

                VARIANT var_name;
                VariantInit(&var_name);
                if (SUCCEEDED(prop_bag->Read(L"FriendlyName", &var_name, nullptr))){
                    info.name = wchar_to_utf8(var_name.bstrVal);
                }
                VariantClear(&var_name);

                VARIANT var_path;
                VariantInit(&var_path);
                if (SUCCEEDED(prop_bag->Read(L"DevicePath", &var_path, nullptr))){
                    info.device_path = wchar_to_utf8(var_path.bstrVal);
                }
                VariantClear(&var_path);

                if (!info.name.empty()){
                    devices.push_back(std::move(info));
                }

                prop_bag->Release();
            }
            moniker->Release();
            index++;
        }
        enum_moniker->Release();
    }

    dev_enum->Release();
    if (com_initialized) CoUninitialize();
    return devices;
}


std::vector<DirectShowDeviceInfo> enumerate_directshow_devices(){
    return enumerate_directshow_category(CLSID_VideoInputDeviceCategory);
}


std::vector<CameraInfo> get_directshow_only_cameras(
    const std::vector<std::string>& qt_camera_names
){
    std::vector<CameraInfo> result;
    auto dshow_devices = enumerate_directshow_devices();

    for (const auto& device : dshow_devices){
        //  Skip if this device is already visible to Qt (matched by name).
        bool found_in_qt = false;
        for (const auto& qt_name : qt_camera_names){
            if (qt_name.find(device.name) != std::string::npos ||
                device.name.find(qt_name) != std::string::npos){
                found_in_qt = true;
                break;
            }
        }
        if (found_in_qt){
            continue;
        }

        //  Format: "dshow:<index>:<friendly_name>"
        std::string device_name = std::string(DSHOW_PREFIX)
            + std::to_string(device.index) + ":" + device.name;
        result.emplace_back(CameraInfo(std::move(device_name)));
    }

    return result;
}


bool is_directshow_device(const CameraInfo& info){
    const std::string& name = info.device_name();
    return name.size() > sizeof(DSHOW_PREFIX) - 1 &&
           name.compare(0, sizeof(DSHOW_PREFIX) - 1, DSHOW_PREFIX) == 0;
}

int get_directshow_device_index(const CameraInfo& info){
    //  Format: "dshow:<index>:<name>"
    const std::string& name = info.device_name();
    size_t prefix_len = sizeof(DSHOW_PREFIX) - 1;
    size_t colon_pos = name.find(':', prefix_len);
    if (colon_pos == std::string::npos){
        return -1;
    }
    return std::stoi(name.substr(prefix_len, colon_pos - prefix_len));
}

std::string get_directshow_display_name(const CameraInfo& info){
    //  Format: "dshow:<index>:<name>"
    const std::string& name = info.device_name();
    size_t prefix_len = sizeof(DSHOW_PREFIX) - 1;
    size_t colon_pos = name.find(':', prefix_len);
    if (colon_pos == std::string::npos || colon_pos + 1 >= name.size()){
        return "DirectShow Device";
    }
    return name.substr(colon_pos + 1) + " (DirectShow)";
}


std::vector<DirectShowDeviceInfo> enumerate_directshow_audio_devices(){
    return enumerate_directshow_category(CLSID_AudioInputDeviceCategory);
}


std::vector<std::string> get_directshow_only_audio_devices(
    const std::vector<std::string>& qt_audio_names
){
    std::vector<std::string> result;
    auto dshow_devices = enumerate_directshow_audio_devices();

    for (const auto& device : dshow_devices){
        bool found_in_qt = false;
        for (const auto& qt_name : qt_audio_names){
            if (qt_name.find(device.name) != std::string::npos ||
                device.name.find(qt_name) != std::string::npos){
                found_in_qt = true;
                break;
            }
        }
        if (!found_in_qt){
            result.push_back(device.name);
        }
    }

    return result;
}


std::vector<std::string> get_video_devices_with_audio_pins(){
    //  Enumerate all DirectShow video capture devices, then check each one
    //  for an audio capture pin.  Capture cards with HDMI input typically
    //  embed audio on the same filter as video.
    std::vector<std::string> result;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool com_ok = SUCCEEDED(hr);

    ICreateDevEnum* dev_enum = nullptr;
    hr = CoCreateInstance(
        CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum, reinterpret_cast<void**>(&dev_enum)
    );
    if (FAILED(hr) || !dev_enum){
        if (com_ok) CoUninitialize();
        return result;
    }

    IEnumMoniker* enum_moniker = nullptr;
    hr = dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enum_moniker, 0);
    if (hr == S_OK && enum_moniker){
        IMoniker* moniker = nullptr;
        while (enum_moniker->Next(1, &moniker, nullptr) == S_OK){
            //  Get friendly name.
            std::string friendly_name;
            {
                IPropertyBag* prop_bag = nullptr;
                hr = moniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag,
                                            reinterpret_cast<void**>(&prop_bag));
                if (SUCCEEDED(hr) && prop_bag){
                    VARIANT var_name;
                    VariantInit(&var_name);
                    if (SUCCEEDED(prop_bag->Read(L"FriendlyName", &var_name, nullptr))){
                        friendly_name = wchar_to_utf8(var_name.bstrVal);
                    }
                    VariantClear(&var_name);
                    prop_bag->Release();
                }
            }
            if (friendly_name.empty()){
                moniker->Release();
                continue;
            }

            //  Bind to the actual filter and look for an audio pin.
            IBaseFilter* filter = nullptr;
            hr = moniker->BindToObject(nullptr, nullptr, IID_IBaseFilter,
                                       reinterpret_cast<void**>(&filter));
            if (SUCCEEDED(hr) && filter){
                IEnumPins* enum_pins = nullptr;
                if (SUCCEEDED(filter->EnumPins(&enum_pins)) && enum_pins){
                    IPin* pin = nullptr;
                    while (enum_pins->Next(1, &pin, nullptr) == S_OK){
                        //  Check if this pin's media type includes audio.
                        IEnumMediaTypes* enum_mt = nullptr;
                        if (SUCCEEDED(pin->EnumMediaTypes(&enum_mt)) && enum_mt){
                            AM_MEDIA_TYPE* mt = nullptr;
                            while (enum_mt->Next(1, &mt, nullptr) == S_OK){
                                if (mt->majortype == MEDIATYPE_Audio){
                                    result.push_back(friendly_name);
                                    //  Free and break.
                                    if (mt->pbFormat) CoTaskMemFree(mt->pbFormat);
                                    CoTaskMemFree(mt);
                                    enum_mt->Release();
                                    pin->Release();
                                    goto next_device;
                                }
                                if (mt->pbFormat) CoTaskMemFree(mt->pbFormat);
                                CoTaskMemFree(mt);
                            }
                            enum_mt->Release();
                        }
                        pin->Release();
                    }
                    enum_pins->Release();
                }
                next_device:
                filter->Release();
            }
            moniker->Release();
        }
        enum_moniker->Release();
    }

    dev_enum->Release();
    if (com_ok) CoUninitialize();
    return result;
}


}

#endif // _WIN32

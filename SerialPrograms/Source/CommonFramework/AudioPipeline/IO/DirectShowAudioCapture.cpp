/*  DirectShow Audio Capture (Windows-only)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef _WIN32

#include <Windows.h>
#include <dshow.h>
#include <mmreg.h>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/AudioPipeline/AudioStream.h"
#include "DirectShowAudioCapture.h"

//  qedit.h is not shipped with modern SDKs.  Define ISampleGrabber inline.
MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
ISampleGrabber : public IUnknown{
public:
    virtual HRESULT STDMETHODCALLTYPE SetOneShot(BOOL OneShot) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMediaType(const AM_MEDIA_TYPE* pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(AM_MEDIA_TYPE* pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(BOOL BufferThem) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(long* pBufferSize, long* pBuffer) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(IMediaSample** ppSample) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetCallback(/*ISampleGrabberCB**/IUnknown* pCallback, long WhichMethodToCallback) = 0;
};

MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
ISampleGrabberCB : public IUnknown{
public:
    virtual HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample* pSample) = 0;
    virtual HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen) = 0;
};

static const CLSID CLSID_SampleGrabber = {
    0xC1F400A0, 0x3F08, 0x11d3,
    {0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37}
};
static const CLSID CLSID_NullRenderer = {
    0xC1F400A4, 0x3F08, 0x11d3,
    {0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37}
};

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ole32.lib")

namespace PokemonAutomation{

namespace{

//  Helper: convert wide string to UTF-8.
std::string wchar_to_utf8_local(const wchar_t* wstr){
    if (!wstr || wstr[0] == L'\0') return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result.data(), len, nullptr, nullptr);
    return result;
}

//  Find a DirectShow video capture moniker by friendly name.
IMoniker* find_video_device_by_name(const std::string& device_name){
    ICreateDevEnum* dev_enum = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum, reinterpret_cast<void**>(&dev_enum)
    );
    if (FAILED(hr) || !dev_enum) return nullptr;

    IEnumMoniker* enum_moniker = nullptr;
    hr = dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enum_moniker, 0);
    IMoniker* result = nullptr;
    if (hr == S_OK && enum_moniker){
        IMoniker* moniker = nullptr;
        while (enum_moniker->Next(1, &moniker, nullptr) == S_OK){
            IPropertyBag* prop_bag = nullptr;
            if (SUCCEEDED(moniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag,
                                                 reinterpret_cast<void**>(&prop_bag))) && prop_bag){
                VARIANT var_name;
                VariantInit(&var_name);
                if (SUCCEEDED(prop_bag->Read(L"FriendlyName", &var_name, nullptr))){
                    std::string name = wchar_to_utf8_local(var_name.bstrVal);
                    if (name == device_name){
                        result = moniker;  // caller must Release()
                        VariantClear(&var_name);
                        prop_bag->Release();
                        break;
                    }
                }
                VariantClear(&var_name);
                prop_bag->Release();
            }
            moniker->Release();
        }
        enum_moniker->Release();
    }
    dev_enum->Release();
    return result;
}

}  // anonymous namespace


DirectShowAudioCapture::~DirectShowAudioCapture(){
    m_stopping.store(true, std::memory_order_release);
    if (m_thread.joinable()){
        m_thread.join();
    }
}

DirectShowAudioCapture::DirectShowAudioCapture(
    Logger& logger,
    AudioStreamToFloat& reader,
    const std::string& device_name,
    int sample_rate,
    int channels
)
    : m_logger(logger)
    , m_reader(reader)
{
    m_logger.log(
        "DirectShowAudioCapture: Starting audio from \"" + device_name + "\" "
        "(" + std::to_string(sample_rate) + "Hz, " + std::to_string(channels) + "ch)"
    );
    m_thread = std::thread(
        &DirectShowAudioCapture::capture_thread, this,
        device_name, sample_rate, channels
    );
}


void DirectShowAudioCapture::capture_thread(
    const std::string& device_name,
    int sample_rate,
    int channels
){
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool com_ok = SUCCEEDED(hr);

    //  Find the device.
    IMoniker* moniker = find_video_device_by_name(device_name);
    if (!moniker){
        m_logger.log("DirectShowAudioCapture: Device not found: \"" + device_name + "\"", COLOR_RED);
        if (com_ok) CoUninitialize();
        return;
    }

    //  Create filter graph.
    IGraphBuilder* graph = nullptr;
    hr = CoCreateInstance(
        CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER,
        IID_IGraphBuilder, reinterpret_cast<void**>(&graph)
    );
    if (FAILED(hr) || !graph){
        m_logger.log("DirectShowAudioCapture: Failed to create filter graph.", COLOR_RED);
        moniker->Release();
        if (com_ok) CoUninitialize();
        return;
    }

    //  Create capture graph builder.
    ICaptureGraphBuilder2* capture_builder = nullptr;
    hr = CoCreateInstance(
        CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC_SERVER,
        IID_ICaptureGraphBuilder2, reinterpret_cast<void**>(&capture_builder)
    );
    if (FAILED(hr) || !capture_builder){
        m_logger.log("DirectShowAudioCapture: Failed to create capture graph builder.", COLOR_RED);
        graph->Release();
        moniker->Release();
        if (com_ok) CoUninitialize();
        return;
    }
    capture_builder->SetFiltergraph(graph);

    //  Bind video capture device to filter.
    IBaseFilter* source_filter = nullptr;
    hr = moniker->BindToObject(nullptr, nullptr, IID_IBaseFilter,
                               reinterpret_cast<void**>(&source_filter));
    moniker->Release();
    if (FAILED(hr) || !source_filter){
        m_logger.log("DirectShowAudioCapture: Failed to bind device filter.", COLOR_RED);
        capture_builder->Release();
        graph->Release();
        if (com_ok) CoUninitialize();
        return;
    }
    graph->AddFilter(source_filter, L"Video Capture");

    //  Create SampleGrabber filter for audio.
    IBaseFilter* grabber_filter = nullptr;
    hr = CoCreateInstance(
        CLSID_SampleGrabber, nullptr, CLSCTX_INPROC_SERVER,
        IID_IBaseFilter, reinterpret_cast<void**>(&grabber_filter)
    );
    if (FAILED(hr) || !grabber_filter){
        m_logger.log("DirectShowAudioCapture: Failed to create SampleGrabber.", COLOR_RED);
        source_filter->Release();
        capture_builder->Release();
        graph->Release();
        if (com_ok) CoUninitialize();
        return;
    }
    graph->AddFilter(grabber_filter, L"Audio Grabber");

    //  Configure SampleGrabber to accept audio.
    ISampleGrabber* grabber = nullptr;
    grabber_filter->QueryInterface(IID_PPV_ARGS(&grabber));
    if (!grabber){
        //  Try querying by CLSID-based IID.
        static const IID IID_ISampleGrabber = {
            0x6B652FFF, 0x11FE, 0x4fce,
            {0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F}
        };
        grabber_filter->QueryInterface(IID_ISampleGrabber,
                                       reinterpret_cast<void**>(&grabber));
    }
    if (!grabber){
        m_logger.log("DirectShowAudioCapture: Failed to get ISampleGrabber interface.", COLOR_RED);
        grabber_filter->Release();
        source_filter->Release();
        capture_builder->Release();
        graph->Release();
        if (com_ok) CoUninitialize();
        return;
    }

    {
        AM_MEDIA_TYPE mt;
        ZeroMemory(&mt, sizeof(mt));
        mt.majortype = MEDIATYPE_Audio;
        mt.subtype = MEDIASUBTYPE_PCM;
        grabber->SetMediaType(&mt);
    }
    grabber->SetOneShot(FALSE);
    grabber->SetBufferSamples(TRUE);

    //  Create a null renderer (sink) for the audio stream.
    IBaseFilter* null_renderer = nullptr;
    hr = CoCreateInstance(
        CLSID_NullRenderer, nullptr, CLSCTX_INPROC_SERVER,
        IID_IBaseFilter, reinterpret_cast<void**>(&null_renderer)
    );
    if (FAILED(hr) || !null_renderer){
        m_logger.log("DirectShowAudioCapture: Failed to create NullRenderer.", COLOR_RED);
        grabber->Release();
        grabber_filter->Release();
        source_filter->Release();
        capture_builder->Release();
        graph->Release();
        if (com_ok) CoUninitialize();
        return;
    }
    graph->AddFilter(null_renderer, L"Null Renderer");

    //  Connect: source_filter [audio pin] -> grabber_filter -> null_renderer
    hr = capture_builder->RenderStream(
        &PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
        source_filter, grabber_filter, null_renderer
    );
    if (FAILED(hr)){
        //  Some devices use PIN_CATEGORY_PREVIEW for audio.
        hr = capture_builder->RenderStream(
            &PIN_CATEGORY_PREVIEW, &MEDIATYPE_Audio,
            source_filter, grabber_filter, null_renderer
        );
    }
    if (FAILED(hr)){
        //  Last resort: try with nullptr category.
        hr = capture_builder->RenderStream(
            nullptr, &MEDIATYPE_Audio,
            source_filter, grabber_filter, null_renderer
        );
    }
    if (FAILED(hr)){
        m_logger.log("DirectShowAudioCapture: Failed to connect audio pin. HRESULT=0x" +
                      std::to_string(hr), COLOR_RED);
        null_renderer->Release();
        grabber->Release();
        grabber_filter->Release();
        source_filter->Release();
        capture_builder->Release();
        graph->Release();
        if (com_ok) CoUninitialize();
        return;
    }

    //  Get the actual audio format that was negotiated.
    AM_MEDIA_TYPE connected_mt;
    ZeroMemory(&connected_mt, sizeof(connected_mt));
    hr = grabber->GetConnectedMediaType(&connected_mt);
    if (FAILED(hr)){
        m_logger.log("DirectShowAudioCapture: Failed to get connected media type.", COLOR_RED);
        null_renderer->Release();
        grabber->Release();
        grabber_filter->Release();
        source_filter->Release();
        capture_builder->Release();
        graph->Release();
        if (com_ok) CoUninitialize();
        return;
    }

    int actual_sample_rate = sample_rate;
    int actual_channels = channels;
    int bits_per_sample = 16;
    if (connected_mt.formattype == FORMAT_WaveFormatEx && connected_mt.pbFormat){
        WAVEFORMATEX* wfx = reinterpret_cast<WAVEFORMATEX*>(connected_mt.pbFormat);
        actual_sample_rate = wfx->nSamplesPerSec;
        actual_channels = wfx->nChannels;
        bits_per_sample = wfx->wBitsPerSample;
        m_logger.log(
            "DirectShowAudioCapture: Negotiated format: " +
            std::to_string(actual_sample_rate) + "Hz, " +
            std::to_string(actual_channels) + "ch, " +
            std::to_string(bits_per_sample) + "-bit"
        );
    }
    if (connected_mt.pbFormat) CoTaskMemFree(connected_mt.pbFormat);
    if (connected_mt.pUnk) connected_mt.pUnk->Release();

    //  Start the graph.
    IMediaControl* control = nullptr;
    graph->QueryInterface(IID_IMediaControl, reinterpret_cast<void**>(&control));
    if (!control || FAILED(control->Run())){
        m_logger.log("DirectShowAudioCapture: Failed to start filter graph.", COLOR_RED);
        if (control) control->Release();
        null_renderer->Release();
        grabber->Release();
        grabber_filter->Release();
        source_filter->Release();
        capture_builder->Release();
        graph->Release();
        if (com_ok) CoUninitialize();
        return;
    }

    m_logger.log("DirectShowAudioCapture: Audio capture started successfully.");

    //  Polling loop: pull audio samples from the SampleGrabber buffer.
    const int bytes_per_sample_frame = actual_channels * (bits_per_sample / 8);
    //  Buffer for ~50ms of audio at a time.
    long buffer_size = actual_sample_rate * bytes_per_sample_frame / 20;
    std::vector<char> buffer(buffer_size);

    while (!m_stopping.load(std::memory_order_acquire)){
        long current_size = buffer_size;
        hr = grabber->GetCurrentBuffer(&current_size, reinterpret_cast<long*>(buffer.data()));
        if (SUCCEEDED(hr) && current_size > 0){
            m_reader.push_bytes(buffer.data(), current_size);
        }

        //  Sleep briefly to avoid busy-spinning (~20ms = ~50fps audio polling).
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    //  Teardown.
    control->Stop();
    control->Release();
    null_renderer->Release();
    grabber->Release();
    grabber_filter->Release();
    source_filter->Release();
    capture_builder->Release();
    graph->Release();
    if (com_ok) CoUninitialize();

    m_logger.log("DirectShowAudioCapture: Audio capture stopped.");
}


}

#endif // _WIN32

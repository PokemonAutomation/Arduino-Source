/*  DirectShow Audio Capture (Windows-only)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef _WIN32

#include <iomanip>
#include <sstream>
#include <Windows.h>
#include <dshow.h>
#include <mmreg.h>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/AudioPipeline/AudioStream.h"
#include "DirectShowAudioCapture.h"

//  qedit.h is not shipped with modern Windows SDKs.
//  Define ISampleGrabberCB and ISampleGrabber interfaces inline.
//  The vtable layout must match the original COM interfaces exactly.

static const IID IID_ISampleGrabberCB = {
    0x0579154A, 0x2B53, 0x4994,
    {0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85}
};
static const IID IID_ISampleGrabber = {
    0x6B652FFF, 0x11FE, 0x4fce,
    {0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F}
};

MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
ISampleGrabberCB : public IUnknown{
public:
    virtual HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample* pSample) = 0;
    virtual HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen) = 0;
};

MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
ISampleGrabber : public IUnknown{
public:
    virtual HRESULT STDMETHODCALLTYPE SetOneShot(BOOL OneShot) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMediaType(const AM_MEDIA_TYPE* pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(AM_MEDIA_TYPE* pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(BOOL BufferThem) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(long* pBufferSize, long* pBuffer) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(IMediaSample** ppSample) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetCallback(ISampleGrabberCB* pCallback, long WhichMethodToCallback) = 0;
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

std::string hresult_to_hex(HRESULT hr){
    std::ostringstream ss;
    ss << "0x" << std::hex << std::setfill('0') << std::setw(8) << static_cast<unsigned long>(hr);
    return ss.str();
}

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


//  ISampleGrabberCB implementation that pushes audio data to AudioStreamToFloat.
class AudioGrabberCallback : public ISampleGrabberCB{
public:
    AudioGrabberCallback(AudioStreamToFloat& reader, Logger& logger)
        : m_reader(reader)
        , m_logger(logger)
        , m_ref_count(1)
        , m_total_bytes(0)
        , m_callback_count(0)
    {}

    //  IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override{
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_ISampleGrabberCB){
            *ppv = static_cast<ISampleGrabberCB*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override{
        return InterlockedIncrement(&m_ref_count);
    }
    STDMETHODIMP_(ULONG) Release() override{
        LONG count = InterlockedDecrement(&m_ref_count);
        if (count == 0) delete this;
        return count;
    }

    //  ISampleGrabberCB
    STDMETHODIMP SampleCB(double SampleTime, IMediaSample* pSample) override{
        return E_NOTIMPL;
    }
    STDMETHODIMP BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen) override{
        if (pBuffer && BufferLen > 0){
            m_reader.push_bytes(reinterpret_cast<const char*>(pBuffer), BufferLen);

            size_t prev_total = m_total_bytes.fetch_add(BufferLen, std::memory_order_relaxed);
            size_t count = m_callback_count.fetch_add(1, std::memory_order_relaxed);

            //  Log periodically (first few callbacks, then every ~500 callbacks ≈ every ~10s).
            if (count < 5 || (count % 500) == 0){
                m_logger.log(
                    "DirectShowAudioCapture: BufferCB #" + std::to_string(count) +
                    " len=" + std::to_string(BufferLen) +
                    " total=" + std::to_string(prev_total + BufferLen) + " bytes"
                );
            }
        }
        return S_OK;
    }

    size_t total_bytes() const{ return m_total_bytes.load(std::memory_order_relaxed); }
    size_t callback_count() const{ return m_callback_count.load(std::memory_order_relaxed); }

private:
    AudioStreamToFloat& m_reader;
    Logger& m_logger;
    LONG m_ref_count;
    std::atomic<size_t> m_total_bytes;
    std::atomic<size_t> m_callback_count;
};


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
    m_logger.log("DirectShowAudioCapture: Device moniker found.");

    //  Create filter graph.
    IGraphBuilder* graph = nullptr;
    hr = CoCreateInstance(
        CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER,
        IID_IGraphBuilder, reinterpret_cast<void**>(&graph)
    );
    if (FAILED(hr) || !graph){
        m_logger.log("DirectShowAudioCapture: Failed to create filter graph. hr=" + hresult_to_hex(hr), COLOR_RED);
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
        m_logger.log("DirectShowAudioCapture: Failed to create capture graph builder. hr=" + hresult_to_hex(hr), COLOR_RED);
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
        m_logger.log("DirectShowAudioCapture: Failed to bind device filter. hr=" + hresult_to_hex(hr), COLOR_RED);
        capture_builder->Release();
        graph->Release();
        if (com_ok) CoUninitialize();
        return;
    }
    hr = graph->AddFilter(source_filter, L"Video Capture");
    m_logger.log("DirectShowAudioCapture: Source filter added to graph. hr=" + hresult_to_hex(hr));

    //  Log all pins on the source filter for diagnostics.
    {
        IEnumPins* enum_pins = nullptr;
        if (SUCCEEDED(source_filter->EnumPins(&enum_pins)) && enum_pins){
            IPin* pin = nullptr;
            int pin_idx = 0;
            while (enum_pins->Next(1, &pin, nullptr) == S_OK){
                PIN_INFO pin_info;
                if (SUCCEEDED(pin->QueryPinInfo(&pin_info))){
                    std::string pin_name = wchar_to_utf8_local(pin_info.achName);
                    std::string dir_str = (pin_info.dir == PINDIR_INPUT) ? "INPUT" : "OUTPUT";
                    m_logger.log(
                        "DirectShowAudioCapture: Pin[" + std::to_string(pin_idx) + "]: "
                        "\"" + pin_name + "\" dir=" + dir_str
                    );

                    //  Enumerate media types on this pin.
                    IEnumMediaTypes* enum_mt = nullptr;
                    if (SUCCEEDED(pin->EnumMediaTypes(&enum_mt)) && enum_mt){
                        AM_MEDIA_TYPE* mt = nullptr;
                        int mt_idx = 0;
                        while (enum_mt->Next(1, &mt, nullptr) == S_OK){
                            std::string major = "unknown";
                            if (mt->majortype == MEDIATYPE_Video) major = "Video";
                            else if (mt->majortype == MEDIATYPE_Audio) major = "Audio";
                            else if (mt->majortype == MEDIATYPE_Stream) major = "Stream";

                            std::string sub = "unknown";
                            if (mt->subtype == MEDIASUBTYPE_PCM) sub = "PCM";
                            else if (mt->subtype == MEDIASUBTYPE_IEEE_FLOAT) sub = "IEEE_FLOAT";
                            else if (mt->subtype == MEDIASUBTYPE_YUY2) sub = "YUY2";
                            else if (mt->subtype == MEDIASUBTYPE_MJPG) sub = "MJPG";
                            else if (mt->subtype == MEDIASUBTYPE_RGB24) sub = "RGB24";

                            if (mt->majortype == MEDIATYPE_Audio && mt->formattype == FORMAT_WaveFormatEx && mt->pbFormat){
                                WAVEFORMATEX* wfx = reinterpret_cast<WAVEFORMATEX*>(mt->pbFormat);
                                m_logger.log(
                                    "DirectShowAudioCapture:   MediaType[" + std::to_string(mt_idx) + "]: "
                                    "major=" + major + " sub=" + sub + " "
                                    + std::to_string(wfx->nSamplesPerSec) + "Hz "
                                    + std::to_string(wfx->nChannels) + "ch "
                                    + std::to_string(wfx->wBitsPerSample) + "-bit"
                                );
                            }else{
                                m_logger.log(
                                    "DirectShowAudioCapture:   MediaType[" + std::to_string(mt_idx) + "]: "
                                    "major=" + major + " sub=" + sub
                                );
                            }

                            if (mt->pbFormat) CoTaskMemFree(mt->pbFormat);
                            if (mt->pUnk) mt->pUnk->Release();
                            CoTaskMemFree(mt);
                            mt_idx++;
                        }
                        enum_mt->Release();
                    }

                    if (pin_info.pFilter) pin_info.pFilter->Release();
                }
                pin->Release();
                pin_idx++;
            }
            enum_pins->Release();
        }
    }

    //  Create SampleGrabber filter for audio.
    IBaseFilter* grabber_filter = nullptr;
    hr = CoCreateInstance(
        CLSID_SampleGrabber, nullptr, CLSCTX_INPROC_SERVER,
        IID_IBaseFilter, reinterpret_cast<void**>(&grabber_filter)
    );
    if (FAILED(hr) || !grabber_filter){
        m_logger.log("DirectShowAudioCapture: Failed to create SampleGrabber. hr=" + hresult_to_hex(hr), COLOR_RED);
        source_filter->Release();
        capture_builder->Release();
        graph->Release();
        if (com_ok) CoUninitialize();
        return;
    }
    graph->AddFilter(grabber_filter, L"Audio Grabber");

    //  Get ISampleGrabber interface.
    ISampleGrabber* grabber = nullptr;
    hr = grabber_filter->QueryInterface(IID_ISampleGrabber,
                                        reinterpret_cast<void**>(&grabber));
    if (FAILED(hr) || !grabber){
        m_logger.log("DirectShowAudioCapture: Failed to get ISampleGrabber. hr=" + hresult_to_hex(hr), COLOR_RED);
        grabber_filter->Release();
        source_filter->Release();
        capture_builder->Release();
        graph->Release();
        if (com_ok) CoUninitialize();
        return;
    }

    //  Configure SampleGrabber to accept audio PCM.
    {
        AM_MEDIA_TYPE mt;
        ZeroMemory(&mt, sizeof(mt));
        mt.majortype = MEDIATYPE_Audio;
        mt.subtype = MEDIASUBTYPE_PCM;
        hr = grabber->SetMediaType(&mt);
        m_logger.log("DirectShowAudioCapture: SetMediaType hr=" + hresult_to_hex(hr));
    }
    grabber->SetOneShot(FALSE);
    grabber->SetBufferSamples(FALSE);  //  We use callback, not polling.

    //  Install callback to receive audio data.
    AudioGrabberCallback* callback = new AudioGrabberCallback(m_reader, m_logger);
    hr = grabber->SetCallback(callback, 1);  //  1 = BufferCB
    m_logger.log("DirectShowAudioCapture: SetCallback hr=" + hresult_to_hex(hr));
    if (FAILED(hr)){
        m_logger.log("DirectShowAudioCapture: SetCallback failed!", COLOR_RED);
        callback->Release();
        grabber->Release();
        grabber_filter->Release();
        source_filter->Release();
        capture_builder->Release();
        graph->Release();
        if (com_ok) CoUninitialize();
        return;
    }

    //  Create a null renderer (sink) for the audio stream.
    IBaseFilter* null_renderer = nullptr;
    hr = CoCreateInstance(
        CLSID_NullRenderer, nullptr, CLSCTX_INPROC_SERVER,
        IID_IBaseFilter, reinterpret_cast<void**>(&null_renderer)
    );
    if (FAILED(hr) || !null_renderer){
        m_logger.log("DirectShowAudioCapture: Failed to create NullRenderer. hr=" + hresult_to_hex(hr), COLOR_RED);
        grabber->SetCallback(nullptr, 1);
        callback->Release();
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
    //  Try multiple pin categories since devices differ.
    m_logger.log("DirectShowAudioCapture: Attempting RenderStream with PIN_CATEGORY_CAPTURE...");
    hr = capture_builder->RenderStream(
        &PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
        source_filter, grabber_filter, null_renderer
    );
    m_logger.log("DirectShowAudioCapture: PIN_CATEGORY_CAPTURE hr=" + hresult_to_hex(hr));
    if (FAILED(hr)){
        m_logger.log("DirectShowAudioCapture: Attempting RenderStream with PIN_CATEGORY_PREVIEW...");
        hr = capture_builder->RenderStream(
            &PIN_CATEGORY_PREVIEW, &MEDIATYPE_Audio,
            source_filter, grabber_filter, null_renderer
        );
        m_logger.log("DirectShowAudioCapture: PIN_CATEGORY_PREVIEW hr=" + hresult_to_hex(hr));
    }
    if (FAILED(hr)){
        m_logger.log("DirectShowAudioCapture: Attempting RenderStream with nullptr category...");
        hr = capture_builder->RenderStream(
            nullptr, &MEDIATYPE_Audio,
            source_filter, grabber_filter, null_renderer
        );
        m_logger.log("DirectShowAudioCapture: nullptr category hr=" + hresult_to_hex(hr));
    }
    if (FAILED(hr)){
        m_logger.log(
            "DirectShowAudioCapture: All RenderStream attempts failed. "
            "The device may not have an accessible audio pin, or it may be in use. "
            "hr=" + hresult_to_hex(hr),
            COLOR_RED
        );
        grabber->SetCallback(nullptr, 1);
        callback->Release();
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
        m_logger.log("DirectShowAudioCapture: GetConnectedMediaType failed. hr=" + hresult_to_hex(hr), COLOR_RED);
    }else{
        if (connected_mt.formattype == FORMAT_WaveFormatEx && connected_mt.pbFormat){
            WAVEFORMATEX* wfx = reinterpret_cast<WAVEFORMATEX*>(connected_mt.pbFormat);
            m_logger.log(
                "DirectShowAudioCapture: Negotiated format: " +
                std::to_string(wfx->nSamplesPerSec) + "Hz, " +
                std::to_string(wfx->nChannels) + "ch, " +
                std::to_string(wfx->wBitsPerSample) + "-bit, " +
                "blockAlign=" + std::to_string(wfx->nBlockAlign) + ", " +
                "avgBytesPerSec=" + std::to_string(wfx->nAvgBytesPerSec)
            );
        }else{
            m_logger.log("DirectShowAudioCapture: Connected media type has unexpected format.");
        }
        if (connected_mt.pbFormat) CoTaskMemFree(connected_mt.pbFormat);
        if (connected_mt.pUnk) connected_mt.pUnk->Release();
    }

    //  Start the graph.
    IMediaControl* control = nullptr;
    graph->QueryInterface(IID_IMediaControl, reinterpret_cast<void**>(&control));
    if (!control){
        m_logger.log("DirectShowAudioCapture: Failed to get IMediaControl.", COLOR_RED);
        grabber->SetCallback(nullptr, 1);
        callback->Release();
        null_renderer->Release();
        grabber->Release();
        grabber_filter->Release();
        source_filter->Release();
        capture_builder->Release();
        graph->Release();
        if (com_ok) CoUninitialize();
        return;
    }
    hr = control->Run();
    if (FAILED(hr)){
        m_logger.log("DirectShowAudioCapture: IMediaControl::Run failed. hr=" + hresult_to_hex(hr), COLOR_RED);
        control->Release();
        grabber->SetCallback(nullptr, 1);
        callback->Release();
        null_renderer->Release();
        grabber->Release();
        grabber_filter->Release();
        source_filter->Release();
        capture_builder->Release();
        graph->Release();
        if (com_ok) CoUninitialize();
        return;
    }

    m_logger.log("DirectShowAudioCapture: Filter graph started. Audio capture running via callback.");

    //  Wait for stop signal.  The callback delivers data on DirectShow threads.
    while (!m_stopping.load(std::memory_order_acquire)){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    m_logger.log(
        "DirectShowAudioCapture: Stopping. Total callbacks=" +
        std::to_string(callback->callback_count()) +
        " total_bytes=" + std::to_string(callback->total_bytes())
    );

    //  Teardown.
    control->Stop();
    grabber->SetCallback(nullptr, 1);  //  Unregister callback before releasing.
    control->Release();
    callback->Release();
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

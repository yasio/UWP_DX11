#include "pch.h"
#include "UWP_DX11Main.h"
#include "Common\DirectXHelper.h"

#include "yasio/obstream.hpp"

using namespace UWP_DX11;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace concurrency;

using namespace yasio;
using namespace yasio::inet;

#if !defined(YASIO_USERAGENT)
#define YASIO_USERAGENT                                                                               \
  "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) "  \
  "Chrome/"                                                                                        \
  "87.0.4820.88"                                                                                   \
  " Safari/537.36"_sv
#endif


// Loads and initializes application assets when the application is loaded.
UWP_DX11Main::UWP_DX11Main(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
    m_deviceResources(deviceResources), m_pointerLocationX(0.0f)
{
    // Register to be notified if the Device is lost or recreated
    m_deviceResources->RegisterDeviceNotify(this);

    // TODO: Replace this with your app's content initialization.
    m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

    m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

UWP_DX11Main::~UWP_DX11Main()
{
    // Deregister device notification
    m_deviceResources->RegisterDeviceNotify(nullptr);
}

void UWP_DX11Main::CreateYAsio()
{
    if (m_service) return;

    yasio::inet::io_hostent endpoints[] = { {"soft.360.cn", 80} };
    m_service = new io_service(endpoints, YASIO_ARRAYSIZE(endpoints));
    m_service->start([=](event_ptr&& event) {
        switch (event->kind())
        {
        case YEK_PACKET: {
            auto packet = std::move(event->packet());
            //total_bytes_transferred += static_cast<int>(packet.size());
            //fwrite(packet.data(), packet.size(), 1, stdout);
            //fflush(stdout);
            packet.push_back('\0');
            OutputDebugStringA(packet.data());
            break;
        }
        case YEK_CONNECT_RESPONSE:
            if (event->status() == 0)
            {
                auto transport = event->transport();
                if (event->cindex() == 0)
                {
                    obstream obs;
                    using namespace cxx17;
                    obs.write_bytes("GET /static/baoku/info_7_0/softinfo_104947374.html HTTP/1.1\r\n"_sv);

                    obs.write_bytes("Host: soft.360.cn\r\n"_sv);

                    obs.write_bytes(YASIO_USERAGENT);
                    obs.write_bytes("Accept: */*;q=0.8\r\n"_sv);
                    obs.write_bytes("Connection: Close\r\n\r\n"_sv);

                    m_service->write(transport, std::move(obs.buffer()));
                }
            }
            break;
        case YEK_CONNECTION_LOST:
            // printf("The connection is lost, %d bytes transferred\n", total_bytes_transferred);
            break;
        }
        });
}

// Updates application state when the window size changes (e.g. device orientation change)
void UWP_DX11Main::CreateWindowSizeDependentResources()
{
    // TODO: Replace this with the size-dependent initialization of your app's content.
    m_sceneRenderer->CreateWindowSizeDependentResources();
}

void UWP_DX11Main::StartRenderLoop()
{
    // If the animation render loop is already running then do not start another thread.
    if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
    {
        return;
    }

    CreateYAsio();

    // Create a task that will be run on a background thread.
    auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction^ action)
        {
            // Calculate the updated frame and render once per vertical blanking interval.
            while (action->Status == AsyncStatus::Started)
            {
                critical_section::scoped_lock lock(m_criticalSection);
                Update();
                if (Render())
                {
                    m_deviceResources->Present();
                }
            }
        });

    // Run task on a dedicated high priority background thread.
    m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void UWP_DX11Main::SendHttpRequest() { m_service->open(0); }

void UWP_DX11Main::StopRenderLoop()
{
    m_renderLoopWorker->Cancel();
}

// Updates the application state once per frame.
void UWP_DX11Main::Update()
{
    ProcessInput();

    // Update scene objects.
    m_timer.Tick([&]()
        {
            // TODO: Replace this with your app's content update functions.
            m_sceneRenderer->Update(m_timer);
            m_fpsTextRenderer->Update(m_timer);
            m_service->dispatch(128);
        });
}

// Process all input from the user before updating game state
void UWP_DX11Main::ProcessInput()
{
    // TODO: Add per frame input handling here.
    m_sceneRenderer->TrackingUpdate(m_pointerLocationX);
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool UWP_DX11Main::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return false;
    }

    auto context = m_deviceResources->GetD3DDeviceContext();

    // Reset the viewport to target the whole screen.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    // Reset render targets to the screen.
    ID3D11RenderTargetView* const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
    context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

    // Clear the back buffer and depth stencil view.
    context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
    context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Render the scene objects.
    // TODO: Replace this with your app's content rendering functions.
    m_sceneRenderer->Render();
    m_fpsTextRenderer->Render();

    return true;
}

// Notifies renderers that device resources need to be released.
void UWP_DX11Main::OnDeviceLost()
{
    m_sceneRenderer->ReleaseDeviceDependentResources();
    m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void UWP_DX11Main::OnDeviceRestored()
{
    m_sceneRenderer->CreateDeviceDependentResources();
    m_fpsTextRenderer->CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

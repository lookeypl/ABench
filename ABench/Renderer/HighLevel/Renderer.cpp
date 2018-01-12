#include "PCH.hpp"
#include "Renderer.hpp"

#include "Renderer/LowLevel/DescriptorAllocator.hpp"
#include "Renderer/LowLevel/Extensions.hpp"
#include "Renderer/LowLevel/Translations.hpp"

#include "Common/Logger.hpp"
#include "Math/Plane.hpp"

#include "DescriptorLayoutManager.hpp"

#include <glslang/Public/ShaderLang.h>


namespace ABench {
namespace Renderer {


Renderer::Renderer()
    : mInstance(nullptr)
    , mDevice(nullptr)
    , mBackbuffer()
    , mImageAcquiredSem()
    , mRenderFinishedSem()
    , mFrameFence()
    , mForwardPass()
{
}

Renderer::~Renderer()
{
    glslang::FinalizeProcess();
}

bool Renderer::Init(const Common::Window& window, bool debugEnable, bool debugVerbose)
{
    if (!glslang::InitializeProcess())
    {
        LOGE("Failed to initialize glslang");
        return false;
    }

    VkDebugReportFlagsEXT debugFlags = 0;

    if (debugEnable)
    {
        debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT;
        if (debugVerbose)
            debugFlags = VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
    }

    mInstance = std::make_shared<Instance>();
    if (!mInstance->Init(debugFlags))
        return false;

    mDevice = std::make_shared<Device>();
    if (!mDevice->Init(mInstance))
        return false;

    // initialize Descriptor Allocator
    DescriptorAllocatorDesc daDesc;
    ZERO_MEMORY(daDesc);
    daDesc.limits[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] = 1;
    daDesc.limits[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = 4;
    daDesc.limits[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] = 3;
    daDesc.limits[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] = 1000;
    if (!DescriptorAllocator::Instance().Init(mDevice, daDesc))
        return false;

    if (!ResourceManager::Instance().Init(mDevice))
        return false;

    if (!GridFrustumsGenerator::Instance().Init(mDevice))
        return false;

    if (!DescriptorLayoutManager::Instance().Init(mDevice))
        return false;

    BackbufferWindowDesc bbWindowDesc;
#ifdef WIN32
    bbWindowDesc.hInstance = window.GetInstance();
    bbWindowDesc.hWnd = window.GetHandle();
#elif defined(__linux__) | defined(__LINUX__)
    bbWindowDesc.connection = window.GetConnection();
    bbWindowDesc.window = window.GetWindow();
#else
#error "Target platform not supported"
#endif

    BackbufferDesc bbDesc;
    bbDesc.instancePtr = mInstance;
    bbDesc.windowDesc = bbWindowDesc;
    bbDesc.requestedFormat = VK_FORMAT_B8G8R8A8_UNORM;
    bbDesc.width = window.GetWidth();
    bbDesc.height = window.GetHeight();
    bbDesc.vsync = false;
    if (!mBackbuffer.Init(mDevice, bbDesc))
        return false;

    // Synchronization primitives
    mImageAcquiredSem = Tools::CreateSem(mDevice);
    if (!mImageAcquiredSem)
        return false;

    mDepthFinishedSem = Tools::CreateSem(mDevice);
    if (!mDepthFinishedSem)
        return false;

    mRenderFinishedSem = Tools::CreateSem(mDevice);
    if (!mRenderFinishedSem)
        return false;

    mFrameFence = Tools::CreateFence(mDevice, true);
    if (!mFrameFence)
        return false;

    // Rendering passes
    DepthPrePassDesc dppDesc;
    dppDesc.width = mBackbuffer.GetWidth();
    dppDesc.height = mBackbuffer.GetHeight();
    if (!mDepthPrePass.Init(mDevice, dppDesc))
        return false;

    ForwardPassDesc fpDesc;
    fpDesc.width = mBackbuffer.GetWidth();
    fpDesc.height = mBackbuffer.GetHeight();
    fpDesc.outputFormat = mBackbuffer.GetFormat();
    if (!mForwardPass.Init(mDevice, fpDesc))
        return false;

    return true;
}

void Renderer::Draw(const Scene::Scene& scene, const Scene::Camera& camera)
{
    // Perform view frustum culling for next scene
    scene.ForEachObject([&](const Scene::Object* o) -> bool {
        if (o->GetComponent()->GetType() == Scene::ComponentType::Model)
        {
            Scene::Model* model = dynamic_cast<Scene::Model*>(o->GetComponent());

            // FIXME below assumption is incorrect - Transform should be used to
            // rotate model's AABB and then it should be re-aligned to axes
            o->SetToRender(camera.Intersects(Math::AABB(
                o->GetTransform() * model->GetAABB()[Math::AABB::AABBVert::MIN],
                o->GetTransform() * model->GetAABB()[Math::AABB::AABBVert::MAX]
            )));
        }

        return true;
    });

    // Wait for previous frame
    VkFence fences[] = { mFrameFence };
    VkResult result = vkWaitForFences(mDevice->GetDevice(), 1, fences, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS)
        LOGW("Failed to wait for fence: " << result << " (" << TranslateVkResultToString(result) << ")");

    result = vkResetFences(mDevice->GetDevice(), 1, fences);
    if (result != VK_SUCCESS)
        LOGW("Failed to reset frame fence: " << result << " (" << TranslateVkResultToString(result) << ")");

    // Rendering
    if (!mBackbuffer.AcquireNextImage(mImageAcquiredSem))
        LOGE("Failed to acquire next image for rendering");

    mDepthPrePass.Draw(scene, camera, mImageAcquiredSem, mDepthFinishedSem, VK_NULL_HANDLE);
    mForwardPass.Draw(scene, camera, mDepthFinishedSem, mRenderFinishedSem, mFrameFence);

    if (!mBackbuffer.Present(mForwardPass.GetTargetTexture(), mRenderFinishedSem))
        LOGE("Error during image presentation");
}

void Renderer::WaitForAll() const
{
    mDevice->Wait(DeviceQueueType::GRAPHICS);
    mDevice->Wait(DeviceQueueType::COMPUTE);
    mDevice->Wait(DeviceQueueType::TRANSFER);
}

} // namespace Renderer
} // namespace ABench

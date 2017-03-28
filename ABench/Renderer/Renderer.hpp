#pragma once

#include "Instance.hpp"
#include "Device.hpp"
#include "Tools.hpp"
#include "Backbuffer.hpp"
#include "Framebuffer.hpp"
#include "Buffer.hpp"
#include "Shader.hpp"
#include "CommandBuffer.hpp"
#include "VertexLayout.hpp"

#include "Common/Window.hpp"

#include "Scene/Camera.hpp"


namespace ABench {
namespace Renderer {

class Renderer final
{
    Instance mInstance;
    Device mDevice;
    Tools mTools;
    Backbuffer mBackbuffer;
    Framebuffer mFramebuffer;
    Buffer mVertexBuffer;
    VertexLayout mVertexLayout;
    Shader mVertexShader;
    Shader mFragmentShader;
    Pipeline mPipeline;
    CommandBuffer mCommandBuffer;

    VkRenderPass mRenderPass;
    VkPipelineLayout mPipelineLayout;

    VkDescriptorSet mVertexShaderSet;
    VkDescriptorSetLayout mVertexShaderLayout;
    VkDescriptorPool mDescriptorPool;
    Buffer mVertexShaderCBuffer;

public:
    Renderer();
    ~Renderer();

    bool Init(const Common::Window& window, bool debugEnable = false, bool debugVerbose = false);
    void Draw(const Scene::Camera& camera);
};

} // namespace Renderer
} // namespace ABench

#pragma once

#include "Prerequisites.hpp"
#include "Device.hpp"
#include "Framebuffer.hpp"
#include "Pipeline.hpp"
#include "Buffer.hpp"


namespace ABench {
namespace Renderer {

enum ClearTypes
{
    ABENCH_CLEAR_COLOR = 1,
    ABENCH_CLEAR_DEPTH = 2,
};

class CommandBuffer
{
    friend class Device;
    friend class Texture;

    VkCommandBuffer mCommandBuffer;
    Framebuffer* mCurrentFramebuffer;

public:
    CommandBuffer();
    ~CommandBuffer();

    bool Init();

    void Begin();
    void BeginRenderPass(VkRenderPass rp, Framebuffer* fb, ClearTypes types, float clearValues[4], float depthValue = 0.0f);
    void BindVertexBuffer(const Buffer* buffer);
    void BindIndexBuffer(const Buffer* buffer);
    void BindPipeline(Pipeline* pipeline);
    void BindDescriptorSet(VkDescriptorSet set, VkPipelineLayout layout);
    void BindDescriptorSet(VkDescriptorSet set, VkPipelineLayout layout, uint32_t dynamicOffset);
    void Clear(ClearTypes types, float clearValues[4], float depthValue);
    void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
    void CopyBufferToTexture(Buffer* src, Texture* dst);
    void Draw(uint32_t vertCount);
    void DrawIndexed(uint32_t vertCount);
    void EndRenderPass();
    bool End();
    void SetViewport(int32_t x, int32_t y, uint32_t width, uint32_t height, float minDepth, float maxDepth);
    void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);
};

} // namespace Renderer
} // namespace ABench

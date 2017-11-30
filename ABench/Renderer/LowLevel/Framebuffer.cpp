#include "PCH.hpp"
#include "Framebuffer.hpp"
#include "Util.hpp"
#include "Extensions.hpp"
#include "Renderer/HighLevel/Renderer.hpp"

#include "Common/Common.hpp"
#include "Common/Logger.hpp"


namespace ABench {
namespace Renderer {

Framebuffer::Framebuffer()
{
}

Framebuffer::~Framebuffer()
{
    if (mFramebuffers.size() > 0)
        for (auto& fb: mFramebuffers)
            vkDestroyFramebuffer(mDevice->GetDevice(), fb, nullptr);
}

bool Framebuffer::Init(const DevicePtr& device, const FramebufferDesc& desc)
{
    mDevice = device;

    mFramebuffers.resize(desc.colorTex->mImages.size());

    if (!desc.colorTex)
    {
        LOGE("Framebuffer requires at least a color texture");
        return false;
    }

    if ((desc.depthTex) &&
        ((desc.colorTex->mWidth != desc.depthTex->mWidth) ||
         (desc.colorTex->mHeight != desc.depthTex->mHeight)))
    {
        LOGE("Cannot create framebuffer - provided color and depth textures are incompatible");
        return false;
    }

    VkFramebufferCreateInfo fbInfo;
    ZERO_MEMORY(fbInfo);
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.width = desc.colorTex->mWidth;
    fbInfo.height = desc.colorTex->mHeight;
    fbInfo.renderPass = desc.renderPass;
    fbInfo.layers = 1;

    VkResult result;
    VkImageView fbAtts[2];
    for (uint32_t i = 0; i < mFramebuffers.size(); ++i)
    {
        fbAtts[0] = desc.colorTex->mImages[i].view;
        fbInfo.attachmentCount = 1;
        if (desc.depthTex)
        {
            fbAtts[1] = desc.depthTex->mImages[0].view;
            fbInfo.attachmentCount++;
        }

        fbInfo.pAttachments = fbAtts;
        result = vkCreateFramebuffer(mDevice->GetDevice(), &fbInfo, nullptr, &mFramebuffers[i]);
        RETURN_FALSE_IF_FAILED(result, "Failed to create Framebuffer #" << i << " for provided Texture");
    }

    mWidth = desc.colorTex->mWidth;
    mHeight = desc.colorTex->mHeight;
    mTexturePtr = desc.colorTex;
    if (desc.depthTex)
        mDepthTexturePtr = desc.depthTex;
    return true;
}

} // namespace Renderer
} // namespace ABench
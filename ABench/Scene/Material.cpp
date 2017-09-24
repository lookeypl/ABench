#include "PCH.hpp"
#include "Material.hpp"
#include "Common/Logger.hpp"


namespace ABench {
namespace Scene {

Material::Material(const std::string& name)
    : mMaterialName(name)
{
}

Material::~Material()
{
}

bool Material::CreateRendererTexture(const std::string& imagePath, VkImageUsageFlags usage, Renderer::Texture& texture)
{
    Common::Image image;
    if (!image.Init(imagePath, true))
        return false;

    std::vector<Renderer::TextureDataDesc> textures;
    textures.reserve(image.GetMipmapCount());
    for (uint32_t i = 0; i < image.GetMipmapCount(); ++i)
        textures.emplace_back(image.GetSubimageData(i), image.GetSubimageSize(i));

    Renderer::TextureDesc texDesc;
    texDesc.width = image.GetWidth();
    texDesc.height = image.GetHeight();
    texDesc.usage = usage;
    texDesc.data = textures.data();
    texDesc.mipmapCount = static_cast<uint32_t>(textures.size());

    switch (image.GetColorType())
    {
    case FIC_RGBALPHA:
        texDesc.format = VK_FORMAT_B8G8R8A8_UNORM;
        break;
    default:
        LOGE("Incorrect color type in loaded image - cannot match format");
        return false;
    }

    return texture.Init(texDesc);
}

bool Material::Init(const MaterialDesc& desc)
{
    if (desc.diffusePath.empty())
    {
        LOGE("Diffuse texture is required");
        return false;
    }

    if (!CreateRendererTexture(desc.diffusePath, VK_IMAGE_USAGE_SAMPLED_BIT, mDiffuseTexture))
    {
        LOGE("Failed to create diffuse texture for material " << mMaterialName);
        return false;
    }

    // normal texture is optional - create it only when provided
    if (!desc.normalPath.empty())
    {
        if (!CreateRendererTexture(desc.normalPath, VK_IMAGE_USAGE_SAMPLED_BIT, mNormalTexture))
        {
            LOGE("Failed to create normal texture for material " << mMaterialName);
            return false;
        }
    }

    return true;
}

} // namespace Scene
} // namespace ABench

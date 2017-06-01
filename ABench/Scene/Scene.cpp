#include "PCH.hpp"
#include "Scene.hpp"

#include "Common/Logger.hpp"

namespace ABench {
namespace Scene {

Scene::Scene()
{
}

Scene::~Scene()
{
}

bool Scene::Init(const std::string& fbxFile)
{
    if (!fbxFile.empty())
    {
        LOGD("Loading scene from FBX file");

        if (!mFBXFile.Open(fbxFile))
        {
            LOGE("Failed to open FBX scene file.");
            return false;
        }

        mFBXFile.Traverse([&](FbxNode* node) {
            uint32_t attributeCount = node->GetNodeAttributeCount();

            for (uint32_t i = 0; i < attributeCount; ++i)
            {
                FbxNodeAttribute* attr = node->GetNodeAttributeByIndex(i);

                if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
                {
                    if (!node->GetMesh()->IsTriangleMesh())
                    {
                        LOGW("Mesh " << node->GetName() << " requires triangulation - converting, this might take some time.");
                        mFBXFile.GetConverter()->Triangulate(attr, true);
                    }

                    // check if we have materials
                    if (node->GetSrcObjectCount<FbxSurfaceMaterial>() == 0)
                    {
                        LOGW("Mesh " << node->GetName() << " has no materials - skipping");
                        return;
                    }

                    FbxSurfaceMaterial* material = node->GetSrcObject<FbxSurfaceMaterial>(0);
                    FbxProperty prop = material->FindProperty("DiffuseColor");
                    if (!prop.IsValid())
                    {
                        LOGW("Material " << node->GetName() << "::" << material->GetName() << " has no DiffuseColor property - skipping");
                        return;
                    }

                    FbxTexture* tex = prop.GetSrcObject<FbxTexture>(0);
                    FbxFileTexture* ftex = FbxCast<FbxFileTexture>(tex);
                    if (ftex == nullptr)
                    {
                        LOGW("Texture is not from file - skipping");
                        return;
                    }

                    Material* mat = CreateMaterial();
                    MaterialDesc matDesc;
                    matDesc.diffusePath = ftex->GetFileName();
                    if (!mat->Init(matDesc))
                    {
                        LOGE("Failed to create material for mesh " << node->GetName());
                        return;
                    }

                    Object* o = CreateObject();
                    Mesh* m = dynamic_cast<Mesh*>(CreateComponent(ComponentType::Mesh));

                    m->Init(node->GetMesh());
                    m->SetMaterial(mat);
                    o->SetComponent(m);

                    o->SetScale(static_cast<float>(node->LclScaling.Get()[0]),
                                static_cast<float>(node->LclScaling.Get()[1]),
                                static_cast<float>(node->LclScaling.Get()[2]));
                    o->SetPosition(static_cast<float>(node->LclTranslation.Get()[0]),
                                   static_cast<float>(node->LclTranslation.Get()[1]),
                                   static_cast<float>(node->LclTranslation.Get()[2]));
                }
            }
        });
    }
    else
        LOGI("Initialized empty scene");

    return true;
}

Object* Scene::CreateObject()
{
    mObjects.emplace_back();
    return &mObjects.back();
}

Component* Scene::CreateComponent(ComponentType type)
{
    switch (type)
    {
    case ComponentType::Mesh:
        mMeshComponents.emplace_back();
        return &mMeshComponents.back();

    default:
        return nullptr;
    }
}

Material* Scene::CreateMaterial()
{
    mMaterials.emplace_back();
    return &mMaterials.back();
}

void Scene::ForEachObject(Scene::ObjectCallback func) const
{
    for (auto& o: mObjects)
        func(&o);
}

} // namespace Scene
} // namespace ABench

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

bool Scene::Init(const std::string fbxFile)
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
                    Object* o = CreateObject();
                    Mesh* m = dynamic_cast<Mesh*>(CreateComponent(ComponentType::Mesh));

                    o->SetComponent(m);
                    // TODO load mesh
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

void Scene::ForEachObject(Scene::ObjectCallback func) const
{
    for (auto& o: mObjects)
        func(&o);
}

} // namespace Scene
} // namespace ABench

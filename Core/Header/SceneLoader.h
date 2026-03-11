#ifndef SCENE_LOADER
#define SCENE_LOADER

#include "Object.h"
#include "SerialParser.h"
#include "Scene.h"
#include "Rigidbody.h"
#include "MeshCollider.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

#include <string>

class Scene;

namespace Apex::Serialization
{
    class SceneLoader
    {
    public:
        SceneLoader(Apex::Resources::ResourceManager& resMgr) : m_resourceManager(resMgr) {}

        void LoadScene(Apex::Rendering::Scene& scene, const std::string& path);

    private:
        Apex::Resources::ResourceManager& m_resourceManager;

        Object* ParseObject(Apex::Rendering::Scene& scene, Apex::Serialization::SerialParser& parser);
        void ParseComponents(Object* obj, Apex::Serialization::SerialParser& parser);

        Apex::Physic::BodyType StringToBodyType(const std::string& str);
        Apex::Physic::MeshColliderType StringToMeshColliderType(const std::string& str);

        Apex::Resources::ResourceHandle<Mesh> GetMeshHandle(const std::string& path);
        Apex::Resources::ResourceHandle<Texture> GetTextureHandle(const std::string& path);
        Apex::Resources::ResourceHandle<Shader> GetShaderHandle(const std::string& path);
        Apex::Resources::ResourceHandle<PhysicMaterial> GetPhysMatHandle(const std::string& path);
    };
}

#endif // !SCENE_LOADER

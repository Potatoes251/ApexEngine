#ifndef SCENE_LOADER
#define SCENE_LOADER

#include "Object.h"
#include "SerializationParser.h"
#include "Scene.h"
#include "Rigidbody.h"
#include "MeshCollider.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "Physic.h"

#include <string>
#include <vector>
#include <unordered_map>

class Scene;

namespace Apex::Serialization
{
    class SceneLoader
    {
    public:
        SceneLoader(Apex::Resources::ResourceManager& resMgr, Apex::Physic::PhysicSystem& physics) : m_resourceManager(resMgr), m_physics(physics) {}

        void LoadScene(Apex::Rendering::Scene& scene, const std::string& path);

    private:
        Apex::Resources::ResourceManager& m_resourceManager;
        Apex::Physic::PhysicSystem& m_physics;
        std::unordered_map<size_t, Apex::Physic::PhysMatHandle> m_materials;

        std::unordered_map<size_t, Apex::Data::Object*> objectsById;
        std::vector<std::pair<size_t, size_t>> parentLinks;

        Apex::Data::Object* ParseObject(Apex::Rendering::Scene& scene, Apex::Serialization::SerialParser& parser);
        void ParseComponents(Apex::Data::Object* obj, Apex::Serialization::SerialParser& parser);
        void ParseMaterials(SerialParser& parser);

        Apex::Physic::BodyType StringToBodyType(const std::string& str);
        Apex::Physic::MeshColliderType StringToMeshColliderType(const std::string& str);

        Apex::Resources::ResourceHandle<Mesh> GetMeshHandle(const std::string& path);
        Apex::Resources::ResourceHandle<Texture> GetTextureHandle(const std::string& path);
        Apex::Resources::ResourceHandle<Shader> GetShaderHandle(const std::string& path);
    };
}

#endif // !SCENE_LOADER

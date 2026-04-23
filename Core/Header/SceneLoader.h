#ifndef SCENE_LOADER
#define SCENE_LOADER

#include "SerializationParser.h"
#include "Rigidbody.h"
#include "MeshCollider.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "Physic.h"
#include "LuaManager.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace Apex::Data { class Object; }
namespace Apex::Rendering { class Scene; }
namespace Apex::Scripting { class ScriptComponent; }
namespace Apex::Resources { class ResourceManager; }

namespace Apex::Serialization
{
    class SceneLoader
    {
    public:
        SceneLoader(Resources::ResourceManager& resMgr, 
            Apex::Physic::PhysicSystem& physics, 
            Apex::Scripting::LuaManager& luaManager) 
            : m_resourceManager(resMgr), m_physics(physics), 
            m_luaManager(luaManager) {}

        void LoadScene(Apex::Rendering::Scene& scene, const std::string& path);

    private:

        Data::Object* ParseObject(Apex::Rendering::Scene& scene, Apex::Serialization::SerialParser& parser);
        void ParseComponents(Data::Object* obj, Apex::Serialization::SerialParser& parser);
        void ParseScriptVar(Scripting::ScriptComponent& script, SerialParser& parser);
        void ParseMaterials(SerialParser& parser);

        Apex::Physic::BodyType StringToBodyType(const std::string& str);
        Apex::Physic::MeshColliderType StringToMeshColliderType(const std::string& str);

        Apex::Resources::ResourceHandle<Mesh> GetMeshHandle(const std::string& path);
        Apex::Resources::ResourceHandle<Texture> GetTextureHandle(const std::string& path);
        Apex::Resources::ResourceHandle<Shader> GetShaderHandle(const std::string& path);

        Apex::Resources::ResourceManager& m_resourceManager;
        Apex::Physic::PhysicSystem& m_physics;
        std::unordered_map<size_t, Apex::Physic::PhysMatHandle> m_materials;
        Apex::Scripting::LuaManager& m_luaManager;

        std::unordered_map<size_t, Apex::Data::Object*> objectsById;
        std::vector<std::pair<size_t, size_t>> parentLinks;
    };
}

#endif // !SCENE_LOADER

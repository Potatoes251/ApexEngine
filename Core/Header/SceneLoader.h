#ifndef SCENE_LOADER
#define SCENE_LOADER

#include "Rigidbody.h"
#include "MeshCollider.h"
#include "Physic.h"

#include "SerializationParser.h"
#include "Material.h"
#include "LuaManager.h"

#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

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
        SceneLoader(Resources::ResourceManager* resMgr, 
            Apex::Physic::PhysicSystem* physics, 
            Apex::Scripting::LuaManager* luaManager) 
            : m_resourceManager(resMgr), m_physics(physics), 
            m_luaManager(luaManager) {}

        void LoadScene(Apex::Rendering::Scene& scene, const std::string& path);

    private:

        Data::Object* ParseObject(Apex::Rendering::Scene& scene, Apex::Serialization::SerialParser& parser);
        void ParseComponents(Data::Object* obj, Apex::Serialization::SerialParser& parser);

        void ParseMeshRenderer(Data::Object* obj, SerialParser& parser);
        void ParseRigidBody(Data::Object* obj, SerialParser& parser);
        void ParseMeshCollider(Data::Object* obj, SerialParser& parser);
        void ParseBoxCollider(Data::Object* obj, SerialParser& parser);
        void ParseCapsuleCollider(Data::Object* obj, SerialParser& parser);
        void ParseDirectionalLight(Data::Object* obj, SerialParser& parser);
        void ParseSpotLight(Data::Object* obj, SerialParser& parser);
        void ParsePointLight(Data::Object* obj, SerialParser& parser);
        void ParseCharacterController(Data::Object* obj, SerialParser& parser);
        void ParseAIController(Data::Object* obj, SerialParser& parser);
        void ParseThirdPersonCamera(Data::Object* obj, SerialParser& parser);
        void ParseFirstPersonCamera(Data::Object* obj, SerialParser& parser);
        void ParseScriptComponent(Data::Object* obj, SerialParser& parser);
        void ParseAnimator(Data::Object* obj, SerialParser& parser);
        void ParseWaterVolume(Data::Object* obj, SerialParser& parser);
        void ParseViewComponent(Data::Object* obj, SerialParser& parser);
        void ParseAudioComponent(Data::Object* obj, SerialParser& parser);
        void ParseWaypointComponent(Data::Object* obj, SerialParser& parser);
		void ParseProjectileComponent(Data::Object* obj, SerialParser& parser);

        void ParseScriptVar(Scripting::ScriptComponent& script, SerialParser& parser);

        Apex::Physic::BodyType StringToBodyType(const std::string& str);
        Apex::Physic::MeshColliderType StringToMeshColliderType(const std::string& str);

        Apex::Resources::ResourceHandle<Model> GetModelHandle(const std::string& path);
        Apex::Resources::ResourceHandle<Texture> GetTextureHandle(const std::string& path);
        Apex::Resources::ResourceHandle<Rendering::Material> GetMaterialHandle(const std::string& path);
        Apex::Resources::ResourceHandle<Shader> GetShaderHandle(const std::string& path);

        Apex::Resources::ResourceManager* m_resourceManager;
        Apex::Physic::PhysicSystem* m_physics;
        Apex::Scripting::LuaManager* m_luaManager;

        std::unordered_map<size_t, Apex::Data::Object*> m_objectsById;
        std::vector<std::pair<size_t, size_t>> m_parentLinks;
    };
}

#endif // !SCENE_LOADER

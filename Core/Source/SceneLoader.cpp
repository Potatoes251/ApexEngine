#include "SceneLoader.h"

#include "Scene.h"
#include "Object.h"
#include "Animator.h"
#include "Waypoint.h"
#include "Rigidbody.h"
#include "Colliders.h"
#include "WaterVolume.h"
#include "AIController.h"
#include "MeshRenderer.h"
#include "ViewComponent.h"
#include "AudioComponent.h"
#include "Lighting/Lights.h"
#include "ScriptComponent.h"
#include "ThirdPersonCamera.h"
#include "FirstPersonCamera.h"
#include "CharacterController.h"
#include "ProjectileComponent.h"

#include "Application.h"
#include "Log.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cassert>

using namespace Apex::Data;
using namespace Apex::Physic;
using namespace Apex::Serialization;

void SceneLoader::LoadScene(Apex::Rendering::Scene& scene, const std::string& path)
{
    std::filesystem::path graphPath{ path };
    scene.GetGraph()->Load(graphPath.replace_extension(".nav").string());
    scene.SetName(path);

    std::ifstream file(path);
    assert(file && "Cannot open scene file");
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();

    SerialParser parser(text);
    
    parser.Expect('{');

    while (!parser.Peek('}'))
    {
        std::string category = parser.ParseString();
        parser.Expect(':');

        if (category == "objects")
        {
            parser.Expect('[');

            while (!parser.Peek(']'))
            {
                ParseObject(scene, parser);

                if (parser.Peek(','))
                    parser.Expect(',');
            }

            parser.Expect(']');
        }

        if (parser.Peek(','))
            parser.Expect(',');
    }

    parser.Expect('}');

    for (auto& [childId, parentId] : m_parentLinks)
    {
        auto child = m_objectsById[childId];
        auto parent = m_objectsById[parentId];

        if (child && parent)
        {
            child->GetSceneNodeRaw()->SetParent(parent->GetSceneNode());
        }
    }
}

Object* SceneLoader::ParseObject(Apex::Rendering::Scene& scene, SerialParser& parser)
{
    parser.Expect('{');

    Object* obj = scene.CreateObject();

    // id
    parser.ParseString(); parser.Expect(':');
    size_t id = parser.ParseInt();
    obj->SetId(id);

    m_objectsById[id] = obj;
    parser.Expect(',');

    // parent
    parser.ParseString(); parser.Expect(':');

    if (parser.Match("null"))
    {
        // root object
    }
    else
    {
        size_t parentId = parser.ParseInt();
        m_parentLinks.emplace_back(id, parentId);
    }

    parser.Expect(',');

    // name
    parser.ParseString(); parser.Expect(':');
    obj->SetName(parser.ParseString());
    parser.Expect(',');

    // position
    LibMath::Transform transform = obj->GetLocalTransform();
    parser.ParseString(); parser.Expect(':');
    transform.setPosition(parser.ParseVector3());
    parser.Expect(',');

    // rotation
    parser.ParseString(); parser.Expect(':');
    transform.setRotation(parser.ParseQuaternion());
    parser.Expect(',');

    // scale
    parser.ParseString(); parser.Expect(':');
    transform.setScale(parser.ParseVector3());
    parser.Expect(',');
    obj->SetLocalTransform(transform);

    // components
    parser.ParseString(); parser.Expect(':');
    ParseComponents(obj, parser);

    parser.Expect('}');

    return obj;
}

void SceneLoader::ParseComponents(Object* obj, SerialParser& parser)
{
    parser.Expect('{'); // "components" category start

    while (!parser.Peek('}'))
    {
        std::string compName = parser.ParseString(); // "MeshRenderer", "RigidBody", etc.
        parser.Expect(':');
        parser.Expect('{'); // component start

        if (compName == "MeshRenderer")
        {
            ParseMeshRenderer(obj, parser);
        }
        else if (compName == "Rigidbody")
        {
            ParseRigidBody(obj, parser);
        }
        else if (compName == "MeshCollider")
        {
            ParseMeshCollider(obj, parser);
        }
        else if (compName == "BoxCollider")
        {
            ParseBoxCollider(obj, parser);
        }
        else if (compName == "CapsuleCollider")
        {
            ParseCapsuleCollider(obj, parser);
        }
        else if (compName == "DirectionalLight")
        {
            ParseDirectionalLight(obj, parser);
        }
        else if (compName == "SpotLight")
        {
            ParseSpotLight(obj, parser);
        }
        else if (compName == "PointLight")
        {
            ParsePointLight(obj, parser);
        }
        else if (compName == "CharacterController")
        {
            ParseCharacterController(obj, parser);
        }
        else if (compName == "AIController")
        {
            ParseAIController(obj, parser);
        }
        else if (compName == "ThirdPersonCamera")
        {
            ParseThirdPersonCamera(obj, parser);
        }
        else if (compName == "FirstPersonCamera")
        {
            ParseFirstPersonCamera(obj, parser);
        }
        else if (compName == "ScriptComponent")
        {
            ParseScriptComponent(obj, parser);
        }
        else if (compName == "Animator")
        {
            ParseAnimator(obj, parser);
        }
        else if (compName == "WaterVolume")
        {
            ParseWaterVolume(obj, parser);
        }
        else if (compName == "ViewComponent")
        {
            ParseViewComponent(obj, parser);
        }
        else if (compName == "AudioComponent")
        {
            ParseAudioComponent(obj, parser);
        }
        else if (compName == "NavGenVolume")
        {
            ParseWaypointComponent(obj, parser);
        }
		else if (compName == "ProjectileComponent")
		{
			ParseProjectileComponent(obj, parser);
		}

        parser.Expect('}'); // component end

        if (parser.Peek(','))
            parser.Expect(',');
    }

    parser.Expect('}'); // "components" category end
}

void SceneLoader::ParseMeshRenderer(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString(); parser.Expect(':');
    bool enabled = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString(); parser.Expect(':');
    std::string mesh = parser.ParseString();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    parser.Expect('[');

    std::vector<ResourceHandle<Apex::Rendering::Material>> materials;

    while (!parser.Peek(']'))
    {
        std::string material = parser.ParseString();

        materials.push_back(GetMaterialHandle(material));

        if (parser.Peek(','))
            parser.Expect(',');
    }

    parser.Expect(']');
    parser.Expect(',');

    parser.ParseString(); parser.Expect(':');
    bool shadow = parser.ParseBool();

    auto& comp = obj->AddComponent<Apex::Rendering::MeshRenderer>();
    comp.SetEnabled(enabled);

    comp.SetModel(GetModelHandle(mesh));
    comp.SetMaterials(materials);
    comp.SetCastShadow(shadow);
}

void SceneLoader::ParseRigidBody(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString(); parser.Expect(':');
    bool enabled = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString(); parser.Expect(':');
    std::string type = parser.ParseString();
    parser.Expect(',');

    parser.ParseString(); parser.Expect(':');
    float mass = parser.ParseFloat();

    auto& comp = obj->AddComponent<Apex::Physic::RigidBodyComponent>(mass, StringToBodyType(type));
    comp.SetEnabled(enabled);
}

void SceneLoader::ParseMeshCollider(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    bool isTrigger = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float staticFriction = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float dynamicFriction = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float bounciness = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    std::string meshPath = parser.ParseString();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    std::string type = parser.ParseString();

    auto& comp = obj->AddComponent<Apex::Physic::MeshCollider>(GetModelHandle(meshPath), StringToMeshColliderType(type), isTrigger);
    comp.SetEnabled(enabled);
    comp.SetStaticFriction(staticFriction);
    comp.SetDynamicFriction(dynamicFriction);
    comp.SetBounciness(bounciness);
}

void SceneLoader::ParseBoxCollider(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    bool isTrigger = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float staticFriction = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float dynamicFriction = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float bounciness = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    LibMath::Vector3 halfExtent = parser.ParseVector3();

    auto& comp = obj->AddComponent<Apex::Physic::BoxCollider>(halfExtent, isTrigger);
    comp.SetEnabled(enabled);
    comp.SetStaticFriction(staticFriction);
    comp.SetDynamicFriction(dynamicFriction);
    comp.SetBounciness(bounciness);
}

void SceneLoader::ParseCapsuleCollider(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    bool isTrigger = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float staticFriction = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float dynamicFriction = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float bounciness = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float halfHeight = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float radius = parser.ParseFloat();

    auto& comp = obj->AddComponent<Apex::Physic::CapsuleCollider>(halfHeight, radius, isTrigger);
    comp.SetEnabled(enabled);
    comp.SetStaticFriction(staticFriction);
    comp.SetDynamicFriction(dynamicFriction);
    comp.SetBounciness(bounciness);
}

void SceneLoader::ParseDirectionalLight(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    LibMath::Vector3 color = parser.ParseVector3();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float intensity = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float distance = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float extent = parser.ParseFloat();

    auto& comp = obj->AddComponent<Apex::Lighting::DirectionalLightComponent>(color, intensity, distance, extent);
    comp.SetEnabled(enabled);
}

void SceneLoader::ParseSpotLight(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    LibMath::Vector3 color = parser.ParseVector3();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float intensity = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float innerCutoff = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float outerCutoff = parser.ParseFloat();

    auto& comp = obj->AddComponent<Apex::Lighting::SpotLightComponent>(color, intensity, innerCutoff, outerCutoff);
    comp.SetEnabled(enabled);
}

void SceneLoader::ParsePointLight(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    LibMath::Vector3 color = parser.ParseVector3();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float intensity = parser.ParseFloat();
    parser.Expect(',');
    parser.ParseString();
    parser.Expect(':');
    float radius = parser.ParseFloat();

    auto& comp = obj->AddComponent<Apex::Lighting::PointLightComponent>(color, intensity, radius);
    comp.SetEnabled(enabled);
}

void SceneLoader::ParseCharacterController(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();

    parser.Expect(',');
    parser.ParseString();
    parser.Expect(':');
    float groundedDistance = parser.ParseFloat();

    parser.Expect(',');
    parser.ParseString();
    parser.Expect(':');
    float speed = parser.ParseFloat();

    parser.Expect(',');
    parser.ParseString();
    parser.Expect(':');
    float rotationSpeed = parser.ParseFloat();

    parser.Expect(',');
    parser.ParseString();
    parser.Expect(':');
    float density = parser.ParseFloat();

    auto& comp = obj->AddComponent<Apex::Controller::CharacterController>();
    comp.SetEnabled(enabled);
    comp.SetSpeed(speed);
    comp.SetRotationSpeed(rotationSpeed);
    comp.SetGroundedDistance(groundedDistance);
    comp.SetDensity(density);
}

void SceneLoader::ParseAIController(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();

    parser.Expect(',');
    parser.ParseString();
    parser.Expect(':');
    float groundedDistance = parser.ParseFloat();

    parser.Expect(',');
    parser.ParseString();
    parser.Expect(':');
    float speed = parser.ParseFloat();

    parser.Expect(',');
    parser.ParseString();
    parser.Expect(':');
    float rotationSpeed = parser.ParseFloat();

    parser.Expect(',');
    parser.ParseString();
    parser.Expect(':');
    float density = parser.ParseFloat();

    parser.Expect(',');
    parser.ParseString();
    parser.Expect(':');
    float arrThres = parser.ParseFloat();

    auto& comp = obj->AddComponent<Apex::Controller::AIController>();
    comp.SetEnabled(enabled);
    comp.SetSpeed(speed);
    comp.SetRotationSpeed(rotationSpeed);
    comp.SetGroundedDistance(groundedDistance);
    comp.SetDensity(density);
    comp.SetArrivalThreshold(arrThres);
}

void SceneLoader::ParseThirdPersonCamera(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();

    parser.Expect(',');
    parser.ParseString();
    parser.Expect(':');
    bool main = parser.ParseBool();

    parser.Expect(',');
    parser.ParseString();
    parser.Expect(':');
    float distance = parser.ParseFloat();

    parser.Expect(',');
    parser.ParseString();
    parser.Expect(':');
    float speed = parser.ParseFloat();

    auto& comp = obj->AddComponent<Apex::Rendering::ThirdPersonCamera>();
    comp.SetEnabled(enabled);
    comp.SetMainCamera(main);
    comp.SetDistance(distance);
    comp.SetSpeed(speed);
}

void SceneLoader::ParseFirstPersonCamera(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();

    parser.Expect(',');
    parser.ParseString();
    parser.Expect(':');
    bool main = parser.ParseBool();

    auto& comp = obj->AddComponent<Apex::Rendering::FirstPersonCamera>();
    comp.SetEnabled(enabled);
    comp.SetMainCamera(main);
}

void SceneLoader::ParseScriptComponent(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString(); parser.Expect(':');
    bool enabled = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString(); parser.Expect(':');
    std::string scriptPath = parser.ParseString();

    auto& comp = obj->AddComponent<Apex::Scripting::ScriptComponent>(m_luaManager);
    comp.SetEnabled(enabled);
    comp.SetScript(scriptPath);

    if (parser.Peek(','))
    {
        parser.Expect(',');

        parser.ParseString(); parser.Expect(':');
        parser.Expect('{');

        while (!parser.Peek('}'))
        {
            ParseScriptVar(comp, parser);

            if (parser.Peek(','))
                parser.Expect(',');
        }

        parser.Expect('}');
    }
}

void SceneLoader::ParseAnimator(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');

    float fps = parser.ParseFloat();

    parser.Expect(',');
    parser.ParseString(); parser.Expect(':');
    std::string meshPath = parser.ParseString();

    Rendering::Animator& comp = obj->AddComponent<Rendering::Animator>(GetModelHandle(meshPath));
    comp.SetFPS(fps);
    comp.SetEnabled(enabled);
}

void SceneLoader::ParseWaterVolume(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');

    LibMath::Vector3 halfExtents = parser.ParseVector3();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');

    float density = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');

    float drag = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');

    float buoyencyMult = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');

    LibMath::Vector3 flowDir = parser.ParseVector3();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');

    float flowSpeed = parser.ParseFloat();

    Physic::WaterVolume& comp = obj->AddComponent<Physic::WaterVolume>(halfExtents, density, drag, buoyencyMult, flowDir, flowSpeed);
    comp.SetEnabled(enabled);
}

void SceneLoader::ParseViewComponent(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float range = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float horizontalFov = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float vertitalFov = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    int horizontalRays = parser.ParseInt();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    int vertitalRays = parser.ParseInt();

    auto& comp = obj->AddComponent<Perception::ViewComponent>(m_physics, range, horizontalFov, vertitalFov, horizontalRays, vertitalRays);
    comp.SetEnabled(enabled);
}

void SceneLoader::ParseAudioComponent(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    std::string name = parser.ParseString();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float volume = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    int channel = parser.ParseInt();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    int loop = parser.ParseInt();

    Audio::IAudioEngine* audio = Application::Get()->GetAudio();

    Audio::AudioComponent& comp = obj->AddComponent<Audio::AudioComponent>(audio, name, channel, volume, loop);
    comp.SetEnabled(enabled);
}

void SceneLoader::ParseWaypointComponent(Data::Object* obj, SerialParser& parser)
{
    parser.ParseString();
    parser.Expect(':');
    bool enabled = parser.ParseBool();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float spacing = parser.ParseFloat();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
    float genHeight = parser.ParseFloat();

    auto& comp = obj->AddComponent<Pathfinding::NavGenVolume>();
    comp.SetEnabled(enabled);
    comp.m_genHeight = genHeight;
    comp.m_spacing = spacing;
}

void Apex::Serialization::SceneLoader::ParseProjectileComponent(Data::Object* obj, SerialParser& parser)
{
	parser.ParseString();
	parser.Expect(':');
	bool enabled = parser.ParseBool();
	parser.Expect(',');

    parser.ParseString();
	parser.Expect(':');
    LibMath::Vector3 direction = parser.ParseVector3();
    parser.Expect(',');

    parser.ParseString();
    parser.Expect(':');
	float speed = parser.ParseFloat();
	parser.Expect(',');

	parser.ParseString();
	parser.Expect(':');
	float lifetime = parser.ParseFloat();

	auto& comp = obj->AddComponent<Gameplay::ProjectileComponent>(direction, speed, lifetime);
	comp.SetEnabled(enabled);
}

void SceneLoader::ParseScriptVar(Scripting::ScriptComponent& script, SerialParser& parser)
{
    std::string varName = parser.ParseString();
    parser.Expect(':');

    char c = parser.PeekChar();

    // STRING
    if (c == '"')
    {
        script.SetVariable(varName, parser.ParseString());
    }
    // BOOL
    else if (c == 't' || c == 'f')
    {
        script.SetVariable(varName, parser.ParseBool());
    }
    // NUMBER
    else
    {
        script.SetVariable(varName, parser.ParseFloat());
    }
}

Apex::Physic::BodyType SceneLoader::StringToBodyType(const std::string& str)
{
    if (str == "Static")   return Apex::Physic::BodyType::Static;
    if (str == "Dynamic")  return Apex::Physic::BodyType::Dynamic;
    if (str == "Kinematic") return Apex::Physic::BodyType::Kinematic;

    assert(false && "Invalid BodyType string");
    return Apex::Physic::BodyType::Static; // fallback
}

Apex::Physic::MeshColliderType SceneLoader::StringToMeshColliderType(const std::string& str)
{
    if (str == "Convex") return Apex::Physic::MeshColliderType::Convex;
    if (str == "Triangle")   return Apex::Physic::MeshColliderType::Triangle;

    assert(false && "Invalid MeshColliderType string");
    return Apex::Physic::MeshColliderType::Convex; // fallback
}

Apex::Resources::ResourceHandle<Model> SceneLoader::GetModelHandle(const std::string& path)
{
    return m_resourceManager->CreateAsync<Model>(path);
}

Apex::Resources::ResourceHandle<Texture> Apex::Serialization::SceneLoader::GetTextureHandle(const std::string& path)
{
    return m_resourceManager->CreateAsync<Texture>(path);
}

Apex::Resources::ResourceHandle<Apex::Rendering::Material> Apex::Serialization::SceneLoader::GetMaterialHandle(const std::string& path)
{
    return m_resourceManager->CreateAsync<Rendering::Material>(path);
}

Apex::Resources::ResourceHandle<Shader> Apex::Serialization::SceneLoader::GetShaderHandle(const std::string& path)
{
    return m_resourceManager->CreateAsync<Shader>(path);
}
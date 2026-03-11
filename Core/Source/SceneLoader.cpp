#include "SceneLoader.h"
#include "Object.h"

#include "MeshRenderer.h"
#include "Rigidbody.h"
#include "BoxCollider.h"
#include "MeshCollider.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace Apex::Serialization;
using namespace Apex::Physic;

void SceneLoader::LoadScene(Apex::Rendering::Scene& scene, const std::string& path)
{
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot open scene file");

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();

    SerialParser parser(text);

    parser.Expect('{');
    parser.ParseString(); // "objects"
    parser.Expect(':');
    parser.Expect('[');

    while (!parser.Peek(']'))
    {
        ParseObject(scene, parser);

        if (parser.Peek(','))
            parser.Expect(',');
    }

    parser.Expect(']');
    parser.Expect('}');
}

Object* SceneLoader::ParseObject(Apex::Rendering::Scene& scene, SerialParser& parser)
{
    parser.Expect('{');

    Object* obj = scene.CreateObject();

    // id
    parser.ParseString(); parser.Expect(':');
    obj->SetId(parser.ParseInt());
    parser.Expect(',');

    // parent
    parser.ParseString(); parser.Expect(':');
    if (parser.Match("null"))
        obj->GetSceneNode()->SetParent(nullptr);
    else
        obj->GetSceneNode()->SetParent(nullptr);
    parser.Expect(',');

    // name
    parser.ParseString(); parser.Expect(':');
    obj->SetName(parser.ParseString());
    parser.Expect(',');

    // position
    parser.ParseString(); parser.Expect(':');
    obj->GetLocalTransform().setPosition(parser.ParseVector3());
    parser.Expect(',');

    // rotation
    parser.ParseString(); parser.Expect(':');
    obj->GetLocalTransform().setRotation(parser.ParseQuaternion());
    parser.Expect(',');

    // scale
    parser.ParseString(); parser.Expect(':');
    obj->GetLocalTransform().setScale(parser.ParseVector3());
    parser.Expect(',');

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
            parser.ParseString(); parser.Expect(':');
            bool enabled = parser.ParseBool();
            parser.Expect(',');

            parser.ParseString(); parser.Expect(':');
            std::string mesh = parser.ParseString();
            parser.Expect(',');

            parser.ParseString(); parser.Expect(':');
            std::string texture = parser.ParseString();
            parser.Expect(',');

            parser.ParseString(); parser.Expect(':');
            std::string shader = parser.ParseString();

            auto& comp = obj->AddComponent<Apex::Rendering::MeshRenderer>();
            comp.SetEnabled(enabled);

            auto a = GetMeshHandle(mesh);
            auto b = GetTextureHandle(texture);
            auto c = GetShaderHandle(shader);
            comp.SetMesh(a);
            comp.SetTexture(b);
            comp.SetShader(c);
        }
        else if (compName == "RigidBody")
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
        
        else if (compName == "MeshCollider")
        {
            parser.ParseString(); parser.Expect(':');
            bool enabled = parser.ParseBool();
            parser.Expect(',');

            parser.ParseString(); parser.Expect(':');
            std::string meshPath = parser.ParseString();
            parser.Expect(',');

            parser.ParseString(); parser.Expect(':');
            std::string type = parser.ParseString();
            parser.Expect(',');

            parser.ParseString(); parser.Expect(':');
            bool isTrigger = parser.ParseBool();
            parser.Expect(',');

            parser.ParseString(); parser.Expect(':');

            if (parser.Match("null")) 
            {
                auto& comp = obj->AddComponent<Apex::Physic::MeshCollider>(GetMeshHandle(meshPath), StringToMeshColliderType(type), isTrigger);
                comp.SetEnabled(enabled);
            }
            else 
            {
                auto& comp = obj->AddComponent<Apex::Physic::MeshCollider>(GetMeshHandle(meshPath), StringToMeshColliderType(type), isTrigger);
                comp.SetEnabled(enabled);
            }
        }

        else if (compName == "BoxCollider")
        {
            parser.ParseString(); parser.Expect(':');
            bool enabled = parser.ParseBool();
            parser.Expect(',');

            parser.ParseString(); parser.Expect(':'), parser.Expect('[');
            float xExtent = parser.ParseFloat();
            parser.Expect(',');
            float yExtent = parser.ParseFloat();
            parser.Expect(',');
            float zExtent = parser.ParseFloat();
            parser.Expect(']');
            parser.Expect(',');

            parser.ParseString(); parser.Expect(':');
            bool isTrigger = parser.ParseBool();
            parser.Expect(',');

            parser.ParseString(); parser.Expect(':');

            if (parser.Match("null"))
            {
                auto& comp = obj->AddComponent<Apex::Physic::BoxCollider>(LibMath::Vector3{ xExtent, yExtent, zExtent }, isTrigger);
                comp.SetEnabled(enabled);
            }
            else
            {
                auto& comp = obj->AddComponent<Apex::Physic::BoxCollider>(LibMath::Vector3{ xExtent, yExtent, zExtent }, isTrigger);
                comp.SetEnabled(enabled);
            }
        }

        parser.Expect('}'); // component end

        if (parser.Peek(','))
            parser.Expect(',');
    }

    parser.Expect('}'); // "components" category end
}

Apex::Physic::BodyType SceneLoader::StringToBodyType(const std::string& str)
{
    if (str == "Static")   return Apex::Physic::BodyType::Static;
    if (str == "Dynamic")  return Apex::Physic::BodyType::Dynamic;
    if (str == "Kinematic") return Apex::Physic::BodyType::Kinematic;

    throw std::runtime_error("Invalid BodyType string: " + str);
}

Apex::Physic::MeshColliderType SceneLoader::StringToMeshColliderType(const std::string& str)
{
    if (str == "Convex") return Apex::Physic::MeshColliderType::Convex;
    if (str == "Triangle")   return Apex::Physic::MeshColliderType::Triangle;

    throw std::runtime_error("Invalid MeshColliderType string: " + str);
}

Apex::Resources::ResourceHandle<Mesh> SceneLoader::GetMeshHandle(const std::string& path)
{
    auto handle = m_resourceManager.Get<Mesh>(path);
    if (!handle.IsValid())
    {
        handle = m_resourceManager.CreateAsync<Mesh>(path);
    }
    return handle;
}

Apex::Resources::ResourceHandle<Texture> Apex::Serialization::SceneLoader::GetTextureHandle(const std::string& path)
{
    auto handle = m_resourceManager.Get<Texture>(path);
    if (!handle.IsValid())
    {
        handle = m_resourceManager.CreateAsync<Texture>(path);
    }
    return handle;
}

Apex::Resources::ResourceHandle<Shader> Apex::Serialization::SceneLoader::GetShaderHandle(const std::string& path)
{
    auto handle = m_resourceManager.Get<Shader>(path);
    if (!handle.IsValid())
    {
        handle = m_resourceManager.CreateAsync<Shader>(path);
    }
    return handle;
}
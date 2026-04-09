#include "SceneLoader.h"
#include "Object.h"

#include "MeshRenderer.h"
#include "Rigidbody.h"
#include "BoxCollider.h"
#include "MeshCollider.h"
#include "ScriptComponent.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cassert>

using namespace Apex::Serialization;
using namespace Apex::Physic;

void SceneLoader::LoadScene(Apex::Rendering::Scene& scene, const std::string& path)
{
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

        if (category == "materials")
        {
            ParseMaterials(parser);
        }
        else if (category == "objects")
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

    for (auto& [childId, parentId] : parentLinks)
    {
        auto child = objectsById[childId];
        auto parent = objectsById[parentId];

        if (child && parent)
        {
            child->GetSceneNodeRaw()->SetParent(parent->GetSceneNodeRaw());
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

    objectsById[id] = obj;
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
        parentLinks.emplace_back(id, parentId);
    }

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

            PhysMatHandle material;

            if (!parser.Match("null"))
            {
                size_t id = parser.ParseInt();
                auto it = m_materials.find(id);
                assert(it != m_materials.end() && "Material id not found");

                material = it->second;
            }

            auto& comp = obj->AddComponent<Apex::Physic::MeshCollider>(GetMeshHandle(meshPath), StringToMeshColliderType(type), isTrigger, material);
            comp.SetEnabled(enabled);
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

            PhysMatHandle material;

            if (!parser.Match("null"))
            {
                size_t id = parser.ParseInt();
                auto it = m_materials.find(id);
                assert(it != m_materials.end() && "Material id not found");

                material = it->second;
            }

            auto& comp = obj->AddComponent<Apex::Physic::BoxCollider>(LibMath::Vector3{ xExtent, yExtent, zExtent }, isTrigger, material);
            comp.SetEnabled(enabled);
        }

        else if (compName == "ScriptComponent")
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
                    std::string varName = parser.ParseString();
                    parser.Expect(':');

                    ExposedVar var = ParseScriptVar(parser);
                    var.name = varName;

                    comp.SetVariable(var.name, var);

                    if (parser.Peek(','))
                        parser.Expect(',');
                }

                parser.Expect('}');
            }
            }

        parser.Expect('}'); // component end

        if (parser.Peek(','))
            parser.Expect(',');
    }

    parser.Expect('}'); // "components" category end
}

Apex::ExposedVar SceneLoader::ParseScriptVar(SerialParser& parser)
{
    ExposedVar var;

    char c = parser.PeekChar();

    // STRING
    if (c == '"')
    {
        std::string str = parser.ParseString();

        char* buffer = new char[256];
        strncpy(buffer, str.c_str(), 255);
        buffer[255] = '\0';

        var.type = ExposedVar::String;
        var.data = buffer;
    }
    // BOOL
    else if (c == 't' || c == 'f')
    {
        bool* v = new bool(parser.ParseBool());
        var.type = ExposedVar::Bool;
        var.data = v;
    }
    // NUMBER (on force en float = plus safe avec Lua)
    else
    {
        float* v = new float(parser.ParseFloat());
        var.type = ExposedVar::Float;
        var.data = v;
    }

    return var;
}

void SceneLoader::ParseMaterials(SerialParser& parser)
{
    parser.Expect('[');

    while (!parser.Peek(']'))
    {
        parser.Expect('{');

        size_t id = 0;
        float staticFriction = 0.5f;
        float dynamicFriction = 0.5f;
        float bounciness = 0.f;

        while (!parser.Peek('}'))
        {
            std::string key = parser.ParseString();
            parser.Expect(':');

            if (key == "id")
                id = parser.ParseInt();

            else if (key == "static friction")
                staticFriction = parser.ParseFloat();

            else if (key == "dynamic friction")
                dynamicFriction = parser.ParseFloat();

            else if (key == "bounciness")
                bounciness = parser.ParseFloat();

            if (parser.Peek(','))
                parser.Expect(',');
        }

        parser.Expect('}');

        auto material = m_physics.CreateMaterial(
            std::to_string(id),
            staticFriction,
            dynamicFriction,
            bounciness
        );

        m_materials[id] = material;

        if (parser.Peek(','))
            parser.Expect(',');
    }

    parser.Expect(']');
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

Apex::Resources::ResourceHandle<Mesh> SceneLoader::GetMeshHandle(const std::string& path)
{
    auto handle = m_resourceManager.CreateAsync<Mesh>(path);
    return handle;
}

Apex::Resources::ResourceHandle<Texture> Apex::Serialization::SceneLoader::GetTextureHandle(const std::string& path)
{
    auto handle = m_resourceManager.CreateAsync<Texture>(path);
    return handle;
}

Apex::Resources::ResourceHandle<Shader> Apex::Serialization::SceneLoader::GetShaderHandle(const std::string& path)
{
    auto handle = m_resourceManager.CreateAsync<Shader>(path);
    return handle;
}
#ifndef CUBE_RENDERER
#define CUBE_RENDERER

#include "RenderPassInfo.h"
#include "RHI.h"

#include "LibMath/Matrix/Matrix4.h"
#include "LibMath/Vector/Vector3.h"

#include "ResourceHandle.h"
#include "ResourceManager.h"

#include "Object.h"

#include "Mesh.h"
#include "Model.h"
#include "Texture.h"
#include "Shader.h"
#include "Material.h"

namespace Apex::Rendering
{
    class MeshRenderer : public Apex::Component
    {
    public:
        MeshRenderer() = default;
        MeshRenderer(Apex::Resources::ResourceManager* resourceManager);
        MeshRenderer(MeshRenderer const&) = default;
        MeshRenderer& operator=(MeshRenderer const&) = default;

        void SetModel(Apex::Resources::ResourceHandle<Model> model);
        void SetMaterials(std::vector<Apex::Resources::ResourceHandle<Material>> materials);
        void SetCastShadow(bool enabled) { m_castShadow = enabled; }

        Resources::ResourceHandle<Model> GetModel() { return m_model; }
        std::vector<Resources::ResourceHandle<Material>> const& GetMaterials() { return m_materials; }

        const char* GetTypeName() const override { return "MeshRenderer"; };
        std::vector<ExposedVar> GetExposedVariables() override;

        std::unique_ptr<Apex::Component> Clone() override { return std::make_unique<MeshRenderer>(*this); }

        void ComputePass();
        void Render(RenderPassInfo passInfo);

        void Serialize(std::ostream& out) const override;

        void SetSkinned(bool enabled) { m_useSkinnedVAO = enabled; }
    private:
        Resources::ResourceHandle<Model> m_model;
        std::vector<Resources::ResourceHandle<Material>> m_materials;
        bool m_castShadow = true;
        bool m_useSkinnedVAO = false;
    };
}

#endif
#ifndef MATERIAL
#define MATERIAL

#include <unordered_map>
#include <variant>
#include <string>
#include <vector>

#include "ResourceHandle.h"
#include "ResourceManager.h"
#include "SerializationParser.h"

#include "ComputeShader.h"

#include "LibMath/Vector.h"

class Texture;
class Shader;

namespace Apex::Rendering
{
	class Material : public IResource
	{
	public:
		Material(std::string path) : IResource(path) {}

		void SetMaterialUniform(std::string name, int val) { m_uniforms[name] = val; }
		void SetMaterialUniform(std::string name, float val) { m_uniforms[name] = val; }
		void SetMaterialUniform(std::string name, LibMath::Vector3& val) { m_uniforms[name] = val; }
		void SetMaterialUniform(std::string name, LibMath::Vector4& val) { m_uniforms[name] = val; }

		void Bind(Resources::ResourceHandle<Shader> shader);
		void BindCompute();
		void StartCompute(uint32_t vertexCount);

		void LoadFromFile() override;
		void Save();

		void SetComputeShader(Resources::ResourceHandle<ComputeShader> shader) { m_computeShader = shader; }

		LibMath::Vector3 GetTint() const { return m_tint; }
		void SetTint(const LibMath::Vector3& t) { m_tint = t; }

		void AddEmptyTexture();

		Resources::ResourceHandle<ComputeShader> GetComputeShader() const { return m_computeShader; }
		std::vector<Resources::ResourceHandle<Texture>>& GetTextures() { return m_textures; }

	private:
		void WriteProperties(std::ofstream& file_out);

		void ParseProperties(Serialization::SerialParser& parser);
		void ParseTextures(Serialization::SerialParser& parser);

		using UniformValue = std::variant<int, float, LibMath::Vector3, LibMath::Vector4>;
		std::unordered_map<std::string, UniformValue> m_uniforms;

		static constexpr int MAX_TEXTURES = 5;
		std::vector<Resources::ResourceHandle<Texture>> m_textures;

		Resources::ResourceHandle<ComputeShader> m_computeShader = nullptr;
		LibMath::Vector3 m_tint{ 1.f, 1.f, 1.f };
	};
}


#endif

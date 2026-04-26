#include "Material.h"

#include "Log.h"

#include <fstream>
#include <sstream>

#include "Shader.h"
#include "Texture.h"

using namespace Apex::Rendering;
using namespace Apex::Resources;

void Material::Bind()
{
	m_shader->Use();

	for (auto& [name, value] : m_uniforms)
	{
		// call the overload of set uniform with the correct type of variant
		std::visit([&](auto&& v)
			{
				m_shader->SetUniform(name, v);
			}, value);
	}

	for (int i = 0; i < m_textures.size(); i++)
		if (m_textures[i].IsValid())
			m_textures[i]->Bind(i);

	if (m_textures.size() > 0)
	{
		m_shader->SetUniform("uUseTexture", 1);
	}
}

void Material::Save()
{
	std::ofstream file(m_path);
	if (!file)
	{
		LOG_ERROR_CAT("Resource", "Failed to open file : \"{}\"", m_path);
		return;
	}

	const bool hasShader = m_shader.IsValid();
	const bool hasUniforms = m_uniforms.size() > 0;
	const bool hasTextures = m_textures.size() > 0;

	file << "{\n";

	bool needsComma = false;

	if (hasShader)
	{
		file << "\t\"shader\": \"" << m_shader->GetPath() << "\"";
		needsComma = true;
	}

	if (hasUniforms)
	{
		if (needsComma) file << ",\n";
		file << "\t\"properties\": {";
		WriteProperties(file);
		file << "\n\t}";
		needsComma = true;
	}

	if (hasTextures)
	{
		if (needsComma) file << ",\n";
		file << "\t\"textures\": [";

		bool firstTex = true;
		for (ResourceHandle<Texture>& tex : m_textures)
		{
			if (tex.IsValid())
			{
				if (!firstTex) file << ",";
				file << "\n\t\t\"" << tex->GetPath() << "\"";
				firstTex = false;
			}
		}

		file << "\n\t]";
	}

	file << "\n}\n";

	LOG_INFO_CAT("Resource", "\"{}\" saved successfully", m_path);
}

void Material::AddEmptyTexture()
{
	if (m_textures.size() >= MAX_TEXTURES)
	{
		LOG_WARNING_CAT("Resource", "Cannot have more than {} textures in a material", MAX_TEXTURES);
		return;
	}

	m_textures.push_back(m_rm->CreateAsync<Texture>("Assets/Textures/Blank.png"));
}

void Material::WriteProperties(std::ofstream& file_out)
{
	bool first = true;
	for (auto& [name, value] : m_uniforms)
	{
		if (!first) file_out << ",\n";
		first = false;

		file_out << "\t\t\"" << name << "\": { ";

		std::visit([&](auto&& v)
			{
				using T = std::decay_t<decltype(v)>;

				if constexpr (std::is_same_v<T, float>)
					file_out << "\"float\": " << v;
				else if constexpr (std::is_same_v<T, int>)
					file_out << "\"int\": " << v;
				else if constexpr (std::is_same_v<T, LibMath::Vector3>)
					file_out << "\"vec3\": [" << v[0] << ", " << v[1] << ", " << v[2] << "]";
				else if constexpr (std::is_same_v<T, LibMath::Vector4>)
					file_out << "\"vec4\": [" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "]";
			}, value);

		file_out << " }";
	}
}

void Material::LoadFromFile()
{
	LOG_INFO_CAT("Resource", "Start loading \"{}\"", m_path);
	std::ifstream file(m_path);
	if (!file)
	{
		LOG_ERROR_CAT("Resource", "Failed to open file : \"{}\"", m_path);
		return;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string text = buffer.str();

	Serialization::SerialParser parser(text);

	parser.Expect('{');

	while (!parser.Peek('}'))
	{
		std::string category = parser.ParseString();
		parser.Expect(':');

		if (category == "shader")
		{
			m_shader = m_rm->CreateAsync<Shader>(parser.ParseString());
		}
		else if (category == "properties")
		{
			ParseProperties(parser);
		}
		else if (category == "textures")
		{
			ParseTextures(parser);
		}

		if (parser.Peek(','))
			parser.Expect(',');
	}
	parser.Expect('}');
}

void Material::ParseProperties(Serialization::SerialParser& parser)
{
	parser.Expect('{');

	while (!parser.Peek('}'))
	{
		std::string name = parser.ParseString();
		parser.Expect(':');

		parser.Expect('{');

		std::string type = parser.ParseString();
		parser.Expect(':');

		if (type == "float")
		{
			m_uniforms[name] = parser.ParseFloat();
		}
		else if (type == "int")
		{
			m_uniforms[name] = parser.ParseInt();
		}
		else if (type == "vec3")
		{
			m_uniforms[name] = parser.ParseVector3();
		}
		else if (type == "vec4")
		{
			m_uniforms[name] = parser.ParseVector4();
		}

		parser.Expect('}');


		if (parser.Peek(','))
			parser.Expect(',');
	}

	parser.Expect('}');
}

void Material::ParseTextures(Serialization::SerialParser& parser)
{
	parser.Expect('[');

	while (!parser.Peek(']'))
	{
		m_textures.push_back(m_rm->CreateAsync<Texture>(parser.ParseString()));

		if (parser.Peek(','))
			parser.Expect(',');
	}

	parser.Expect(']');
}

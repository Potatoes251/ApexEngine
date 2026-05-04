#include "EditorIcon.h"

#include <glad/glad.h>

extern "C"
{
    unsigned char* stbi_load(const char* filename,
        int* x, int* y,
        int* channels_in_file,
        int  desired_channels);
    void stbi_image_free(void* ptr);
}

namespace Apex::Editor
{
    uint32_t EditorIcon::Get(const std::string& path)
    {
        auto it = m_cache.find(path);
        if (it != m_cache.end())
            return it->second;

        uint32_t id = Load(path);
        m_cache[path] = id;   // cache even on failure (id == 0) to avoid retry spam
        return id;
    }

    uint32_t EditorIcon::Load(const std::string& path)
    {
        int width = 0, height = 0, channel = 0;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channel, 4);
        if (!data) return 0;

        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data);
        return static_cast<uint32_t>(tex);
    }

    void EditorIcon::Shutdown()
    {
        for (auto& [path, id] : m_cache)
        {
            if (id != 0)
            {
                GLuint tex = id;
                glDeleteTextures(1, &tex);
            }
        }
        m_cache.clear();
    }
} 
#ifndef IRESOURCE
#define IRESOURCE

enum class ResourceState
{
    Unloaded,
    Loading,
    Loaded,     // CPU ready
    Uploaded,   // GPU ready
    Failed      // couldn't be loaded
};

#include <atomic>

#include "RHI.h"

namespace Apex::Resources { class ResourceManager; }

class IResource
{
public:
    IResource() = default;
    IResource(const std::string& path) : m_path(path) {}
    IResource(IResource&& other) noexcept : m_path(other.m_path), m_rhi(other.m_rhi), m_rm(other.m_rm)
    {
        m_state.store(other.m_state);
        other.m_state = ResourceState::Unloaded;
    }
    IResource& operator=(IResource&& other) noexcept   
    {
        m_path = other.m_path;
        m_rhi = other.m_rhi;
        m_rm = other.m_rm;
        m_state.store(other.m_state);
        other.m_state = ResourceState::Unloaded;
    }
    virtual ~IResource() = default;
    // load on the CPU
    virtual void LoadFromFile() { m_state = ResourceState::Loaded; }
    // Perform GPU part
    virtual void UploadToGpu() { m_state = ResourceState::Uploaded; }

    const std::string& GetPath() const { return m_path; }

    ResourceState   GetState() const { return m_state; }
    void            SetState(ResourceState state) { m_state = state; }
    void            SetRhi(Apex::Rendering::IRHI* rhi) { m_rhi = rhi; }
    void            SetResourceManager(Apex::Resources::ResourceManager* rm) { m_rm = rm; }

protected:
    std::string m_path;
    Apex::Rendering::IRHI* m_rhi = nullptr;
    Apex::Resources::ResourceManager* m_rm = nullptr;
private:
    std::atomic<ResourceState>   m_state = ResourceState::Unloaded;
};

#endif // !IRESOURCE
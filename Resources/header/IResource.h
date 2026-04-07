#ifndef IRESOURCE
#define IRESOURCE

#include "RHI.h"

enum class ResourceState
{
    Unloaded,
    Loading,
    Loaded,     // CPU ready
    Uploaded,   // GPU ready
    Failed      // couldn't be loaded
};

class IResource
{
public:
    IResource() = default;
    IResource(const std::string& path) : m_path(path) {}
    virtual ~IResource() = default;
    // load on the CPU
    virtual void LoadFromFile() {}
    // Perform GPU part
    virtual void UploadToGpu() {}

    const std::string& GetPath() const { return m_path; }

    ResourceState   GetState() const { return m_state; }
    void            SetState(ResourceState state) { m_state = state; }
    void            SetRhi(Apex::Rendering::IRHI* rhi) { m_rhi = rhi; }

protected:
    std::string m_path;
    Apex::Rendering::IRHI* m_rhi = nullptr;
private:
    ResourceState   m_state = ResourceState::Unloaded;
};

#endif // !IRESOURCE
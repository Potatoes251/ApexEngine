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
    virtual ~IResource() = default;
    // load on the CPU
    virtual void LoadFromFile() {}
    // Perform GPU part
    virtual void UploadToGpu() {}

    ResourceState   GetState() const { return m_state; }
    void            SetState(ResourceState state) { m_state = state; }
    void            SetRhi(Apex::Rendering::IRHI* rhi) { m_rhi = rhi; }

protected:
    Apex::Rendering::IRHI* m_rhi = nullptr;
    ResourceState   m_state = ResourceState::Unloaded;
};

#endif // !IRESOURCE
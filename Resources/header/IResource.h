#ifndef IRESOURCE
#define IRESOURCE

enum class ResourceState
{
    Unloaded,
    Loading,
    Loaded,     // CPU ready
    Uploaded    // GPU ready
};

class IResource
{
public:
    virtual ~IResource() = default;
    // load on the CPU
    virtual void LoadFromFile() {}
    // Perform GPU part
    virtual void UploadToGpu() {}

    ResourceState   getState() const { return m_state; }
    void            setState(ResourceState state) { m_state = state; }

protected:
    ResourceState   m_state = ResourceState::Unloaded;
};

#endif // !IRESOURCE
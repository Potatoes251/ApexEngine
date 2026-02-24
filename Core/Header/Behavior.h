#ifndef BEHAVIOR
#define BEHAVIOR

class Behavior 
{
public:
    virtual void Start() {}
    virtual void Update(float dt) {}
    virtual ~Behavior() = default;
};

#endif // BEHAVIOR
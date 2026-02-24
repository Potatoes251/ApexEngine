#ifndef BEHAVIOUR
#define BEHAVIOUR

namespace Apex::Scripting
{
    class Behaviour 
    {
    public:
        virtual void Start() {}
        virtual void Update(float dt) {}
        virtual ~Behaviour() = default;
    };
}

#endif // BEHAVIOUR
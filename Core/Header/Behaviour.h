#ifndef BEHAVIOUR
#define BEHAVIOUR

namespace Apex::Scripting
{
    class Behaviour 
    {
    public:
        Behaviour() = default;

        virtual ~Behaviour() = default;

        Behaviour(const Behaviour&) = delete;
        Behaviour& operator=(const Behaviour&) = delete;

        virtual void Start() {}
        virtual void Update(float /*dt*/) {}
    };
}

#endif // BEHAVIOUR
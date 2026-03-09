#ifndef LUA_BEHAVIOUR
#define LUA_BEHAVIOUR

#include "Behaviour.h"
#include "LuaManager.h"

#include <lua.hpp>

namespace Apex::Scripting
{
    class LuaBehaviour : public Behaviour
    {
    public:
        LuaBehaviour(LuaManager* manager, int scriptInstance)
            : m_manager(manager), m_instanceRef(scriptInstance) {}
        ~LuaBehaviour();
         
        LuaBehaviour(const LuaBehaviour&) = delete;
        LuaBehaviour& operator=(const LuaBehaviour&) = delete;

        void Start() override;
        void Update(float dt) override;

    private:
        LuaManager* m_manager;
        int m_instanceRef;
    };
}

#endif // LUA_BEHAVIOUR
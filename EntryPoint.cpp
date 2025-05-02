#include <Windows.h>

#include "RBX/RBX.hpp"
#include "Miscellaneous/ThreadPool.hpp"
#include "Environment/Environment.hpp"
#include "RBX/Scheduler/Scheduler.hpp"
#include "Miscellaneous/Update/Engine.hpp"
#include "RBX/Execution/Execution.hpp"
#include "Miscellaneous/Manager.hpp"
#include <httplib/httplib.hpp>

extern "C" __declspec(dllexport) LRESULT InitHook(int Code, WPARAM WParam, LPARAM LParam) { return CallNextHookEx(nullptr, Code, WParam, LParam); }

void MainThread(HMODULE Module) {
    RBX::Scheduler->Initialize();

    auto ScriptContext = RBX::Scheduler->GetScriptContext();
    auto GlobalState = ScriptContext + Update::ScriptContext::GlobalState;

    uintptr_t StateIndex[] = { 0 };
    uintptr_t ActorIndex[] = { 0, 0 };

    lua_State* L = RBX::DecryptState(RBX::GetGlobalStateForInstance(GlobalState, StateIndex, ActorIndex) + Update::ScriptContext::DecryptState);

    lua_State* ExploitThread = Execution->NewThread(L);
    luaL_sandboxthread(ExploitThread);

    Manager->SetLuaState(ExploitThread);
    RBX::Scheduler->HookJob("Heartbeat");

    Environment->Initialize(Manager->GetLuaState());
    while (true) {} // Stop thread from ending, dont remove it, if the thread ends, Roblox will crash (nigga butter chicken)
}

bool __stdcall DllMain(HMODULE Module, uintptr_t Reason, void*) {
    if (Reason == DLL_PROCESS_ATTACH)
        ThreadPool->Run(MainThread, Module);

    return true;
}
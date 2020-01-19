#include "clw.hpp"
#include <string>


Lua::strict_mode_t::strict_mode_t(const Lua::strict_mode_t& strict): instance(strict.instance) {}
Lua::strict_mode_t::strict_mode_t(Lua& instance): instance(instance) {}

// TODO add the expected and current types to error message
#define _wrap(expr, T) if(lua_type(instance.L, -1) == LUA_T ## T) {\
    v = expr;\
    lua_pop(instance.L, 1);\
} else {\
    instance << "Lua-C++ Type conflict";\
    instance.error = 1;\
    throw "Lua-C++ Type conflict";\
}
const Lua::strict_mode_t& Lua::strict_mode_t::operator>>(void*& v)          const { _wrap(null, NIL) return *this; }
const Lua::strict_mode_t& Lua::strict_mode_t::operator>>(bool& v)           const { _wrap(lua_toboolean(instance.L, -1), BOOLEAN) return *this; }
const Lua::strict_mode_t& Lua::strict_mode_t::operator>>(int& v)            const { _wrap(lua_tointeger(instance.L, -1), NUMBER) return *this; }
const Lua::strict_mode_t& Lua::strict_mode_t::operator>>(float& v)          const { _wrap(lua_tonumber(instance.L, -1), NUMBER) return *this; }
const Lua::strict_mode_t& Lua::strict_mode_t::operator>>(double& v)         const { _wrap(lua_tonumber(instance.L, -1), NUMBER) return *this; }
const Lua::strict_mode_t& Lua::strict_mode_t::operator>>(std::string& v)    const { _wrap(lua_tostring(instance.L, -1), STRING) return *this; }
const Lua::strict_mode_t& Lua::strict_mode_t::operator>>(const char*& v)    const { _wrap(lua_tostring(instance.L, -1), STRING) return *this; }
const Lua::strict_mode_t& Lua::strict_mode_t::operator>>(Lua::CFunction& v) const { _wrap(lua_tocfunction(instance.L, -1), FUNCTION) return *this; }
#undef _wrap
    

Lua& Lua::call_function(int argc, int retc) { error = lua_pcall(L, argc, retc, 0); return  *this; }


Lua::variable_t::variable_t(const Lua::variable_t& var):
instance(var.instance),
variable_name(var.variable_name) {}

Lua::variable_t::variable_t(Lua& instance, const char * variable_name):
instance(instance),
variable_name(variable_name) {}

Lua::variable_t::variable_t(Lua& instance, std::string& variable_name):
instance(instance),
variable_name(variable_name.c_str()) {}

void Lua::variable_t::operator<<(null_t)                const { instance << null; lua_setglobal(instance.L, variable_name); }
void Lua::variable_t::operator<<(bool v)                const { instance << v; lua_setglobal(instance.L, variable_name); }
void Lua::variable_t::operator<<(int v)                 const { instance << v; lua_setglobal(instance.L, variable_name); }
void Lua::variable_t::operator<<(float v)               const { instance << v; lua_setglobal(instance.L, variable_name); }
void Lua::variable_t::operator<<(double v)              const { instance << v; lua_setglobal(instance.L, variable_name); }
void Lua::variable_t::operator<<(const std::string& v)  const { instance << v; lua_setglobal(instance.L, variable_name); }
void Lua::variable_t::operator<<(char*& v)              const { instance << v; lua_setglobal(instance.L, variable_name); }
void Lua::variable_t::operator<<(const char* v)         const { instance << v; lua_setglobal(instance.L, variable_name); }
void Lua::variable_t::operator<<(Lua::CFunction v)      const { instance << v; lua_setglobal(instance.L, variable_name); }

void Lua::variable_t::operator>>(void* v)           const { lua_getglobal(instance.L, variable_name); instance >> v; }
void Lua::variable_t::operator>>(bool& v)           const { lua_getglobal(instance.L, variable_name); instance >> v; }
void Lua::variable_t::operator>>(int& v)            const { lua_getglobal(instance.L, variable_name); instance >> v; }
void Lua::variable_t::operator>>(float& v)          const { lua_getglobal(instance.L, variable_name); instance >> v; }
void Lua::variable_t::operator>>(double& v)         const { lua_getglobal(instance.L, variable_name); instance >> v; }
void Lua::variable_t::operator>>(std::string& v)    const { lua_getglobal(instance.L, variable_name); instance >> v; }
void Lua::variable_t::operator>>(const char*& v)    const { lua_getglobal(instance.L, variable_name); instance >> v; }
void Lua::variable_t::operator>>(Lua::CFunction& v) const { lua_getglobal(instance.L, variable_name); instance >> v; }
void Lua::variable_t::operator>>(Lua::Function& v)  const { lua_getglobal(instance.L, variable_name); v.instance = &instance; }


const int Lua::run(const char* dotlua) {
    int err = luaL_dofile(L, dotlua);
    if(err) {
        error = err;
        throw "specified lua script cannot be executed";
    } else {
        return err;
    }
}


Lua::Function::function_style_call::function_style_call(Lua::Function& fn): fn(fn) {}
Lua& Lua::Function::function_style_call::operator>(int retc) const {
    fn.private_call(retc); // low overhead
    // fn << Lua::Function::call(retc);
    return *fn.instance;
}

Lua::Function::call::call(): retc(0) {}
Lua::Function::call::call(int retc): retc(retc) {}

Lua::Function::Function(): instance(null), argc(0) {}
Lua::Function::Function(const Lua::Function& fn): instance(fn.instance), argc(0) {}
Lua::Function::Function(Lua& instance): instance(&instance), argc(0) {}
Lua::Function::Function(const Lua::variable_t& v): instance(&(v.instance)), argc(0) {}

#define check if(!instance) { throw "Lua::Function is null"; }
Lua& Lua::Function::operator<<(const Lua::Function::call& c) {
    check;
    instance->call_function(argc, c.retc);
    argc = 0;
    return *instance;
}

void Lua::Function::private_call(int retc)  {
    check;
    instance->call_function(argc, retc);
    argc = 0;
}

Lua::Function& Lua::Function::operator<<(null_t)               { check; *instance << null; ++ argc; return *this; }
Lua::Function& Lua::Function::operator<<(bool v)               { check; *instance << v; ++ argc; return *this; }
Lua::Function& Lua::Function::operator<<(int v)                { check; *instance << v; ++ argc; return *this; }
Lua::Function& Lua::Function::operator<<(float v)              { check; *instance << v; ++ argc; return *this; }
Lua::Function& Lua::Function::operator<<(double v)             { check; *instance << v; ++ argc; return *this; }
Lua::Function& Lua::Function::operator<<(const std::string& v) { check; *instance << v; ++ argc; return *this; }
Lua::Function& Lua::Function::operator<<(char*& v)             { check; *instance << v; ++ argc; return *this; }
Lua::Function& Lua::Function::operator<<(const char* v)        { check; *instance << v; ++ argc; return *this; }
Lua::Function& Lua::Function::operator<<(Lua::CFunction& v)    { check; *instance << v; ++ argc; return *this; }
#undef check

Lua::Function::function_style_call Lua::Function::operator()(void) { return Lua::Function::function_style_call(*this); }

Lua::Lua(): L(luaL_newstate()), owner(true) {
    luaL_openlibs(L);
}
Lua::Lua(const Lua& lua): L(lua.L), owner(false) {}
Lua::Lua(Lua::vm L): L(L), owner(false) {
    luaL_openlibs(L);
}
Lua::Lua(const char * dotlua): Lua() {
    run(dotlua);
}
Lua::~Lua() {
    if(owner) {
        lua_close(L);
    }
}

bool Lua::operator()(const char* dotlua) { return !run(dotlua); }

Lua& Lua::operator<<(null_t)                { lua_pushnil(L); return *this; }
Lua& Lua::operator<<(bool v)                { lua_pushboolean(L, v); return *this; }
Lua& Lua::operator<<(int v)                 { lua_pushinteger(L, v); return *this; }
Lua& Lua::operator<<(float v)               { lua_pushnumber(L, v); return *this; }
Lua& Lua::operator<<(double v)              { lua_pushnumber(L, v); return *this; }
Lua& Lua::operator<<(const std::string& v)  { lua_pushstring(L, v.c_str()); return *this; }
Lua& Lua::operator<<(char*& v)              { lua_pushstring(L, v); return *this; }
Lua& Lua::operator<<(const char* v)         { lua_pushstring(L, v); return *this; }
Lua& Lua::operator<<(Lua::CFunction& v)     { lua_pushcfunction(L, v); return *this; }

// strict mode checks the type
const Lua::strict_mode_t Lua::operator>>(const Lua::strict_t) { return Lua::strict_mode_t(*this); }

Lua& Lua::operator>>(void* v)            { v = null; lua_pop(L, 1); return *this; }
Lua& Lua::operator>>(bool& v)            { v = lua_toboolean(L, -1); lua_pop(L, 1); return *this; }
Lua& Lua::operator>>(int& v)             { v = lua_tointeger(L, -1); lua_pop(L, 1); return *this; }
Lua& Lua::operator>>(float& v)           { v = lua_tonumber(L, -1); lua_pop(L, 1); return *this; }
Lua& Lua::operator>>(double& v)          { v = lua_tonumber(L, -1); lua_pop(L, 1); return *this; }
Lua& Lua::operator>>(std::string& v)     { v = lua_tostring(L, -1); lua_pop(L, 1); return *this; }
Lua& Lua::operator>>(const char*& v)     { v = lua_tostring(L, -1); lua_pop(L, 1); return *this; }
Lua& Lua::operator>>(int (*&v)(Lua::vm)) { v = lua_tocfunction(L, -1); lua_pop(L, 1); return *this; }

Lua::operator int() { return error; }
Lua::operator std::string() { std::string err(lua_tostring(L, -1)); lua_pop(L, 1); return err; }


const Lua::variable_info_t Lua::variable(const char* name) {
    return Lua::variable_info_t({name});
}
Lua::variable_t Lua::operator[](std::string& name) { return Lua::variable_t(*this, name); }
Lua::variable_t Lua::operator[](const char* name) { return Lua::variable_t(*this, name); }

Lua::strict_t const Lua::strict;

#ifndef CLW_HPP
#define CLW_HPP
#include "include_lua.hpp"
#include <string>
#define null nullptr
typedef decltype(null) null_t;

// DONE :
// stack push pop
// LuaFn in CPP
// CPPFn in Lua
// global variable set get

// DOING :
// strict mode

// TODO :
// table
// libs

class Lua {
public:
    typedef lua_State* vm;
    typedef int(*CFunction)(Lua::vm);
    
    class types {
        Lua& instance;
        public:
        types(Lua&);
        static unsigned char nil;
        static unsigned char boolean;
        static unsigned char number;
        static unsigned char string;
        static unsigned char function;
        void assert(unsigned char t) const;
    };
    const types& type;
    
private:
    vm L;
    bool owner;
    int error;

    typedef struct variable_info_t {
        const char * name;
    } variable_info_t;

    class strict_mode_t {
        Lua& instance;
        public:
        strict_mode_t(const strict_mode_t& strict);
        strict_mode_t(Lua& instance);
        const strict_mode_t& operator>>(void*&) const;
        const strict_mode_t& operator>>(bool&) const;
        const strict_mode_t& operator>>(int&) const;
        const strict_mode_t& operator>>(float&) const;
        const strict_mode_t& operator>>(double&) const;
        const strict_mode_t& operator>>(std::string&) const;
        const strict_mode_t& operator>>(const char*&) const;
        const strict_mode_t& operator>>(CFunction&) const;
    };

public:
    typedef struct strict_t {} strict_t;
    static const strict_t strict;
    class Function;
private:
    Lua& call_function(int argc, int retc);
    
    class variable_t {
        Lua& instance;
        const char* variable_name;

        public:
        variable_t(const variable_t&);
        variable_t(Lua&, const char * variable_name);
        variable_t(Lua&, std::string& variable_name);
        
        void operator<<(null_t) const;
        void operator<<(bool) const;
        void operator<<(int) const;
        void operator<<(float) const;
        void operator<<(double) const;
        void operator<<(const std::string&) const;
        void operator<<(char*&) const;
        void operator<<(const char*) const;
        void operator<<(CFunction) const;

        void operator>>(void*) const;
        void operator>>(bool&) const;
        void operator>>(int&) const;
        void operator>>(float&) const;
        void operator>>(double&) const;
        void operator>>(std::string&) const;
        void operator>>(const char*&) const;
        void operator>>(CFunction&) const;
        void operator>>(Function&) const;
        
        friend class Function;
    };

    int run(std::string& dotlua);
    int run(const char* dotlua);

public:
    class Function {
        Lua *instance;
        int argc;

        private:
        class function_style_call {
            Function &fn;
            function_style_call(Function&);
            public:
            Lua& operator>(int retc) const;
            friend class Function;
        };

        public:
        class call {
            int retc;
            public:
            call();
            call(int retc);
            friend class Function;
        };

        Function();
        Function(const Function&);
        Function(Lua&);
        Function(const variable_t&);

        Lua& operator<<(const call&);
        private:
        void private_call(int retc);

        public:
        Function& operator<<(null_t);
        Function& operator<<(bool);
        Function& operator<<(int);
        Function& operator<<(float);
        Function& operator<<(double);
        Function& operator<<(const std::string&);
        Function& operator<<(char*&);
        Function& operator<<(const char*);
        Function& operator<<(CFunction&);

        function_style_call operator()(void);
        template<typename T>
        const function_style_call operator()(T v) { return Lua::Function::function_style_call(*this << v); }
        template<typename T, typename ...Args>
        const function_style_call operator()(T v, Args... args) { return (*this << v)(args...); }

        friend class variable_t;
    };

public:
    Lua();
    Lua(const Lua&);
    Lua(vm);
    Lua(const char * dotlua);
    Lua(std::string& dotlua);
    ~Lua();

    bool operator()(std::string& dotlua);
    bool operator()(const char* dotlua);
    Lua& operator<<(null_t);
    Lua& operator<<(bool);
    Lua& operator<<(int);
    Lua& operator<<(float);
    Lua& operator<<(double);
    Lua& operator<<(const std::string&);
    Lua& operator<<(char*&);
    Lua& operator<<(const char*);
    Lua& operator<<(CFunction&);

    // strict mode checks the type
    const strict_mode_t operator>>(const strict_t);

    Lua& operator>>(void*);
    Lua& operator>>(bool&);
    Lua& operator>>(int&);
    Lua& operator>>(float&);
    Lua& operator>>(double&);
    Lua& operator>>(std::string&);
    Lua& operator>>(const char*&);
    Lua& operator>>(CFunction&);

    operator int();
    operator std::string();



    static const variable_info_t variable(const char* name);
    variable_t operator[](std::string& name);
    variable_t operator[](const char* name);
};

#endif

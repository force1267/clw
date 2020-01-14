# Classic Lua Wrapper

helps embeding lua to your project.


## add CLW :
1. install Lua and Lua source.
2. include real `lua/lua.hpp` from Lua source inside `include_lua.hpp`
3. copy `include_lua.hpp`, `clw.hpp` and `clw.cpp` to your project's directories
4. add the header (`clw.hpp`) to your code.
5. add the implementation (`clw.cpp`) to your build pipeline (maybe `make`).
6. link against Lua library. (google it)

## use CLW :

### `Lua` type

provides methods to interact with Lua
it uses C-Lua interface provided by Lua library.

#### `Lua::vm`

it keeps lua stack and state.
its just `typedef`ed `lua_State*`.

#### initialize new Lua state

```cpp
Lua lua;
```

#### new Lua state and run a .lua file

```cpp
Lua lua("script.lua");
```

#### copy Lua state

```cpp
Lua original("script.lua");
Lua copy(lua);
```

when `copy` dies `Lua::vm` inside it won't destruct. but `original` closes its `Lua::vm` at destruction.

### Executing a script

1. start a script at `Lua` initialization like :

```cpp
Lua lua("script.lua");
```

2. start after initialization. like :

```cpp
Lua lua;
lua("script.lua");
```

#### you can start more than one script

```cpp
Lua lua("script1.lua");
lua("script2.lua");
lua("script3.lua");
```
after runing each script their global scope will be available at `lua`

#### it throws error

```cpp
try {
    lua("plugin.lua");
} catch (const char * msg) {
    std::cerr << msg << "\n";
    return 1;
}
```

### C-Lua stack

data between C++ and Lua is passed trough a stack. functions pop stack values to use them as arguments and pushes return values to stack. (lua functions can return more than one value)

#### pop from stack

```cpp
int a;
string b;
lua >> a >> b;
```

using `>>` operator you can read data from Lua. but you need to know the types.

if types are important and no conversion is allowed. `Lua::strict` can be used. it throws error if types don't match

```cpp
int a;
string b;
try {
    lua >> Lua::strict >> a >> b;
} catch (const char * msg) {
    std::cerr << msg << "\n";
    return 1;
}
```

#### push to stack

```cpp
int a = 12;
lua << a << "hello from cpp";
```

#### push `null` for Lua's `nil`

Lua has `nil` to show not absence of value.
it can be pushed to stack using `null`. `null` is just renamed `nullptr`.

> pushing anytihng that is not supported and can be converted to `void*` puts a Lua `nil` on top of the stack

### Lua global variables

they can be accessed using `[]` operator.

```cpp
int a;
string b;
lua["LuaNumber"] >> a;
lua["LuaString"] << b;
```

use `<<` to set a lua varaiable and `>>` to get its value.

using this functionality Lua scripts can be used as fancy config files. like below:

> `config.lua`
```lua
-- modify these
MY_CONFIG_1 = 4

-- dont touch these
--------------------
sum = 0
for i = 1, MY_CONFIG_1, 1 do
    sum = sum + i
end
MY_CONFIG_2 = sum
```

> `program.cpp`
```cpp
Lua config("config.lua");
int MY_CONFIG_1;
int MY_CONFIG_2;
config["MY_CONFIG_1"] >> MY_CONFIG_1;
config["MY_CONFIG_2"] >> MY_CONFIG_2;
```

### lua functions in C++

lua treats its functions as first class citizens. it means functions can be stored in a variable or passed as arguments. they can be used like normal variables with `Lua::Function` class as their type.

> `plugin.lua`
```lua
function fn(a, b)
    return (a + b), (a - b)
end
```
> `program.cpp`
```cpp
Lua::Function fn;
lua["fn"] >> fn;
```

`fn` is a lua function and can be called now. (its a cpp functor)
one diffrence from cpp functions is that lua functions can return multiple values. so when calling from cpp number of returned values must be shown after a `>` operator.

```cpp
fn(5, 3) > 2;
```

`fn(5, 3)` gets the arguments and `(2)` gets number of returned values. function call happens after that. second call defaults to 0 return values
returned values will be stored on stack and can be retrived usign `lua >>`. but `>>` operator can be used right after the call.
in fact `fn(...)(int)` returns `Lua&`

```cpp
int r1, r2;
// (a + b), (a - b)
fn(5, 3) > 2 >> r2 >> r1;
```

> pop return-values from last one because it's a stack. Lua pushes return-values from left to right. so when poping them their order changes


#### it throws error

```cpp
try {
    fn(5, 3) > 2;
} catch (const char * msg) {
    std::cerr << msg << "\n";
    return 1;
}
```

#### calling lua functions using stack

there is another way to call a lua function which is more stack like.

```cpp
fn << 5 << 3 << Lua::Function::call(2);
```

it does the same thing. `Lua::Function::call` gets number of returned values and calls the function.
it returns `Lua&` too. so this is valid :

```cpp
int r1, r2;
fn << 5 << 3 <<
Lua::Function::call(2) >> r2 >> r1;
```

### C++ functions in Lua

C++ functions can be called from lua. using them you can expose a plugin API or provide more functionalities to Lua scripts

1. define a `int()(Lua::vm)` function.

> `int(*)(Lua::vm)` is also renamed to `Lua::CFunction`

```cpp
static int sum(Lua::vm vm) {
```

2. use `Lua::vm` to initialize a local `Lua`

```cpp
    Lua lua(vm);
```

3. pop arguments from stack. do something with it. then push results to stack.

```cpp
double a, b;
// read the arguments.
lua >> b >> a;
// do the function's job
int c = a + b;
// push return values to stack
lua << c;
```

> lua numbers are `double`s. don't panic. normally its precise enough. also it doesn't mean you can't use `int` or `float`. but when it goes to Lua it will become `double`

> pop arguments from last one because it's a stack. Lua pushes arguments from left to right. so when poping them their order changes

4. return the number of pushed results.

```cpp
    // we only pushed `c`
    return 1;
}
```

5. set it as a Lua global variable. (before runing the script that uses it)

```cpp
int main() {
    Lua lua();
    lua["sum"] << sum;
    lua("script.lua");
}
```

6. use it in Lua scripts

> `script.lua`
```lua
print(sum(2, 5))
```

the cpp code looks like this

```cpp
static int sum(Lua::vm vm) {
    Lua lua(vm);
    double a, b;
    lua >> b >> a; // a = 2, b = 5
    int c = a + b;
    lua << c;
    return 1
}

int main() {
    Lua lua();
    lua["sum"] = sum;
    lua("script.lua");
}
```

#### `Lua::CFunction`

`int()(Lua::vm)` is renamed to `Lua::CFunction` and is accessible :

```cpp
static int fn(Lua::vm) {}

int main() {
    Lua::CFunction cb;
    cb = fn
    Lua::CFunction& cb = fn;
}
```

it can be pushed to and poped from stack.

```cpp
Lua::CFunction fn;
lua >> fn;
lua << fn;
```

#### callbacks

functions can be pushed to stack as argument before calling a lua function. the lua function can use it to send back the result

> `plugin.lua`
```lua
function fn(a, b, cb)
    return cb((a + b), (a - b))
end
```
> `program.cpp`
```cpp
static int handle(Lua::vm vm) {
    Lua lua(vm);
    double a, b;
    lua >> b >> a;
    cout << a << " " << b << endl;
}
int main() {
    Lua::Function fn;
    lua["fn"] >> fn;
    fn(4, 2, handle) > 0;
}
```

<!-- it can be used to send (javascript-like) callback functions to Lua

```cpp
static int callback(Lua::vm vm) {
    Lua lua(vm);
    bool error;
    Lua::Table t;
    lua >> t >> error;
    if(!error) {
        std::cout << t;
    }
    return 0;
}

int main() {
    Lua::CFunction cb;
    cb = fn
    Lua::CFunction& cb = fn;
}
``` -->

## TODO

1. `Lua::Table` // use lua tables as cpp Iterators
2. `Lua::Library` // use cpp functions in lua as libs
3. `Lua::Pointer` // userdata
3. `Lua::JustPointer` // light userdata

## classic
because it uses classes ...
enjoy ;)
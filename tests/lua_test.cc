#include <minilua.h>
#include <sled/random.h>

TEST_SUITE("lua")
{
    TEST_CASE("Base Test")
    {
        lua_State *state = luaL_newstate();
        CHECK(state != nullptr);
        luaL_openlibs(state);
        REQUIRE_EQ(lua_gettop(state), 0);

        SUBCASE("add")
        {
            REQUIRE_EQ(LUA_OK, luaL_loadstring(state, "function add(a, b) return a+b end"));
            // load script
            REQUIRE_EQ(lua_pcall(state, 0, 0, 0), LUA_OK);
            // push function

            sled::Random rand(-1);
            for (int i = 0; i < 100; i++) {
                int a = rand.Rand(0, 100);
                int b = rand.Rand(0, 100);

                CHECK_EQ(LUA_TFUNCTION, lua_getglobal(state, "add"));
                CHECK(lua_isfunction(state, -1));

                REQUIRE_EQ(lua_gettop(state), 1);
                lua_pushnumber(state, a);
                lua_pushnumber(state, b);
                CHECK_EQ(lua_gettop(state), 3);

                CHECK_MESSAGE(lua_pcall(state, 2, 1, 0) == LUA_OK, "call add(a,b) error: ", lua_tostring(state, -1));
                CHECK(lua_isnumber(state, -1));
                CHECK_EQ(lua_tonumber(state, -1), a + b);
                lua_pop(state, 1);
                CHECK_EQ(lua_gettop(state), 0);
            }
        }

        SUBCASE("dump all function")
        {
            const char *dump_func
                = "    local seen={}\n"
                  "\n"
                  "    function dump(t,i)\n"
                  "        seen[t]=true\n"
                  "        local s={}\n"
                  "        local n=0\n"
                  "        for k in pairs(t) do\n"
                  "            n=n+1\n"
                  "            s[n]=k\n"
                  "        end\n"
                  "        table.sort(s)\n"
                  "        for k,v in ipairs(s) do\n"
                  "            -- print(i,v)\n"
                  "            v=t[v]\n"
                  "            if type(v)==\"table\" and not seen[v] then\n"
                  "                dump(v,i..\"\\t\")\n"
                  "            end\n"
                  "        end\n"
                  "    end\n"
                  "\n"
                  "dump(_G,\"\")";

            REQUIRE_EQ(LUA_OK, luaL_loadstring(state, dump_func));
            CHECK_EQ(lua_gettop(state), 1);
            CHECK_EQ(LUA_TFUNCTION, lua_type(state, -1));
            REQUIRE_EQ(LUA_OK, lua_pcall(state, 0, 0, 0));
        }
        lua_close(state);
    }
}

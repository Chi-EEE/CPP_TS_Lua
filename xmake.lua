add_rules("mode.debug", "mode.release")

add_requires("boost", {configs = {iostreams = true}})
add_requires("cpp-dump", "sol2", "magic_enum")

target("run_lua", function()
    set_kind("binary")
    set_languages("c++17")
    add_packages("boost", "cpp-dump", "sol2", "magic_enum")

    add_files("src/*.cpp")
    
    if is_plat("windows") then
        add_defines("NOMINMAX")
        add_cxflags("/utf-8")
    end

    add_configfiles("build/lua/(main.lua)", {onlycopy = true})
    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")
end)
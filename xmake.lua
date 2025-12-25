set_optimize("fastest")
set_warnings("all", "error")
add_rules("mode.debug", "mode.release")

target("interp")
    set_kind("binary")
    add_files("interp.c")

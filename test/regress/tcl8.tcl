proc assert condition {
    if {![uplevel 1 $condition]} {
        return -code error "assertion failed: $condition"
    }
}

run
assert {expr [exa /u/sub/done] == 1}

# The foreign procedure registered by the plugin must be bound again
# after the plugin is reloaded by restart
restart
assert {expr [exa /u/sub/done] == 0}

run
assert {expr [exa /u/sub/done] == 1}

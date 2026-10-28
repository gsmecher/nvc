proc assert condition {
    if {![uplevel 1 $condition]} {
        return -code error "assertion failed: $condition"
    }
}

assert {expr ![catch {run 1 ns}]}

# The failure stops the simulation which should be reported as an error
assert {catch {run} msg}
assert {string equal $msg "simulation stopped with status 1"}
assert {expr $now == 2000000}

# The simulation cannot be resumed
assert {catch {run 1 ns}}

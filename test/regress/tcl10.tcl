proc assert condition {
    if {![uplevel 1 $condition]} {
        return -code error "assertion failed: $condition"
    }
}

run
assert {expr [exa /x] == 1}

# Elaborating again replaces the current simulation
elaborate -generic N=5 tcl10
assert {expr $now == 0}
assert {expr [exa /x] == 0}

run
assert {expr [exa /x] == 5}

# The new design is kept across a restart
restart
run
assert {expr [exa /x] == 5}

assert {catch {elaborate}}
assert {catch {elaborate -foo tcl10}}
assert {catch {elaborate tcl10 -foo}}
assert {catch {elaborate -generic N tcl10}}

# Mistakes found before elaboration starts leave the simulation open
assert {catch {elaborate nosuch}}
assert {expr $now == 1000000}
assert {expr [exa /x] == 5}

# A failed elaboration leaves no simulation loaded
assert {catch {elaborate -generic N=0 tcl10}}
assert {catch {run}}

# Overrides only apply to a single elaboration
elaborate -generic UNUSED=1 tcl10
elaborate -generic UNUSED=2 tcl10

elaborate tcl10
run
assert {expr [exa /x] == 1}

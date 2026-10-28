proc assert condition {
    if {![uplevel 1 $condition]} {
        return -code error "assertion failed: $condition"
    }
}

set testdir [file dirname [file normalize [info script]]]

set sigs [find signals /*]
puts $sigs

assert {expr [exa /x] == -2147483648}

run

assert {expr [exa /x] == 2}

set root [vhpi::handle RootInst 0]

restart

puts [exa /x]
assert {expr [exa /x] == -2147483648}

assert {expr $now == 0}

# Handles from before the restart are no longer valid
assert {catch {vhpi::get_str Name $root}}
set root [vhpi::handle RootInst 0]
vhpi::assert {[vhpi::get_str Name $root] == "TCL2"}

run

assert {expr [exa /x] == 2}

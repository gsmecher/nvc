proc assert condition {
    if {![uplevel 1 $condition]} {
        return -code error "assertion failed: $condition"
    }
}

proc write_design {value} {
    set f [open tcl11.vhd w]
    puts $f "entity tcl11 is"
    puts $f "end entity;"
    puts $f "architecture test of tcl11 is"
    puts $f "    signal x : integer := 0;"
    puts $f "begin"
    puts $f "    x <= $value after 1 ns;"
    puts $f "end architecture;"
    close $f
}

# No design is loaded yet
assert {catch {run}}

write_design 1
analyse tcl11.vhd
assert {catch {run}}

elaborate tcl11
run
assert {expr [exa /x] == 1}

# Analysis closes the simulation which starts again from time zero with
# the existing elaborated design
write_design 2
analyse tcl11.vhd
assert {expr $now == 0}
assert {expr [exa /x] == 0}
run
assert {expr [exa /x] == 1}

elaborate tcl11
run
assert {expr [exa /x] == 2}

# Errors are reported and do not affect later analysis
set f [open bad.vhd w]
puts $f "entity bad is"
close $f

assert {catch {analyse bad.vhd}}
assert {catch {analyse}}
assert {catch {analyse -foo tcl11.vhd}}

# The American spelling is an alias
write_design 3
analyze tcl11.vhd

# A unit analysed with errors does not replace the previous version
write_design "3 +"
assert {catch {analyse tcl11.vhd}}

elaborate tcl11
run
assert {expr [exa /x] == 3}

# The -work option only applies to a single command
write_design 9
analyse -work other tcl11.vhd
elaborate -work other tcl11
run
assert {expr [exa /x] == 9}

elaborate tcl11
run
assert {expr [exa /x] == 3}

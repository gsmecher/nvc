# The simulation ends at restart before the plugin is loaded again so
# the plugin sees two complete simulations
run 10 ns
restart
run 10 ns

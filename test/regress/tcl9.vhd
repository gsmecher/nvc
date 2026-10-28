entity tcl9 is
end entity;

architecture test of tcl9 is
    signal clk : bit := '0';
begin
    clk <= not clk after 1 ns;
end architecture;

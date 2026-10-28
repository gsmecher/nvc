entity cmdline24 is
end entity;

architecture test of cmdline24 is
    signal clk : bit := '0';
begin
    clk <= not clk after 1 ns;
end architecture;

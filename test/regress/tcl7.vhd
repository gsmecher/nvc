entity tcl7 is
end entity;

architecture test of tcl7 is
begin

    process is
    begin
        wait for 2 ns;
        report "stop here" severity failure;
        wait;
    end process;

end architecture;

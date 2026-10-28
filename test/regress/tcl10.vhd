entity tcl10 is
    generic ( N : integer := 1 );
end entity;

architecture test of tcl10 is
    constant C : integer := 10 / N;
    signal x : integer := 0;
begin

    x <= N after 1 ns;

end architecture;

package tcl10_pkg is
end package;

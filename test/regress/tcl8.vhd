entity sub is
end entity;

architecture test of sub is
    signal done : integer := 0;

    procedure check (x : in integer);

    procedure check (x : in integer) is
    begin
        assert false report "foreign binding was not resolved" severity failure;
    end procedure;

    attribute foreign : string;
    attribute foreign of check : procedure is "VHPI vhpi23 check";
begin
    process is
    begin
        check(42);
        done <= 1;
        wait;
    end process;
end architecture;

-------------------------------------------------------------------------------

entity tcl8 is
end entity;

architecture test of tcl8 is
    -- Instantiate through a component rather than directly so the foreign
    -- subprogram is declared inside a component instance
    component sub is
    end component;
begin
    u: component sub;
end architecture;

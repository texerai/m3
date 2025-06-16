module SimSerial (
    input         clock,
    input         reset,
    input         serial_out_valid,
    output        serial_out_ready,
    input  [31:0] serial_out_bits,

    output        serial_in_valid,
    input         serial_in_ready,
    output [31:0] serial_in_bits,

    output [31:0] exit
);

// Empty module. Disabling HTIF related stuff.

endmodule

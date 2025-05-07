/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

module marionette_counter(
    input clock,
    input reset,
    output [63:0] count_o);
reg [63:0] q;
assign count_o = q;
always @(posedge clock) begin
    if (!reset) begin
        q <= q + 1;
    end else begin
        q <= 0;
    end
end
endmodule

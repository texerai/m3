harness.v
	SimDromajoCosimBlackBox
		.pc_0(dromajo_pc_0)
		
assign dbridge_io_trace_insns_0_iaddr = chiptop_trace_traces_0_insns_0_iaddr

ChipTop chiptop (
...
    .trace_traces_0_insns_0_iaddr(chiptop_trace_traces_0_insns_0_iaddr),
);

module ChipTop(
...
  output [39:0] trace_traces_0_insns_0_iaddr,
...
);
  assign trace_traces_0_insns_0_iaddr = system_traceIO_traces_0_insns_0_iaddr;
  
  DigitalTop system (
  ...
      .traceIO_traces_0_insns_0_iaddr(system_traceIO_traces_0_insns_0_iaddr),
  ...
  );
endmodule

module DigitalTop(
...
  output [39:0] traceIO_traces_0_insns_0_iaddr,
...
)
  assign traceIO_traces_0_insns_0_iaddr = tile_prci_domain_auto_tile_reset_domain_boom_tile_trace_out_0_iaddr;
  
  TilePRCIDomain tile_prci_domain (
  ...
  .auto_tile_reset_domain_boom_tile_trace_out_0_iaddr(
      tile_prci_domain_auto_tile_reset_domain_boom_tile_trace_out_0_iaddr),
  ...
  );
endmodule

module TilePRCIDomain(
...
  output [39:0] auto_tile_reset_domain_boom_tile_trace_out_0_iaddr,
...
)
  assign auto_tile_reset_domain_boom_tile_trace_out_0_iaddr = tile_reset_domain_auto_boom_tile_trace_out_0_iaddr;
  
  TileResetDomain tile_reset_domain (
  ...
    .auto_boom_tile_trace_out_0_iaddr(tile_reset_domain_auto_boom_tile_trace_out_0_iaddr),
  ...
  );
endmodule

module TileResetDomain(
...
  output [39:0] auto_boom_tile_trace_out_0_iaddr,
...
)
  assign auto_boom_tile_trace_out_0_iaddr = boom_tile_auto_trace_out_0_iaddr;
  BoomTile boom_tile (
  ...
      .auto_trace_out_0_iaddr(boom_tile_auto_trace_out_0_iaddr),
  ...
  );
endmodule

module BoomTile(
...
  output [39:0] auto_trace_out_0_iaddr,
...
)
  assign auto_trace_out_0_iaddr = trace_auto_out_0_iaddr;
  BundleBridgeNexus_13 trace (
  ...
	  // PC
      .auto_in_0_iaddr(trace_auto_in_0_iaddr),
      .auto_out_0_iaddr(trace_auto_out_0_iaddr),
  ...
  )
  
  assign trace_auto_in_0_iaddr = core_io_trace_0_iaddr;
    BoomCore core (
	  ...
      .io_trace_0_iaddr(core_io_trace_0_iaddr),
	  ...
	);
endmodule

module BoomCore (
...
  output [39:0] io_trace_0_iaddr,
...
)

  // This should be passed to Dromajo PC.
  assign io_trace_0_iaddr = _io_trace_0_iaddr_T_2[39:0];
  
  
endmodule



// This module wraps around the signals.
module BundleBridgeNexus_13 (
...
  input  [39:0] auto_in_0_iaddr,
  output [39:0] auto_out_0_iaddr,
...
)
  assign auto_out_0_iaddr = auto_in_0_iaddr;
  
endmodule
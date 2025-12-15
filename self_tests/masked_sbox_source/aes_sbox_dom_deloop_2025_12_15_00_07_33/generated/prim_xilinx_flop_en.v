(* DONT_TOUCH = "yes" *) module prim_xilinx_flop_en (
	clk_i,
	rst_ni,
	en_i,
	d_i,
	q_o
);
	parameter signed [31:0] Width = 1;
	parameter [0:0] EnSecBuf = 0;
	parameter [Width - 1:0] ResetValue = 0;
	input clk_i;
	input rst_ni;
	input en_i;
	input [Width - 1:0] d_i;
	output reg [Width - 1:0] q_o;
	always @(posedge clk_i or negedge rst_ni)
		if (!rst_ni)
			q_o <= ResetValue;
		else if (en_i)
			q_o <= d_i;
endmodule

module tlul_err (
	clk_i,
	rst_ni,
	tl_i,
	err_o
);
	input clk_i;
	input rst_ni;
	localparam signed [31:0] prim_mubi_pkg_MuBi4Width = 4;
	localparam signed [31:0] tlul_pkg_DataIntgWidth = 7;
	localparam signed [31:0] tlul_pkg_H2DCmdIntgWidth = 7;
	localparam signed [31:0] top_pkg_TL_AIW = 8;
	localparam signed [31:0] top_pkg_TL_AW = 32;
	localparam signed [31:0] top_pkg_TL_DW = 32;
	localparam signed [31:0] top_pkg_TL_DBW = top_pkg_TL_DW >> 3;
	localparam signed [31:0] top_pkg_TL_SZW = $clog2($clog2(top_pkg_TL_DBW) + 1);
	input wire [(((((7 + top_pkg_TL_SZW) + top_pkg_TL_AIW) + top_pkg_TL_AW) + top_pkg_TL_DBW) + top_pkg_TL_DW) + 23:0] tl_i;
	output wire err_o;
	localparam signed [31:0] IW = top_pkg_TL_AIW;
	localparam signed [31:0] SZW = top_pkg_TL_SZW;
	localparam signed [31:0] DW = top_pkg_TL_DW;
	localparam signed [31:0] MW = top_pkg_TL_DBW;
	localparam signed [31:0] SubAW = 2;
	wire opcode_allowed;
	wire a_config_allowed;
	wire op_full;
	wire op_partial;
	wire op_get;
	assign op_full = tl_i[6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55))))-:((6 + (top_pkg_TL_SZW + ((32'sd8 + 32'sd32) + (top_pkg_TL_DBW + 55)))) >= (3 + (top_pkg_TL_SZW + ((32'sd8 + 32'sd32) + (top_pkg_TL_DBW + 56)))) ? ((6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55))))) - (3 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 56)))))) + 1 : ((3 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 56))))) - (6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55)))))) + 1)] == 3'h0;
	assign op_partial = tl_i[6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55))))-:((6 + (top_pkg_TL_SZW + ((32'sd8 + 32'sd32) + (top_pkg_TL_DBW + 55)))) >= (3 + (top_pkg_TL_SZW + ((32'sd8 + 32'sd32) + (top_pkg_TL_DBW + 56)))) ? ((6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55))))) - (3 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 56)))))) + 1 : ((3 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 56))))) - (6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55)))))) + 1)] == 3'h1;
	assign op_get = tl_i[6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55))))-:((6 + (top_pkg_TL_SZW + ((32'sd8 + 32'sd32) + (top_pkg_TL_DBW + 55)))) >= (3 + (top_pkg_TL_SZW + ((32'sd8 + 32'sd32) + (top_pkg_TL_DBW + 56)))) ? ((6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55))))) - (3 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 56)))))) + 1 : ((3 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 56))))) - (6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55)))))) + 1)] == 3'h4;
	wire instr_wr_err;
	function automatic [3:0] sv2v_cast_289E7;
		input reg [3:0] inp;
		sv2v_cast_289E7 = inp;
	endfunction
	function automatic prim_mubi_pkg_mubi4_test_true_strict;
		input reg [3:0] val;
		prim_mubi_pkg_mubi4_test_true_strict = sv2v_cast_289E7(4'h6) == val;
	endfunction
	assign instr_wr_err = prim_mubi_pkg_mubi4_test_true_strict(tl_i[18-:4]) & (op_full | op_partial);
	wire instr_type_err;
	function automatic prim_mubi_pkg_mubi4_test_invalid;
		input reg [3:0] val;
		prim_mubi_pkg_mubi4_test_invalid = ~(|{((sv2v_cast_289E7(4'h6) ^ (val ^ val)) === (val ^ (sv2v_cast_289E7(4'h6) ^ sv2v_cast_289E7(4'h6)))) & ((((val ^ val) ^ (sv2v_cast_289E7(4'h6) ^ sv2v_cast_289E7(4'h6))) === (sv2v_cast_289E7(4'h6) ^ sv2v_cast_289E7(4'h6))) | 1'bx), ((sv2v_cast_289E7(4'h9) ^ (val ^ val)) === (val ^ (sv2v_cast_289E7(4'h9) ^ sv2v_cast_289E7(4'h9)))) & ((((val ^ val) ^ (sv2v_cast_289E7(4'h9) ^ sv2v_cast_289E7(4'h9))) === (sv2v_cast_289E7(4'h9) ^ sv2v_cast_289E7(4'h9))) | 1'bx)});
	endfunction
	assign instr_type_err = prim_mubi_pkg_mubi4_test_invalid(tl_i[18-:4]);
	assign err_o = (~(opcode_allowed & a_config_allowed) | instr_wr_err) | instr_type_err;
	assign opcode_allowed = ((tl_i[6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55))))-:((6 + (top_pkg_TL_SZW + ((32'sd8 + 32'sd32) + (top_pkg_TL_DBW + 55)))) >= (3 + (top_pkg_TL_SZW + ((32'sd8 + 32'sd32) + (top_pkg_TL_DBW + 56)))) ? ((6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55))))) - (3 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 56)))))) + 1 : ((3 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 56))))) - (6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55)))))) + 1)] == 3'h0) | (tl_i[6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55))))-:((6 + (top_pkg_TL_SZW + ((32'sd8 + 32'sd32) + (top_pkg_TL_DBW + 55)))) >= (3 + (top_pkg_TL_SZW + ((32'sd8 + 32'sd32) + (top_pkg_TL_DBW + 56)))) ? ((6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55))))) - (3 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 56)))))) + 1 : ((3 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 56))))) - (6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55)))))) + 1)] == 3'h1)) | (tl_i[6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55))))-:((6 + (top_pkg_TL_SZW + ((32'sd8 + 32'sd32) + (top_pkg_TL_DBW + 55)))) >= (3 + (top_pkg_TL_SZW + ((32'sd8 + 32'sd32) + (top_pkg_TL_DBW + 56)))) ? ((6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55))))) - (3 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 56)))))) + 1 : ((3 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 56))))) - (6 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55)))))) + 1)] == 3'h4);
	reg addr_sz_chk;
	reg mask_chk;
	reg fulldata_chk;
	wire [MW - 1:0] mask;
	assign mask = 1 << tl_i[(top_pkg_TL_AW + (top_pkg_TL_DBW + 55)) - 30:(top_pkg_TL_AW + (top_pkg_TL_DBW + 55)) - 31];
	always @(*) begin
		addr_sz_chk = 1'b0;
		mask_chk = 1'b0;
		fulldata_chk = 1'b0;
		if (tl_i[7 + (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55))))])
			case (tl_i[top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55)))-:((top_pkg_TL_SZW + ((32'sd8 + 32'sd32) + (top_pkg_TL_DBW + 55))) >= ((32'sd8 + 32'sd32) + (top_pkg_TL_DBW + 56)) ? ((top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55)))) - (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 56)))) + 1 : ((top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 56))) - (top_pkg_TL_SZW + (top_pkg_TL_AIW + (top_pkg_TL_AW + (top_pkg_TL_DBW + 55))))) + 1)])
				'h0: begin
					addr_sz_chk = 1'b1;
					mask_chk = ~|(tl_i[top_pkg_TL_DBW + 55-:((top_pkg_TL_DBW + 55) >= 56 ? top_pkg_TL_DBW : 57 - (top_pkg_TL_DBW + 55))] & ~mask);
					fulldata_chk = |(tl_i[top_pkg_TL_DBW + 55-:((top_pkg_TL_DBW + 55) >= 56 ? top_pkg_TL_DBW : 57 - (top_pkg_TL_DBW + 55))] & mask);
				end
				'h1: begin
					addr_sz_chk = ~tl_i[(top_pkg_TL_AW + (top_pkg_TL_DBW + 55)) - 31];
					mask_chk = (tl_i[(top_pkg_TL_AW + (top_pkg_TL_DBW + 55)) - 30] ? ~|(tl_i[top_pkg_TL_DBW + 55-:((top_pkg_TL_DBW + 55) >= 56 ? top_pkg_TL_DBW : 57 - (top_pkg_TL_DBW + 55))] & 4'b0011) : ~|(tl_i[top_pkg_TL_DBW + 55-:((top_pkg_TL_DBW + 55) >= 56 ? top_pkg_TL_DBW : 57 - (top_pkg_TL_DBW + 55))] & 4'b1100));
					fulldata_chk = (tl_i[(top_pkg_TL_AW + (top_pkg_TL_DBW + 55)) - 30] ? &tl_i[(top_pkg_TL_DBW + 55) - (top_pkg_TL_DBW - 4):(top_pkg_TL_DBW + 55) - (top_pkg_TL_DBW - 3)] : &tl_i[(top_pkg_TL_DBW + 55) - (top_pkg_TL_DBW - 2):(top_pkg_TL_DBW + 55) - (top_pkg_TL_DBW - 1)]);
				end
				'h2: begin
					addr_sz_chk = ~|tl_i[(top_pkg_TL_AW + (top_pkg_TL_DBW + 55)) - 30:(top_pkg_TL_AW + (top_pkg_TL_DBW + 55)) - 31];
					mask_chk = 1'b1;
					fulldata_chk = &tl_i[(top_pkg_TL_DBW + 55) - (top_pkg_TL_DBW - 4):(top_pkg_TL_DBW + 55) - (top_pkg_TL_DBW - 1)];
				end
				default: begin
					addr_sz_chk = 1'b0;
					mask_chk = 1'b0;
					fulldata_chk = 1'b0;
				end
			endcase
		else begin
			addr_sz_chk = 1'b0;
			mask_chk = 1'b0;
			fulldata_chk = 1'b0;
		end
	end
	assign a_config_allowed = (addr_sz_chk & mask_chk) & ((op_get | op_partial) | fulldata_chk);
endmodule

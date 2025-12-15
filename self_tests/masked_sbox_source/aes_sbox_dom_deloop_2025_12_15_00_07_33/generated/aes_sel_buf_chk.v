module aes_sel_buf_chk (
	clk_i,
	rst_ni,
	sel_i,
	sel_o,
	err_o
);
	parameter signed [31:0] Num = 2;
	parameter signed [31:0] Width = 1;
	parameter [0:0] EnSecBuf = 1'b0;
	input wire clk_i;
	input wire rst_ni;
	input wire [Width - 1:0] sel_i;
	output wire [Width - 1:0] sel_o;
	output reg err_o;
	wire unused_clk;
	wire unused_rst;
	assign unused_clk = clk_i;
	assign unused_rst = rst_ni;
	generate
		if (EnSecBuf) begin : gen_sec_buf
			prim_xilinx_buf #(.Width(Width)) u_prim_xilinx_buf_sel_i(
				.in_i(sel_i),
				.out_o(sel_o)
			);
		end
		else begin : gen_buf
			prim_xilinx_buf #(.Width(Width)) u_prim_xilinx_buf_sel_i(
				.in_i(sel_i),
				.out_o(sel_o)
			);
		end
	endgenerate
	localparam signed [31:0] aes_pkg_Mux2SelWidth = 3;
	localparam signed [31:0] aes_pkg_Mux3SelWidth = 5;
	localparam signed [31:0] aes_pkg_Mux4SelWidth = 5;
	localparam signed [31:0] aes_pkg_Mux6SelWidth = 6;
	function automatic [5:0] sv2v_cast_8208B;
		input reg [5:0] inp;
		sv2v_cast_8208B = inp;
	endfunction
	function automatic [4:0] sv2v_cast_02721;
		input reg [4:0] inp;
		sv2v_cast_02721 = inp;
	endfunction
	function automatic [4:0] sv2v_cast_F4B48;
		input reg [4:0] inp;
		sv2v_cast_F4B48 = inp;
	endfunction
	function automatic [2:0] sv2v_cast_0397F;
		input reg [2:0] inp;
		sv2v_cast_0397F = inp;
	endfunction
	generate
		if (Num == 2) begin : gen_mux2_sel_chk
			wire [2:0] sel_chk;
			assign sel_chk = sv2v_cast_0397F(sel_o);
			always @(*) begin : mux2_sel_chk
				case (sel_chk)
					sv2v_cast_0397F(3'b011), sv2v_cast_0397F(3'b100): err_o = 1'b0;
					default: err_o = 1'b1;
				endcase
			end
		end
		else if (Num == 3) begin : gen_mux3_sel_chk
			wire [4:0] sel_chk;
			assign sel_chk = sv2v_cast_F4B48(sel_o);
			always @(*) begin : mux3_sel_chk
				case (sel_chk)
					sv2v_cast_F4B48(5'b01110), sv2v_cast_F4B48(5'b11000), sv2v_cast_F4B48(5'b00001): err_o = 1'b0;
					default: err_o = 1'b1;
				endcase
			end
		end
		else if (Num == 4) begin : gen_mux4_sel_chk
			wire [4:0] sel_chk;
			assign sel_chk = sv2v_cast_02721(sel_o);
			always @(*) begin : mux4_sel_chk
				case (sel_chk)
					sv2v_cast_02721(5'b01110), sv2v_cast_02721(5'b11000), sv2v_cast_02721(5'b00001), sv2v_cast_02721(5'b10111): err_o = 1'b0;
					default: err_o = 1'b1;
				endcase
			end
		end
		else if (Num == 6) begin : gen_mux6_sel_chk
			wire [5:0] sel_chk;
			assign sel_chk = sv2v_cast_8208B(sel_o);
			always @(*) begin : mux6_sel_chk
				case (sel_chk)
					sv2v_cast_8208B(6'b011101), sv2v_cast_8208B(6'b110000), sv2v_cast_8208B(6'b001000), sv2v_cast_8208B(6'b000011), sv2v_cast_8208B(6'b111110), sv2v_cast_8208B(6'b100101): err_o = 1'b0;
					default: err_o = 1'b1;
				endcase
			end
		end
		else begin : gen_width_unsupported
			wire [1:1] sv2v_tmp_398A5;
			assign sv2v_tmp_398A5 = 1'b1;
			always @(*) err_o = sv2v_tmp_398A5;
		end
	endgenerate
endmodule

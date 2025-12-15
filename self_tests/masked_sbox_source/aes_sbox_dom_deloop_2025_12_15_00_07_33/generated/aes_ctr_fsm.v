module aes_ctr_fsm (
	clk_i,
	rst_ni,
	incr_i,
	ready_o,
	incr_err_i,
	mr_err_i,
	alert_o,
	ctr_slice_idx_o,
	ctr_slice_i,
	ctr_slice_o,
	ctr_we_o
);
	input wire clk_i;
	input wire rst_ni;
	input wire incr_i;
	output reg ready_o;
	input wire incr_err_i;
	input wire mr_err_i;
	output reg alert_o;
	localparam [31:0] aes_pkg_SliceSizeCtr = 16;
	localparam signed [31:0] aes_reg_pkg_NumRegsIv = 4;
	localparam [31:0] aes_pkg_NumSlicesCtr = 8;
	function automatic integer prim_util_pkg_vbits;
		input integer value;
		prim_util_pkg_vbits = (value == 1 ? 1 : $clog2(value));
	endfunction
	localparam [31:0] aes_pkg_SliceIdxWidth = prim_util_pkg_vbits(aes_pkg_NumSlicesCtr);
	output wire [aes_pkg_SliceIdxWidth - 1:0] ctr_slice_idx_o;
	input wire [15:0] ctr_slice_i;
	output wire [15:0] ctr_slice_o;
	output reg ctr_we_o;
	localparam signed [31:0] aes_pkg_CtrStateWidth = 5;
	reg [4:0] aes_ctr_ns;
	wire [4:0] aes_ctr_cs;
	reg [aes_pkg_SliceIdxWidth - 1:0] ctr_slice_idx_d;
	reg [aes_pkg_SliceIdxWidth - 1:0] ctr_slice_idx_q;
	reg ctr_carry_d;
	reg ctr_carry_q;
	wire [aes_pkg_SliceSizeCtr:0] ctr_value;
	assign ctr_value = ctr_slice_i + {{15 {1'b0}}, ctr_carry_q};
	assign ctr_slice_o = ctr_value[15:0];
	function automatic [4:0] sv2v_cast_A7620;
		input reg [4:0] inp;
		sv2v_cast_A7620 = inp;
	endfunction
	function automatic signed [aes_pkg_SliceIdxWidth - 1:0] sv2v_cast_12137_signed;
		input reg signed [aes_pkg_SliceIdxWidth - 1:0] inp;
		sv2v_cast_12137_signed = inp;
	endfunction
	always @(*) begin : aes_ctr_fsm_comb
		ready_o = 1'b0;
		ctr_we_o = 1'b0;
		alert_o = 1'b0;
		aes_ctr_ns = aes_ctr_cs;
		ctr_slice_idx_d = ctr_slice_idx_q;
		ctr_carry_d = ctr_carry_q;
		case (aes_ctr_cs)
			sv2v_cast_A7620(5'b01110): begin
				ready_o = 1'b1;
				if (incr_i == 1'b1) begin
					ctr_slice_idx_d = 1'sb0;
					ctr_carry_d = 1'b1;
					aes_ctr_ns = sv2v_cast_A7620(5'b11000);
				end
			end
			sv2v_cast_A7620(5'b11000): begin
				ctr_slice_idx_d = ctr_slice_idx_q + sv2v_cast_12137_signed(1);
				ctr_carry_d = ctr_value[aes_pkg_SliceSizeCtr];
				ctr_we_o = 1'b1;
				if (ctr_slice_idx_q == {aes_pkg_SliceIdxWidth {1'b1}})
					aes_ctr_ns = sv2v_cast_A7620(5'b01110);
			end
			sv2v_cast_A7620(5'b00001): alert_o = 1'b1;
			default: begin
				aes_ctr_ns = sv2v_cast_A7620(5'b00001);
				alert_o = 1'b1;
			end
		endcase
		if (incr_err_i || mr_err_i)
			aes_ctr_ns = sv2v_cast_A7620(5'b00001);
	end
	always @(posedge clk_i or negedge rst_ni)
		if (!rst_ni) begin
			ctr_slice_idx_q <= 1'sb0;
			ctr_carry_q <= 1'sb0;
		end
		else begin
			ctr_slice_idx_q <= ctr_slice_idx_d;
			ctr_carry_q <= ctr_carry_d;
		end
	prim_sparse_fsm_flop #(
		.Width(aes_pkg_CtrStateWidth),
		.ResetValue(sv2v_cast_A7620(5'b01110)),
		.EnableAlertTriggerSVA(1)
	) u_state_regs(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.state_i(aes_ctr_ns),
		.state_o(aes_ctr_cs)
	);
	assign ctr_slice_idx_o = ctr_slice_idx_q;
endmodule

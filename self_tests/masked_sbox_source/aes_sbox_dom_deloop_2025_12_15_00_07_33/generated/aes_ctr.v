module aes_ctr (
	clk_i,
	rst_ni,
	incr_i,
	ready_o,
	alert_o,
	ctr_i,
	ctr_o,
	ctr_we_o
);
	input wire clk_i;
	input wire rst_ni;
	localparam signed [31:0] aes_pkg_Mux2SelWidth = 3;
	localparam signed [31:0] aes_pkg_Sp2VWidth = aes_pkg_Mux2SelWidth;
	input wire [2:0] incr_i;
	output wire [2:0] ready_o;
	output wire alert_o;
	localparam [31:0] aes_pkg_SliceSizeCtr = 16;
	localparam signed [31:0] aes_reg_pkg_NumRegsIv = 4;
	localparam [31:0] aes_pkg_NumSlicesCtr = 8;
	input wire [(aes_pkg_NumSlicesCtr * aes_pkg_SliceSizeCtr) - 1:0] ctr_i;
	output wire [(aes_pkg_NumSlicesCtr * aes_pkg_SliceSizeCtr) - 1:0] ctr_o;
	output wire [(aes_pkg_NumSlicesCtr * aes_pkg_Sp2VWidth) - 1:0] ctr_we_o;
	function automatic [127:0] aes_rev_order_byte;
		input reg [127:0] in;
		reg [127:0] out;
		begin
			begin : sv2v_autoblock_1
				reg signed [31:0] i;
				for (i = 0; i < 16; i = i + 1)
					out[i * 8+:8] = in[(15 - i) * 8+:8];
			end
			aes_rev_order_byte = out;
		end
	endfunction
	function automatic [(aes_pkg_NumSlicesCtr * aes_pkg_Sp2VWidth) - 1:0] aes_rev_order_sp2v;
		input reg [(aes_pkg_NumSlicesCtr * aes_pkg_Sp2VWidth) - 1:0] in;
		reg [(aes_pkg_NumSlicesCtr * aes_pkg_Sp2VWidth) - 1:0] out;
		begin
			begin : sv2v_autoblock_2
				reg signed [31:0] i;
				for (i = 0; i < aes_pkg_NumSlicesCtr; i = i + 1)
					out[i * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth] = in[(7 - i) * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth];
			end
			aes_rev_order_sp2v = out;
		end
	endfunction
	function automatic integer prim_util_pkg_vbits;
		input integer value;
		prim_util_pkg_vbits = (value == 1 ? 1 : $clog2(value));
	endfunction
	localparam [31:0] aes_pkg_SliceIdxWidth = prim_util_pkg_vbits(aes_pkg_NumSlicesCtr);
	reg [aes_pkg_SliceIdxWidth - 1:0] ctr_slice_idx;
	wire [(aes_pkg_NumSlicesCtr * aes_pkg_SliceSizeCtr) - 1:0] ctr_i_rev;
	reg [(aes_pkg_NumSlicesCtr * aes_pkg_SliceSizeCtr) - 1:0] ctr_o_rev;
	reg [(aes_pkg_NumSlicesCtr * aes_pkg_Sp2VWidth) - 1:0] ctr_we_o_rev;
	wire [2:0] ctr_we;
	wire [15:0] ctr_i_slice;
	reg [15:0] ctr_o_slice;
	wire [2:0] incr;
	wire incr_err;
	reg mr_err;
	wire [2:0] sp_incr;
	wire [2:0] sp_ready;
	wire [2:0] sp_ctr_we;
	wire [2:0] mr_alert;
	wire [(aes_pkg_Sp2VWidth * aes_pkg_SliceIdxWidth) - 1:0] mr_ctr_slice_idx;
	wire [(aes_pkg_Sp2VWidth * aes_pkg_SliceSizeCtr) - 1:0] mr_ctr_o_slice;
	assign ctr_i_rev = aes_rev_order_byte(ctr_i);
	wire [2:0] incr_raw;
	localparam signed [31:0] aes_pkg_Sp2VNum = 2;
	aes_sel_buf_chk #(
		.Num(aes_pkg_Sp2VNum),
		.Width(aes_pkg_Sp2VWidth),
		.EnSecBuf(1'b0)
	) u_aes_sb_en_buf_chk(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.sel_i(incr_i),
		.sel_o(incr_raw),
		.err_o(incr_err)
	);
	function automatic [2:0] sv2v_cast_AC049;
		input reg [2:0] inp;
		sv2v_cast_AC049 = inp;
	endfunction
	assign incr = sv2v_cast_AC049(incr_raw);
	assign ctr_i_slice = ctr_i_rev[ctr_slice_idx * aes_pkg_SliceSizeCtr+:aes_pkg_SliceSizeCtr];
	assign sp_incr = {incr};
	genvar _gv_i_1;
	function automatic [2:0] sv2v_cast_0397F;
		input reg [2:0] inp;
		sv2v_cast_0397F = inp;
	endfunction
	localparam [2:0] aes_pkg_SP2V_LOGIC_HIGH = {sv2v_cast_AC049(sv2v_cast_0397F(3'b011))};
	generate
		for (_gv_i_1 = 0; _gv_i_1 < aes_pkg_Sp2VWidth; _gv_i_1 = _gv_i_1 + 1) begin : gen_fsm
			localparam i = _gv_i_1;
			if (aes_pkg_SP2V_LOGIC_HIGH[i] == 1'b1) begin : gen_fsm_p
				aes_ctr_fsm_p u_aes_ctr_fsm_i(
					.clk_i(clk_i),
					.rst_ni(rst_ni),
					.incr_i(sp_incr[i]),
					.ready_o(sp_ready[i]),
					.incr_err_i(incr_err),
					.mr_err_i(mr_err),
					.alert_o(mr_alert[i]),
					.ctr_slice_idx_o(mr_ctr_slice_idx[i * aes_pkg_SliceIdxWidth+:aes_pkg_SliceIdxWidth]),
					.ctr_slice_i(ctr_i_slice),
					.ctr_slice_o(mr_ctr_o_slice[i * aes_pkg_SliceSizeCtr+:aes_pkg_SliceSizeCtr]),
					.ctr_we_o(sp_ctr_we[i])
				);
			end
			else begin : gen_fsm_n
				aes_ctr_fsm_n u_aes_ctr_fsm_i(
					.clk_i(clk_i),
					.rst_ni(rst_ni),
					.incr_ni(sp_incr[i]),
					.ready_no(sp_ready[i]),
					.incr_err_i(incr_err),
					.mr_err_i(mr_err),
					.alert_o(mr_alert[i]),
					.ctr_slice_idx_o(mr_ctr_slice_idx[i * aes_pkg_SliceIdxWidth+:aes_pkg_SliceIdxWidth]),
					.ctr_slice_i(ctr_i_slice),
					.ctr_slice_o(mr_ctr_o_slice[i * aes_pkg_SliceSizeCtr+:aes_pkg_SliceSizeCtr]),
					.ctr_we_no(sp_ctr_we[i])
				);
			end
		end
	endgenerate
	assign ready_o = sv2v_cast_AC049(sp_ready);
	assign ctr_we = sv2v_cast_AC049(sp_ctr_we);
	assign alert_o = |mr_alert;
	always @(*) begin : combine_sparse_signals
		ctr_slice_idx = 1'sb0;
		ctr_o_slice = 1'sb0;
		mr_err = 1'b0;
		begin : sv2v_autoblock_3
			reg signed [31:0] i;
			for (i = 0; i < aes_pkg_Sp2VWidth; i = i + 1)
				begin
					ctr_slice_idx = ctr_slice_idx | mr_ctr_slice_idx[i * aes_pkg_SliceIdxWidth+:aes_pkg_SliceIdxWidth];
					ctr_o_slice = ctr_o_slice | mr_ctr_o_slice[i * aes_pkg_SliceSizeCtr+:aes_pkg_SliceSizeCtr];
				end
		end
		begin : sv2v_autoblock_4
			reg signed [31:0] i;
			for (i = 0; i < aes_pkg_Sp2VWidth; i = i + 1)
				if ((ctr_slice_idx != mr_ctr_slice_idx[i * aes_pkg_SliceIdxWidth+:aes_pkg_SliceIdxWidth]) || (ctr_o_slice != mr_ctr_o_slice[i * aes_pkg_SliceSizeCtr+:aes_pkg_SliceSizeCtr]))
					mr_err = 1'b1;
		end
	end
	always @(*) begin
		ctr_o_rev = ctr_i_rev;
		ctr_o_rev[ctr_slice_idx * aes_pkg_SliceSizeCtr+:aes_pkg_SliceSizeCtr] = ctr_o_slice;
	end
	always @(*) begin
		ctr_we_o_rev = {aes_pkg_NumSlicesCtr {sv2v_cast_AC049(sv2v_cast_0397F(3'b100))}};
		ctr_we_o_rev[ctr_slice_idx * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth] = ctr_we;
	end
	assign ctr_o = aes_rev_order_byte(ctr_o_rev);
	assign ctr_we_o = aes_rev_order_sp2v(ctr_we_o_rev);
endmodule

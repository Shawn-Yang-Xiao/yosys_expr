module aes_core (
	clk_i,
	rst_ni,
	rst_shadowed_ni,
	entropy_clearing_req_o,
	entropy_clearing_ack_i,
	entropy_clearing_i,
	entropy_masking_req_o,
	entropy_masking_ack_i,
	entropy_masking_i,
	keymgr_key_i,
	lc_escalate_en_i,
	shadowed_storage_err_i,
	shadowed_update_err_i,
	intg_err_alert_i,
	alert_recov_o,
	alert_fatal_o,
	reg2hw,
	hw2reg
);
	parameter [0:0] AES192Enable = 1;
	parameter [0:0] SecMasking = 1;
	parameter integer SecSBoxImpl = 32'sd4;
	parameter [31:0] SecStartTriggerDelay = 0;
	parameter [0:0] SecAllowForcingMasks = 0;
	parameter [0:0] SecSkipPRNGReseeding = 0;
	localparam [31:0] edn_pkg_ENDPOINT_BUS_WIDTH = 32;
	parameter [31:0] EntropyWidth = edn_pkg_ENDPOINT_BUS_WIDTH;
	localparam signed [31:0] NumShares = (SecMasking ? 2 : 1);
	localparam signed [31:0] aes_pkg_ClearingLfsrWidth = 64;
	localparam [63:0] aes_pkg_RndCnstClearingLfsrSeedDefault = 64'hc32d580f74f1713a;
	parameter [63:0] RndCnstClearingLfsrSeed = aes_pkg_RndCnstClearingLfsrSeedDefault;
	localparam [383:0] aes_pkg_RndCnstClearingLfsrPermDefault = 384'hb33fdfc81deb6292c21f8a31025850679c2f4be1bbe937b4b7c9d7f4e57568d99c8ae291a899143e0d8459d31b143223;
	parameter [383:0] RndCnstClearingLfsrPerm = aes_pkg_RndCnstClearingLfsrPermDefault;
	localparam [383:0] aes_pkg_RndCnstClearingSharePermDefault = 384'hf66fd61b27847edc2286706fb3a2e9009736b95ac3f3b5205caf8dc536aad73605d393c8dd94476e830e97891d4828d0;
	parameter [383:0] RndCnstClearingSharePerm = aes_pkg_RndCnstClearingSharePermDefault;
	localparam signed [31:0] aes_pkg_MaskingLfsrWidth = 160;
	localparam [159:0] aes_pkg_RndCnstMaskingLfsrSeedDefault = 160'h0c132b5723c5a4cf4743b3c7c32d580f74f1713a;
	parameter [159:0] RndCnstMaskingLfsrSeed = aes_pkg_RndCnstMaskingLfsrSeedDefault;
	localparam [1279:0] aes_pkg_RndCnstMaskingLfsrPermDefault = 1280'h17261943423e4c5c03872194050c7e5f8497081d96666d406f4b6064733034698e7c721c8832471f59919e0b128f067b25622768462e554d8970815d490d7f44048c867d907a239b20220f6c79071a852d76485452189f14091b1e744e3967374f785b772b352f6550613c58130a8b104a3f28019c9a380233956b00563a512c808d419d63982a16995e0e3b57826a36718a9329452492533d83115a75316e15;
	parameter [1279:0] RndCnstMaskingLfsrPerm = aes_pkg_RndCnstMaskingLfsrPermDefault;
	input wire clk_i;
	input wire rst_ni;
	input wire rst_shadowed_ni;
	output wire entropy_clearing_req_o;
	input wire entropy_clearing_ack_i;
	input wire [EntropyWidth - 1:0] entropy_clearing_i;
	output wire entropy_masking_req_o;
	input wire entropy_masking_ack_i;
	input wire [EntropyWidth - 1:0] entropy_masking_i;
	localparam signed [31:0] keymgr_pkg_KeyWidth = 256;
	localparam signed [31:0] keymgr_pkg_Shares = 2;
	input wire [(1 + (keymgr_pkg_Shares * keymgr_pkg_KeyWidth)) - 1:0] keymgr_key_i;
	localparam signed [31:0] lc_ctrl_pkg_TxWidth = 4;
	input wire [3:0] lc_escalate_en_i;
	input wire shadowed_storage_err_i;
	input wire shadowed_update_err_i;
	input wire intg_err_alert_i;
	output wire alert_recov_o;
	output wire alert_fatal_o;
	input wire [963:0] reg2hw;
	output reg [937:0] hw2reg;
	wire ctrl_qe;
	wire ctrl_we;
	wire ctrl_phase;
	localparam signed [31:0] aes_pkg_AES_OP_WIDTH = 2;
	wire [1:0] aes_op_q;
	localparam signed [31:0] aes_pkg_AES_MODE_WIDTH = 6;
	wire [5:0] aes_mode_q;
	wire [1:0] cipher_op;
	wire [1:0] cipher_op_buf;
	localparam signed [31:0] aes_pkg_AES_KEYLEN_WIDTH = 3;
	wire [2:0] key_len_q;
	wire sideload_q;
	localparam signed [31:0] aes_pkg_AES_PRNGRESEEDRATE_WIDTH = 3;
	wire [2:0] prng_reseed_rate_q;
	wire manual_operation_q;
	wire ctrl_reg_err_update;
	wire ctrl_reg_err_storage;
	wire ctrl_err_update;
	wire ctrl_err_storage;
	wire ctrl_err_storage_d;
	reg ctrl_err_storage_q;
	wire ctrl_alert;
	wire key_touch_forces_reseed;
	wire force_masks;
	wire mux_sel_err;
	wire sp_enc_err_d;
	reg sp_enc_err_q;
	wire clear_on_fatal;
	reg [127:0] state_in;
	localparam signed [31:0] aes_pkg_Mux2SelWidth = 3;
	localparam signed [31:0] aes_pkg_SISelWidth = aes_pkg_Mux2SelWidth;
	wire [2:0] state_in_sel_raw;
	wire [2:0] state_in_sel_ctrl;
	wire [2:0] state_in_sel;
	wire state_in_sel_err;
	reg [127:0] add_state_in;
	localparam signed [31:0] aes_pkg_AddSISelWidth = aes_pkg_Mux2SelWidth;
	wire [2:0] add_state_in_sel_raw;
	wire [2:0] add_state_in_sel_ctrl;
	wire [2:0] add_state_in_sel;
	wire add_state_in_sel_err;
	wire [127:0] state_mask;
	wire [(((NumShares * 4) * 4) * 8) - 1:0] state_init;
	wire [(((NumShares * 4) * 4) * 8) - 1:0] state_done;
	wire [127:0] state_out;
	localparam [31:0] aes_pkg_NumSharesKey = 2;
	localparam signed [31:0] aes_reg_pkg_NumRegsKey = 8;
	reg [((aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) * 32) - 1:0] key_init;
	reg [7:0] key_init_qe [0:1];
	wire [(aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) - 1:0] key_init_qe_buf;
	reg [((aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) * 32) - 1:0] key_init_d;
	reg [((aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) * 32) - 1:0] key_init_q;
	wire [((NumShares * aes_reg_pkg_NumRegsKey) * 32) - 1:0] key_init_cipher;
	localparam signed [31:0] aes_pkg_Sp2VWidth = aes_pkg_Mux2SelWidth;
	wire [((aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) * 3) - 1:0] key_init_we_ctrl;
	wire [(aes_reg_pkg_NumRegsKey * aes_pkg_Sp2VWidth) - 1:0] key_init_we [0:1];
	localparam signed [31:0] aes_pkg_Mux3SelWidth = 5;
	localparam signed [31:0] aes_pkg_KeyInitSelWidth = aes_pkg_Mux3SelWidth;
	wire [4:0] key_init_sel_raw;
	wire [4:0] key_init_sel_ctrl;
	wire [4:0] key_init_sel;
	wire key_init_sel_err;
	reg [((aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) * 32) - 1:0] key_sideload;
	localparam signed [31:0] aes_reg_pkg_NumRegsIv = 4;
	reg [127:0] iv;
	reg [3:0] iv_qe;
	wire [3:0] iv_qe_buf;
	localparam [31:0] aes_pkg_SliceSizeCtr = 16;
	localparam [31:0] aes_pkg_NumSlicesCtr = 8;
	reg [(aes_pkg_NumSlicesCtr * aes_pkg_SliceSizeCtr) - 1:0] iv_d;
	reg [(aes_pkg_NumSlicesCtr * aes_pkg_SliceSizeCtr) - 1:0] iv_q;
	wire [(aes_pkg_NumSlicesCtr * aes_pkg_Sp2VWidth) - 1:0] iv_we_ctrl;
	wire [(aes_pkg_NumSlicesCtr * aes_pkg_Sp2VWidth) - 1:0] iv_we;
	localparam signed [31:0] aes_pkg_Mux6SelWidth = 6;
	localparam signed [31:0] aes_pkg_IVSelWidth = aes_pkg_Mux6SelWidth;
	wire [5:0] iv_sel_raw;
	wire [5:0] iv_sel_ctrl;
	wire [5:0] iv_sel;
	wire iv_sel_err;
	wire [(aes_pkg_NumSlicesCtr * aes_pkg_SliceSizeCtr) - 1:0] ctr;
	wire [(aes_pkg_NumSlicesCtr * aes_pkg_Sp2VWidth) - 1:0] ctr_we;
	wire [2:0] ctr_incr;
	wire [2:0] ctr_ready;
	wire ctr_alert;
	localparam signed [31:0] aes_reg_pkg_NumRegsData = 4;
	reg [127:0] data_in_prev_d;
	reg [127:0] data_in_prev_q;
	wire [2:0] data_in_prev_we_ctrl;
	wire [2:0] data_in_prev_we;
	localparam signed [31:0] aes_pkg_DIPSelWidth = aes_pkg_Mux2SelWidth;
	wire [2:0] data_in_prev_sel_raw;
	wire [2:0] data_in_prev_sel_ctrl;
	wire [2:0] data_in_prev_sel;
	wire data_in_prev_sel_err;
	reg [127:0] data_in;
	reg [3:0] data_in_qe;
	wire [3:0] data_in_qe_buf;
	wire data_in_we;
	reg [127:0] add_state_out;
	localparam signed [31:0] aes_pkg_AddSOSelWidth = aes_pkg_Mux3SelWidth;
	wire [4:0] add_state_out_sel_raw;
	wire [4:0] add_state_out_sel_ctrl;
	wire [4:0] add_state_out_sel;
	wire add_state_out_sel_err;
	wire [127:0] data_out_d;
	reg [127:0] data_out_q;
	wire [2:0] data_out_we_ctrl;
	wire [2:0] data_out_we;
	reg [3:0] data_out_re;
	wire [3:0] data_out_re_buf;
	wire [2:0] cipher_in_valid;
	wire [2:0] cipher_in_ready;
	wire [2:0] cipher_out_valid;
	wire [2:0] cipher_out_ready;
	wire [2:0] cipher_crypt;
	wire [2:0] cipher_crypt_busy;
	wire [2:0] cipher_dec_key_gen;
	wire [2:0] cipher_dec_key_gen_busy;
	wire cipher_prng_reseed;
	wire cipher_prng_reseed_busy;
	wire cipher_key_clear;
	wire cipher_key_clear_busy;
	wire cipher_data_out_clear;
	wire cipher_data_out_clear_busy;
	wire cipher_alert;
	localparam [31:0] aes_pkg_WidthPRDClearing = 64;
	wire [(NumShares * aes_pkg_WidthPRDClearing) - 1:0] cipher_prd_clearing;
	wire [(aes_pkg_NumSharesKey * aes_pkg_WidthPRDClearing) - 1:0] prd_clearing;
	wire prd_clearing_upd_req;
	wire prd_clearing_upd_ack;
	wire prd_clearing_rsd_req;
	wire prd_clearing_rsd_ack;
	wire [127:0] prd_clearing_128 [0:NumShares - 1];
	wire [511:0] prd_clearing_256;
	reg [127:0] unused_data_out_q;
	aes_prng_clearing #(
		.Width(aes_pkg_WidthPRDClearing),
		.EntropyWidth(EntropyWidth),
		.SecSkipPRNGReseeding(SecSkipPRNGReseeding),
		.RndCnstLfsrSeed(RndCnstClearingLfsrSeed),
		.RndCnstLfsrPerm(RndCnstClearingLfsrPerm),
		.RndCnstSharePerm(RndCnstClearingSharePerm)
	) u_aes_prng_clearing(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.data_req_i(prd_clearing_upd_req),
		.data_ack_o(prd_clearing_upd_ack),
		.data_o(prd_clearing),
		.reseed_req_i(prd_clearing_rsd_req),
		.reseed_ack_o(prd_clearing_rsd_ack),
		.entropy_req_o(entropy_clearing_req_o),
		.entropy_ack_i(entropy_clearing_ack_i),
		.entropy_i(entropy_clearing_i)
	);
	genvar _gv_s_1;
	localparam [31:0] aes_pkg_NumChunksPRDClearing128 = 2;
	generate
		for (_gv_s_1 = 0; _gv_s_1 < NumShares; _gv_s_1 = _gv_s_1 + 1) begin : gen_prd_clearing_128_shares
			localparam s = _gv_s_1;
			genvar _gv_c_1;
			for (_gv_c_1 = 0; _gv_c_1 < aes_pkg_NumChunksPRDClearing128; _gv_c_1 = _gv_c_1 + 1) begin : gen_prd_clearing_128
				localparam c = _gv_c_1;
				assign prd_clearing_128[s][c * aes_pkg_WidthPRDClearing+:aes_pkg_WidthPRDClearing] = prd_clearing[(1 - s) * aes_pkg_WidthPRDClearing+:aes_pkg_WidthPRDClearing];
			end
		end
	endgenerate
	genvar _gv_s_2;
	localparam [31:0] aes_pkg_NumChunksPRDClearing256 = 4;
	generate
		for (_gv_s_2 = 0; _gv_s_2 < aes_pkg_NumSharesKey; _gv_s_2 = _gv_s_2 + 1) begin : gen_prd_clearing_256_shares
			localparam s = _gv_s_2;
			genvar _gv_c_2;
			for (_gv_c_2 = 0; _gv_c_2 < aes_pkg_NumChunksPRDClearing256; _gv_c_2 = _gv_c_2 + 1) begin : gen_prd_clearing_256
				localparam c = _gv_c_2;
				assign prd_clearing_256[((1 - s) * 256) + (c * aes_pkg_WidthPRDClearing)+:aes_pkg_WidthPRDClearing] = prd_clearing[(1 - s) * aes_pkg_WidthPRDClearing+:aes_pkg_WidthPRDClearing];
			end
		end
	endgenerate
	always @(*) begin : key_init_get
		begin : sv2v_autoblock_1
			reg signed [31:0] i;
			for (i = 0; i < aes_reg_pkg_NumRegsKey; i = i + 1)
				begin
					key_init[(8 + i) * 32+:32] = reg2hw[696 + ((i * 33) + 32)-:32];
					key_init_qe[0][i] = reg2hw[696 + (i * 33)];
					key_init[(0 + i) * 32+:32] = reg2hw[432 + ((i * 33) + 32)-:32];
					key_init_qe[1][i] = reg2hw[432 + (i * 33)];
				end
		end
	end
	prim_xilinx_buf #(.Width(aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey)) u_prim_xilinx_buf_key_init_qe(
		.in_i({key_init_qe[1], key_init_qe[0]}),
		.out_o({key_init_qe_buf[0+:aes_reg_pkg_NumRegsKey], key_init_qe_buf[8+:aes_reg_pkg_NumRegsKey]})
	);
	always @(*) begin : key_sideload_get
		begin : sv2v_autoblock_2
			reg signed [31:0] s;
			for (s = 0; s < aes_pkg_NumSharesKey; s = s + 1)
				begin : sv2v_autoblock_3
					reg signed [31:0] i;
					for (i = 0; i < aes_reg_pkg_NumRegsKey; i = i + 1)
						key_sideload[(((1 - s) * aes_reg_pkg_NumRegsKey) + i) * 32+:32] = keymgr_key_i[((keymgr_pkg_Shares * keymgr_pkg_KeyWidth) - 1) - (((keymgr_pkg_Shares * keymgr_pkg_KeyWidth) - 1) - ((s * keymgr_pkg_KeyWidth) + (i * 32)))+:32];
				end
		end
	end
	always @(*) begin : iv_get
		begin : sv2v_autoblock_4
			reg signed [31:0] i;
			for (i = 0; i < aes_reg_pkg_NumRegsIv; i = i + 1)
				begin
					iv[i * 32+:32] = reg2hw[300 + ((i * 33) + 32)-:32];
					iv_qe[i] = reg2hw[300 + (i * 33)];
				end
		end
	end
	prim_xilinx_buf #(.Width(aes_reg_pkg_NumRegsIv)) u_prim_xilinx_buf_iv_qe(
		.in_i(iv_qe),
		.out_o(iv_qe_buf)
	);
	always @(*) begin : data_in_get
		begin : sv2v_autoblock_5
			reg signed [31:0] i;
			for (i = 0; i < aes_reg_pkg_NumRegsData; i = i + 1)
				begin
					data_in[i * 32+:32] = reg2hw[168 + ((i * 33) + 32)-:32];
					data_in_qe[i] = reg2hw[168 + (i * 33)];
				end
		end
	end
	prim_xilinx_buf #(.Width(aes_reg_pkg_NumRegsData)) u_prim_xilinx_buf_data_in_qe(
		.in_i(data_in_qe),
		.out_o(data_in_qe_buf)
	);
	always @(*) begin : data_out_get
		begin : sv2v_autoblock_6
			reg signed [31:0] i;
			for (i = 0; i < aes_reg_pkg_NumRegsData; i = i + 1)
				begin
					unused_data_out_q[i * 32+:32] = reg2hw[36 + ((i * 33) + 32)-:32];
					data_out_re[i] = reg2hw[36 + (i * 33)];
				end
		end
	end
	prim_xilinx_buf #(.Width(aes_reg_pkg_NumRegsData)) u_prim_xilinx_buf_data_out_re(
		.in_i(data_out_re),
		.out_o(data_out_re_buf)
	);
	function automatic [4:0] sv2v_cast_F4B48;
		input reg [4:0] inp;
		sv2v_cast_F4B48 = inp;
	endfunction
	function automatic [4:0] sv2v_cast_C3037;
		input reg [4:0] inp;
		sv2v_cast_C3037 = inp;
	endfunction
	always @(*) begin : key_init_mux
		case (key_init_sel)
			sv2v_cast_C3037(sv2v_cast_F4B48(5'b01110)): key_init_d = key_init;
			sv2v_cast_C3037(sv2v_cast_F4B48(5'b11000)): key_init_d = key_sideload;
			sv2v_cast_C3037(sv2v_cast_F4B48(5'b00001)): key_init_d = prd_clearing_256;
			default: key_init_d = prd_clearing_256;
		endcase
	end
	function automatic [255:0] sv2v_cast_0C010;
		input reg [255:0] inp;
		sv2v_cast_0C010 = inp;
	endfunction
	function automatic [2:0] sv2v_cast_0397F;
		input reg [2:0] inp;
		sv2v_cast_0397F = inp;
	endfunction
	function automatic [2:0] sv2v_cast_AC049;
		input reg [2:0] inp;
		sv2v_cast_AC049 = inp;
	endfunction
	always @(posedge clk_i or negedge rst_ni) begin : key_init_reg
		if (!rst_ni)
			key_init_q <= {aes_pkg_NumSharesKey {sv2v_cast_0C010(1'sb0)}};
		else begin : sv2v_autoblock_7
			reg signed [31:0] s;
			for (s = 0; s < aes_pkg_NumSharesKey; s = s + 1)
				begin : sv2v_autoblock_8
					reg signed [31:0] i;
					for (i = 0; i < aes_reg_pkg_NumRegsKey; i = i + 1)
						if (key_init_we[s][i * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth] == sv2v_cast_AC049(sv2v_cast_0397F(3'b011)))
							key_init_q[(((1 - s) * aes_reg_pkg_NumRegsKey) + i) * 32+:32] <= key_init_d[(((1 - s) * aes_reg_pkg_NumRegsKey) + i) * 32+:32];
				end
		end
	end
	function automatic [127:0] aes_pkg_aes_transpose;
		input reg [127:0] in;
		reg [127:0] transpose;
		begin
			transpose = 1'sb0;
			begin : sv2v_autoblock_9
				reg signed [31:0] j;
				for (j = 0; j < 4; j = j + 1)
					begin : sv2v_autoblock_10
						reg signed [31:0] i;
						for (i = 0; i < 4; i = i + 1)
							transpose[((i * 4) + j) * 8+:8] = in[((j * 4) + i) * 8+:8];
					end
			end
			aes_pkg_aes_transpose = transpose;
		end
	endfunction
	function automatic [5:0] sv2v_cast_8208B;
		input reg [5:0] inp;
		sv2v_cast_8208B = inp;
	endfunction
	function automatic [5:0] sv2v_cast_23662;
		input reg [5:0] inp;
		sv2v_cast_23662 = inp;
	endfunction
	always @(*) begin : iv_mux
		case (iv_sel)
			sv2v_cast_23662(sv2v_cast_8208B(6'b011101)): iv_d = iv;
			sv2v_cast_23662(sv2v_cast_8208B(6'b110000)): iv_d = data_out_d;
			sv2v_cast_23662(sv2v_cast_8208B(6'b001000)): iv_d = aes_pkg_aes_transpose(state_out);
			sv2v_cast_23662(sv2v_cast_8208B(6'b000011)): iv_d = data_in_prev_q;
			sv2v_cast_23662(sv2v_cast_8208B(6'b111110)): iv_d = ctr;
			sv2v_cast_23662(sv2v_cast_8208B(6'b100101)): iv_d = prd_clearing_128[0];
			default: iv_d = prd_clearing_128[0];
		endcase
	end
	always @(posedge clk_i or negedge rst_ni) begin : iv_reg
		if (!rst_ni)
			iv_q <= 1'sb0;
		else begin : sv2v_autoblock_11
			reg signed [31:0] i;
			for (i = 0; i < aes_pkg_NumSlicesCtr; i = i + 1)
				if (iv_we[i * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth] == sv2v_cast_AC049(sv2v_cast_0397F(3'b011)))
					iv_q[i * aes_pkg_SliceSizeCtr+:aes_pkg_SliceSizeCtr] <= iv_d[i * aes_pkg_SliceSizeCtr+:aes_pkg_SliceSizeCtr];
		end
	end
	function automatic [2:0] sv2v_cast_47617;
		input reg [2:0] inp;
		sv2v_cast_47617 = inp;
	endfunction
	always @(*) begin : data_in_prev_mux
		case (data_in_prev_sel)
			sv2v_cast_47617(sv2v_cast_0397F(3'b011)): data_in_prev_d = data_in;
			sv2v_cast_47617(sv2v_cast_0397F(3'b100)): data_in_prev_d = prd_clearing_128[0];
			default: data_in_prev_d = prd_clearing_128[0];
		endcase
	end
	always @(posedge clk_i or negedge rst_ni) begin : data_in_prev_reg
		if (!rst_ni)
			data_in_prev_q <= 1'sb0;
		else if (data_in_prev_we == sv2v_cast_AC049(sv2v_cast_0397F(3'b011)))
			data_in_prev_q <= data_in_prev_d;
	end
	aes_ctr u_aes_ctr(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.incr_i(ctr_incr),
		.ready_o(ctr_ready),
		.alert_o(ctr_alert),
		.ctr_i(iv_q),
		.ctr_o(ctr),
		.ctr_we_o(ctr_we)
	);
	function automatic [5:0] sv2v_cast_BC361;
		input reg [5:0] inp;
		sv2v_cast_BC361 = inp;
	endfunction
	function automatic [1:0] sv2v_cast_E41EB;
		input reg [1:0] inp;
		sv2v_cast_E41EB = inp;
	endfunction
	assign cipher_op = ((aes_mode_q == sv2v_cast_BC361(6'b000001)) && (aes_op_q == sv2v_cast_E41EB(2'b01)) ? sv2v_cast_E41EB(2'b01) : ((aes_mode_q == sv2v_cast_BC361(6'b000001)) && (aes_op_q == sv2v_cast_E41EB(2'b10)) ? sv2v_cast_E41EB(2'b10) : ((aes_mode_q == sv2v_cast_BC361(6'b000010)) && (aes_op_q == sv2v_cast_E41EB(2'b01)) ? sv2v_cast_E41EB(2'b01) : ((aes_mode_q == sv2v_cast_BC361(6'b000010)) && (aes_op_q == sv2v_cast_E41EB(2'b10)) ? sv2v_cast_E41EB(2'b10) : (aes_mode_q == sv2v_cast_BC361(6'b000100) ? sv2v_cast_E41EB(2'b01) : (aes_mode_q == sv2v_cast_BC361(6'b001000) ? sv2v_cast_E41EB(2'b01) : (aes_mode_q == sv2v_cast_BC361(6'b010000) ? sv2v_cast_E41EB(2'b01) : sv2v_cast_E41EB(2'b01))))))));
	wire [1:0] cipher_op_raw;
	prim_xilinx_buf #(.Width(aes_pkg_AES_OP_WIDTH)) u_prim_xilinx_buf_op(
		.in_i(cipher_op),
		.out_o(cipher_op_raw)
	);
	assign cipher_op_buf = sv2v_cast_E41EB(cipher_op_raw);
	genvar _gv_s_3;
	generate
		for (_gv_s_3 = 0; _gv_s_3 < NumShares; _gv_s_3 = _gv_s_3 + 1) begin : gen_cipher_prd_clearing
			localparam s = _gv_s_3;
			assign cipher_prd_clearing[((NumShares - 1) - s) * aes_pkg_WidthPRDClearing+:aes_pkg_WidthPRDClearing] = prd_clearing[(1 - s) * aes_pkg_WidthPRDClearing+:aes_pkg_WidthPRDClearing];
		end
	endgenerate
	function automatic [2:0] sv2v_cast_1C745;
		input reg [2:0] inp;
		sv2v_cast_1C745 = inp;
	endfunction
	always @(*) begin : state_in_mux
		case (state_in_sel)
			sv2v_cast_1C745(sv2v_cast_0397F(3'b011)): state_in = 1'sb0;
			sv2v_cast_1C745(sv2v_cast_0397F(3'b100)): state_in = aes_pkg_aes_transpose(data_in);
			default: state_in = 1'sb0;
		endcase
	end
	function automatic [2:0] sv2v_cast_9C95F;
		input reg [2:0] inp;
		sv2v_cast_9C95F = inp;
	endfunction
	always @(*) begin : add_state_in_mux
		case (add_state_in_sel)
			sv2v_cast_9C95F(sv2v_cast_0397F(3'b011)): add_state_in = 1'sb0;
			sv2v_cast_9C95F(sv2v_cast_0397F(3'b100)): add_state_in = aes_pkg_aes_transpose(iv_q);
			default: add_state_in = 1'sb0;
		endcase
	end
	generate
		if (!SecMasking) begin : gen_state_init_unmasked
			assign state_init[8 * (4 * ((NumShares - 1) * 4))+:128] = state_in ^ add_state_in;
			wire [127:0] unused_state_mask;
			assign unused_state_mask = state_mask;
		end
		else begin : gen_state_init_masked
			assign state_init[8 * (4 * ((NumShares - 1) * 4))+:128] = (state_in ^ add_state_in) ^ state_mask;
			assign state_init[8 * (4 * ((NumShares - 2) * 4))+:128] = state_mask;
		end
		if (!SecMasking) begin : gen_key_init_unmasked
			assign key_init_cipher[32 * ((NumShares - 1) * aes_reg_pkg_NumRegsKey)+:256] = key_init_q[256+:256] ^ key_init_q[0+:256];
		end
		else begin : gen_key_init_masked
			assign key_init_cipher = key_init_q;
		end
	endgenerate
	aes_cipher_core #(
		.AES192Enable(AES192Enable),
		.SecMasking(SecMasking),
		.SecSBoxImpl(SecSBoxImpl),
		.SecAllowForcingMasks(SecAllowForcingMasks),
		.SecSkipPRNGReseeding(SecSkipPRNGReseeding),
		.RndCnstMaskingLfsrSeed(RndCnstMaskingLfsrSeed),
		.RndCnstMaskingLfsrPerm(RndCnstMaskingLfsrPerm)
	) u_aes_cipher_core(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.in_valid_i(cipher_in_valid),
		.in_ready_o(cipher_in_ready),
		.out_valid_o(cipher_out_valid),
		.out_ready_i(cipher_out_ready),
		.cfg_valid_i(~ctrl_err_storage),
		.op_i(cipher_op_buf),
		.key_len_i(key_len_q),
		.crypt_i(cipher_crypt),
		.crypt_o(cipher_crypt_busy),
		.dec_key_gen_i(cipher_dec_key_gen),
		.dec_key_gen_o(cipher_dec_key_gen_busy),
		.prng_reseed_i(cipher_prng_reseed),
		.prng_reseed_o(cipher_prng_reseed_busy),
		.key_clear_i(cipher_key_clear),
		.key_clear_o(cipher_key_clear_busy),
		.data_out_clear_i(cipher_data_out_clear),
		.data_out_clear_o(cipher_data_out_clear_busy),
		.alert_fatal_i(alert_fatal_o),
		.alert_o(cipher_alert),
		.prd_clearing_i(cipher_prd_clearing),
		.force_masks_i(force_masks),
		.data_in_mask_o(state_mask),
		.entropy_req_o(entropy_masking_req_o),
		.entropy_ack_i(entropy_masking_ack_i),
		.entropy_i(entropy_masking_i),
		.state_init_i(state_init),
		.key_init_i(key_init_cipher),
		.state_o(state_done)
	);
	generate
		if (!SecMasking) begin : gen_state_out_unmasked
			assign state_out = state_done[8 * (4 * ((NumShares - 1) * 4))+:128];
		end
		else begin : gen_state_out_masked
			wire [127:0] state_done_muxed [0:NumShares - 1];
			genvar _gv_s_4;
			for (_gv_s_4 = 0; _gv_s_4 < NumShares; _gv_s_4 = _gv_s_4 + 1) begin : gen_state_done_muxed
				localparam s = _gv_s_4;
				assign state_done_muxed[s] = (cipher_out_valid == sv2v_cast_AC049(sv2v_cast_0397F(3'b011)) ? state_done[8 * (4 * (((NumShares - 1) - s) * 4))+:128] : prd_clearing_128[s]);
			end
			wire [127:0] state_done_buf [0:NumShares - 1];
			prim_xilinx_buf #(.Width(128 * NumShares)) u_prim_state_done_muxed(
				.in_i({state_done_muxed[1], state_done_muxed[0]}),
				.out_o({state_done_buf[1], state_done_buf[0]})
			);
			assign state_out = state_done_buf[0] ^ state_done_buf[1];
		end
	endgenerate
	function automatic [4:0] sv2v_cast_870A5;
		input reg [4:0] inp;
		sv2v_cast_870A5 = inp;
	endfunction
	always @(*) begin : add_state_out_mux
		case (add_state_out_sel)
			sv2v_cast_870A5(sv2v_cast_F4B48(5'b01110)): add_state_out = 1'sb0;
			sv2v_cast_870A5(sv2v_cast_F4B48(5'b11000)): add_state_out = aes_pkg_aes_transpose(iv_q);
			sv2v_cast_870A5(sv2v_cast_F4B48(5'b00001)): add_state_out = aes_pkg_aes_transpose(data_in_prev_q);
			default: add_state_out = 1'sb0;
		endcase
	end
	assign data_out_d = aes_pkg_aes_transpose(state_out ^ add_state_out);
	aes_ctrl_reg_shadowed #(.AES192Enable(AES192Enable)) u_ctrl_reg_shadowed(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.rst_shadowed_ni(rst_shadowed_ni),
		.qe_o(ctrl_qe),
		.we_i(ctrl_we),
		.phase_o(ctrl_phase),
		.operation_o(aes_op_q),
		.mode_o(aes_mode_q),
		.key_len_o(key_len_q),
		.sideload_o(sideload_q),
		.prng_reseed_rate_o(prng_reseed_rate_q),
		.manual_operation_o(manual_operation_q),
		.err_update_o(ctrl_reg_err_update),
		.err_storage_o(ctrl_reg_err_storage),
		.reg2hw_ctrl_i(reg2hw[35-:28]),
		.hw2reg_ctrl_o(hw2reg[37-:16])
	);
	assign key_touch_forces_reseed = reg2hw[7];
	assign force_masks = reg2hw[6];
	aes_control #(
		.SecMasking(SecMasking),
		.SecStartTriggerDelay(SecStartTriggerDelay)
	) u_aes_control(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.ctrl_qe_i(ctrl_qe),
		.ctrl_we_o(ctrl_we),
		.ctrl_phase_i(ctrl_phase),
		.ctrl_err_storage_i(ctrl_err_storage),
		.op_i(aes_op_q),
		.mode_i(aes_mode_q),
		.cipher_op_i(cipher_op_buf),
		.sideload_i(sideload_q),
		.prng_reseed_rate_i(prng_reseed_rate_q),
		.manual_operation_i(manual_operation_q),
		.key_touch_forces_reseed_i(key_touch_forces_reseed),
		.start_i(reg2hw[5]),
		.key_iv_data_in_clear_i(reg2hw[4]),
		.data_out_clear_i(reg2hw[3]),
		.prng_reseed_i(reg2hw[2]),
		.mux_sel_err_i(mux_sel_err),
		.sp_enc_err_i(sp_enc_err_q),
		.lc_escalate_en_i(lc_escalate_en_i),
		.alert_fatal_i(alert_fatal_o),
		.alert_o(ctrl_alert),
		.key_sideload_valid_i(keymgr_key_i[(keymgr_pkg_Shares * keymgr_pkg_KeyWidth) + 0]),
		.key_init_qe_i(key_init_qe_buf),
		.iv_qe_i(iv_qe_buf),
		.data_in_qe_i(data_in_qe_buf),
		.data_out_re_i(data_out_re_buf),
		.data_in_we_o(data_in_we),
		.data_out_we_o(data_out_we_ctrl),
		.data_in_prev_sel_o(data_in_prev_sel_ctrl),
		.data_in_prev_we_o(data_in_prev_we_ctrl),
		.state_in_sel_o(state_in_sel_ctrl),
		.add_state_in_sel_o(add_state_in_sel_ctrl),
		.add_state_out_sel_o(add_state_out_sel_ctrl),
		.ctr_incr_o(ctr_incr),
		.ctr_ready_i(ctr_ready),
		.ctr_we_i(ctr_we),
		.cipher_in_valid_o(cipher_in_valid),
		.cipher_in_ready_i(cipher_in_ready),
		.cipher_out_valid_i(cipher_out_valid),
		.cipher_out_ready_o(cipher_out_ready),
		.cipher_crypt_o(cipher_crypt),
		.cipher_crypt_i(cipher_crypt_busy),
		.cipher_dec_key_gen_o(cipher_dec_key_gen),
		.cipher_dec_key_gen_i(cipher_dec_key_gen_busy),
		.cipher_prng_reseed_o(cipher_prng_reseed),
		.cipher_prng_reseed_i(cipher_prng_reseed_busy),
		.cipher_key_clear_o(cipher_key_clear),
		.cipher_key_clear_i(cipher_key_clear_busy),
		.cipher_data_out_clear_o(cipher_data_out_clear),
		.cipher_data_out_clear_i(cipher_data_out_clear_busy),
		.key_init_sel_o(key_init_sel_ctrl),
		.key_init_we_o(key_init_we_ctrl),
		.iv_sel_o(iv_sel_ctrl),
		.iv_we_o(iv_we_ctrl),
		.prng_data_req_o(prd_clearing_upd_req),
		.prng_data_ack_i(prd_clearing_upd_ack),
		.prng_reseed_req_o(prd_clearing_rsd_req),
		.prng_reseed_ack_i(prd_clearing_rsd_ack),
		.start_o(hw2reg[21]),
		.start_we_o(hw2reg[20]),
		.key_iv_data_in_clear_o(hw2reg[19]),
		.key_iv_data_in_clear_we_o(hw2reg[18]),
		.data_out_clear_o(hw2reg[17]),
		.data_out_clear_we_o(hw2reg[16]),
		.prng_reseed_o(hw2reg[15]),
		.prng_reseed_we_o(hw2reg[14]),
		.idle_o(hw2reg[13]),
		.idle_we_o(hw2reg[12]),
		.stall_o(hw2reg[11]),
		.stall_we_o(hw2reg[10]),
		.output_lost_i(reg2hw[0]),
		.output_lost_o(hw2reg[9]),
		.output_lost_we_o(hw2reg[8]),
		.output_valid_o(hw2reg[7]),
		.output_valid_we_o(hw2reg[6]),
		.input_ready_o(hw2reg[5]),
		.input_ready_we_o(hw2reg[4])
	);
	always @(*) begin : data_in_reg_clear
		begin : sv2v_autoblock_12
			reg signed [31:0] i;
			for (i = 0; i < aes_reg_pkg_NumRegsData; i = i + 1)
				begin
					hw2reg[166 + ((i * 33) + 32)-:32] = prd_clearing_128[0][i * 32+:32];
					hw2reg[166 + (i * 33)] = data_in_we;
				end
		end
	end
	localparam signed [31:0] aes_pkg_DIPSelNum = 2;
	aes_sel_buf_chk #(
		.Num(aes_pkg_DIPSelNum),
		.Width(aes_pkg_DIPSelWidth),
		.EnSecBuf(1'b1)
	) u_aes_data_in_prev_sel_buf_chk(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.sel_i(data_in_prev_sel_ctrl),
		.sel_o(data_in_prev_sel_raw),
		.err_o(data_in_prev_sel_err)
	);
	assign data_in_prev_sel = sv2v_cast_47617(data_in_prev_sel_raw);
	localparam signed [31:0] aes_pkg_SISelNum = 2;
	aes_sel_buf_chk #(
		.Num(aes_pkg_SISelNum),
		.Width(aes_pkg_SISelWidth),
		.EnSecBuf(1'b1)
	) u_aes_state_in_sel_buf_chk(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.sel_i(state_in_sel_ctrl),
		.sel_o(state_in_sel_raw),
		.err_o(state_in_sel_err)
	);
	assign state_in_sel = sv2v_cast_1C745(state_in_sel_raw);
	localparam signed [31:0] aes_pkg_AddSISelNum = 2;
	aes_sel_buf_chk #(
		.Num(aes_pkg_AddSISelNum),
		.Width(aes_pkg_AddSISelWidth),
		.EnSecBuf(1'b1)
	) u_aes_add_state_in_sel_buf_chk(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.sel_i(add_state_in_sel_ctrl),
		.sel_o(add_state_in_sel_raw),
		.err_o(add_state_in_sel_err)
	);
	assign add_state_in_sel = sv2v_cast_9C95F(add_state_in_sel_raw);
	localparam signed [31:0] aes_pkg_AddSOSelNum = 3;
	aes_sel_buf_chk #(
		.Num(aes_pkg_AddSOSelNum),
		.Width(aes_pkg_AddSOSelWidth),
		.EnSecBuf(1'b1)
	) u_aes_add_state_out_sel_buf_chk(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.sel_i(add_state_out_sel_ctrl),
		.sel_o(add_state_out_sel_raw),
		.err_o(add_state_out_sel_err)
	);
	assign add_state_out_sel = sv2v_cast_870A5(add_state_out_sel_raw);
	localparam signed [31:0] aes_pkg_KeyInitSelNum = 3;
	aes_sel_buf_chk #(
		.Num(aes_pkg_KeyInitSelNum),
		.Width(aes_pkg_KeyInitSelWidth),
		.EnSecBuf(1'b1)
	) u_aes_key_init_sel_buf_chk(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.sel_i(key_init_sel_ctrl),
		.sel_o(key_init_sel_raw),
		.err_o(key_init_sel_err)
	);
	assign key_init_sel = sv2v_cast_C3037(key_init_sel_raw);
	localparam signed [31:0] aes_pkg_IVSelNum = 6;
	aes_sel_buf_chk #(
		.Num(aes_pkg_IVSelNum),
		.Width(aes_pkg_IVSelWidth),
		.EnSecBuf(1'b1)
	) u_aes_iv_sel_buf_chk(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.sel_i(iv_sel_ctrl),
		.sel_o(iv_sel_raw),
		.err_o(iv_sel_err)
	);
	assign iv_sel = sv2v_cast_23662(iv_sel_raw);
	assign mux_sel_err = ((((data_in_prev_sel_err | state_in_sel_err) | add_state_in_sel_err) | add_state_out_sel_err) | key_init_sel_err) | iv_sel_err;
	localparam [31:0] NumSp2VSig = ((aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) + aes_pkg_NumSlicesCtr) + 2;
	wire [(NumSp2VSig * aes_pkg_Sp2VWidth) - 1:0] sp2v_sig;
	wire [(NumSp2VSig * aes_pkg_Sp2VWidth) - 1:0] sp2v_sig_chk;
	wire [(NumSp2VSig * aes_pkg_Sp2VWidth) - 1:0] sp2v_sig_chk_raw;
	wire [NumSp2VSig - 1:0] sp2v_sig_err;
	genvar _gv_s_5;
	generate
		for (_gv_s_5 = 0; _gv_s_5 < aes_pkg_NumSharesKey; _gv_s_5 = _gv_s_5 + 1) begin : gen_use_key_init_we_ctrl_shares
			localparam s = _gv_s_5;
			genvar _gv_i_1;
			for (_gv_i_1 = 0; _gv_i_1 < aes_reg_pkg_NumRegsKey; _gv_i_1 = _gv_i_1 + 1) begin : gen_use_key_init_we_ctrl
				localparam i = _gv_i_1;
				assign sp2v_sig[((s * aes_reg_pkg_NumRegsKey) + i) * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth] = key_init_we_ctrl[(((1 - s) * aes_reg_pkg_NumRegsKey) + i) * 3+:3];
			end
		end
	endgenerate
	genvar _gv_i_2;
	generate
		for (_gv_i_2 = 0; _gv_i_2 < aes_pkg_NumSlicesCtr; _gv_i_2 = _gv_i_2 + 1) begin : gen_use_iv_we_ctrl
			localparam i = _gv_i_2;
			assign sp2v_sig[((aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) + i) * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth] = iv_we_ctrl[i * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth];
		end
	endgenerate
	assign sp2v_sig[(((aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) + aes_pkg_NumSlicesCtr) + 0) * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth] = data_in_prev_we_ctrl;
	assign sp2v_sig[(((aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) + aes_pkg_NumSlicesCtr) + 1) * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth] = data_out_we_ctrl;
	localparam [NumSp2VSig - 1:0] Sp2VEnSecBuf = {NumSp2VSig {1'b1}};
	genvar _gv_i_3;
	localparam signed [31:0] aes_pkg_Sp2VNum = 2;
	generate
		for (_gv_i_3 = 0; _gv_i_3 < NumSp2VSig; _gv_i_3 = _gv_i_3 + 1) begin : gen_sel_buf_chk
			localparam i = _gv_i_3;
			aes_sel_buf_chk #(
				.Num(aes_pkg_Sp2VNum),
				.Width(aes_pkg_Sp2VWidth),
				.EnSecBuf(Sp2VEnSecBuf[i])
			) u_aes_sp2v_sig_buf_chk_i(
				.clk_i(clk_i),
				.rst_ni(rst_ni),
				.sel_i(sp2v_sig[i * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth]),
				.sel_o(sp2v_sig_chk_raw[i * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth]),
				.err_o(sp2v_sig_err[i])
			);
			assign sp2v_sig_chk[i * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth] = sv2v_cast_AC049(sp2v_sig_chk_raw[i * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth]);
		end
	endgenerate
	genvar _gv_s_6;
	generate
		for (_gv_s_6 = 0; _gv_s_6 < aes_pkg_NumSharesKey; _gv_s_6 = _gv_s_6 + 1) begin : gen_key_init_we_shares
			localparam s = _gv_s_6;
			genvar _gv_i_4;
			for (_gv_i_4 = 0; _gv_i_4 < aes_reg_pkg_NumRegsKey; _gv_i_4 = _gv_i_4 + 1) begin : gen_key_init_we
				localparam i = _gv_i_4;
				assign key_init_we[s][i * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth] = sp2v_sig_chk[((s * aes_reg_pkg_NumRegsKey) + i) * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth];
			end
		end
	endgenerate
	genvar _gv_i_5;
	generate
		for (_gv_i_5 = 0; _gv_i_5 < aes_pkg_NumSlicesCtr; _gv_i_5 = _gv_i_5 + 1) begin : gen_iv_we
			localparam i = _gv_i_5;
			assign iv_we[i * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth] = sp2v_sig_chk[((aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) + i) * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth];
		end
	endgenerate
	assign data_in_prev_we = sp2v_sig_chk[(((aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) + aes_pkg_NumSlicesCtr) + 0) * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth];
	assign data_out_we = sp2v_sig_chk[(((aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) + aes_pkg_NumSlicesCtr) + 1) * aes_pkg_Sp2VWidth+:aes_pkg_Sp2VWidth];
	assign sp_enc_err_d = |sp2v_sig_err;
	always @(posedge clk_i or negedge rst_ni) begin : reg_sp_enc_err
		if (!rst_ni)
			sp_enc_err_q <= 1'b0;
		else if (sp_enc_err_d)
			sp_enc_err_q <= 1'b1;
	end
	always @(posedge clk_i or negedge rst_ni) begin : data_out_reg
		if (!rst_ni)
			data_out_q <= 1'sb0;
		else if (data_out_we == sv2v_cast_AC049(sv2v_cast_0397F(3'b011)))
			data_out_q <= data_out_d;
	end
	always @(*) begin : key_reg_put
		begin : sv2v_autoblock_13
			reg signed [31:0] i;
			for (i = 0; i < aes_reg_pkg_NumRegsKey; i = i + 1)
				begin
					hw2reg[682 + ((i * 32) + 31)-:32] = key_init_q[(8 + i) * 32+:32];
					hw2reg[426 + ((i * 32) + 31)-:32] = key_init_q[(0 + i) * 32+:32];
				end
		end
	end
	always @(*) begin : iv_reg_put
		begin : sv2v_autoblock_14
			reg signed [31:0] i;
			for (i = 0; i < aes_reg_pkg_NumRegsIv; i = i + 1)
				hw2reg[298 + ((i * 32) + 31)-:32] = {iv_q[((2 * i) + 1) * aes_pkg_SliceSizeCtr+:aes_pkg_SliceSizeCtr], iv_q[(2 * i) * aes_pkg_SliceSizeCtr+:aes_pkg_SliceSizeCtr]};
		end
	end
	always @(*) begin : data_out_put
		begin : sv2v_autoblock_15
			reg signed [31:0] i;
			for (i = 0; i < aes_reg_pkg_NumRegsData; i = i + 1)
				hw2reg[38 + ((i * 32) + 31)-:32] = data_out_q[i * 32+:32];
		end
	end
	localparam [0:0] aes_pkg_ClearStatusOnFatalAlert = 1'b0;
	assign clear_on_fatal = (aes_pkg_ClearStatusOnFatalAlert ? alert_fatal_o : 1'b0);
	assign ctrl_err_update = ctrl_reg_err_update | shadowed_update_err_i;
	assign alert_recov_o = ctrl_err_update;
	wire [1:1] sv2v_tmp_8E257;
	assign sv2v_tmp_8E257 = ctrl_err_update & ~clear_on_fatal;
	always @(*) hw2reg[3] = sv2v_tmp_8E257;
	wire [1:1] sv2v_tmp_1A9FA;
	assign sv2v_tmp_1A9FA = (ctrl_err_update | ctrl_we) | clear_on_fatal;
	always @(*) hw2reg[2] = sv2v_tmp_1A9FA;
	assign ctrl_err_storage_d = ctrl_reg_err_storage | shadowed_storage_err_i;
	always @(posedge clk_i or negedge rst_ni) begin : ctrl_err_storage_reg
		if (!rst_ni)
			ctrl_err_storage_q <= 1'b0;
		else if (ctrl_err_storage_d)
			ctrl_err_storage_q <= 1'b1;
	end
	assign ctrl_err_storage = ctrl_err_storage_d | ctrl_err_storage_q;
	assign alert_fatal_o = (((ctrl_err_storage | ctr_alert) | cipher_alert) | ctrl_alert) | intg_err_alert_i;
	wire [1:1] sv2v_tmp_9AF19;
	assign sv2v_tmp_9AF19 = alert_fatal_o;
	always @(*) hw2reg[1] = sv2v_tmp_9AF19;
	wire [1:1] sv2v_tmp_81A02;
	assign sv2v_tmp_81A02 = alert_fatal_o;
	always @(*) hw2reg[0] = sv2v_tmp_81A02;
	wire unused_alert_signals;
	assign unused_alert_signals = ^reg2hw[963-:4];
	wire unused_idle;
	assign unused_idle = reg2hw[1];
	localparam signed [31:0] AesCoreSecMaskingNonDefault = (SecMasking == 1 ? 1 : 2);
	function automatic [AesCoreSecMaskingNonDefault - 1:0] sv2v_cast_E842C;
		input reg [AesCoreSecMaskingNonDefault - 1:0] inp;
		sv2v_cast_E842C = inp;
	endfunction
	always @(*) begin : sv2v_autoblock_16
		reg unused_assert_static_lint_error;
		unused_assert_static_lint_error = sv2v_cast_E842C(1'b1);
	end
	wire [127:0] state_done_transposed;
	wire [127:0] unused_state_done_transposed;
	generate
		if (!SecMasking) begin : gen_state_done_transposed_unmasked
			assign state_done_transposed = aes_pkg_aes_transpose(state_done[8 * (4 * ((NumShares - 1) * 4))+:128]);
		end
		else begin : gen_state_done_transposed_masked
			assign state_done_transposed = aes_pkg_aes_transpose(state_done[8 * (4 * ((NumShares - 1) * 4))+:128] ^ state_done[8 * (4 * ((NumShares - 2) * 4))+:128]);
		end
	endgenerate
	assign unused_state_done_transposed = state_done_transposed;
endmodule

module aes_control_fsm (
	clk_i,
	rst_ni,
	ctrl_qe_i,
	ctrl_we_o,
	ctrl_phase_i,
	ctrl_err_storage_i,
	op_i,
	mode_i,
	cipher_op_i,
	sideload_i,
	prng_reseed_rate_i,
	manual_operation_i,
	key_touch_forces_reseed_i,
	start_i,
	key_iv_data_in_clear_i,
	data_out_clear_i,
	prng_reseed_i,
	mux_sel_err_i,
	sp_enc_err_i,
	lc_escalate_en_i,
	alert_fatal_i,
	alert_o,
	key_sideload_valid_i,
	key_init_qe_i,
	iv_qe_i,
	data_in_qe_i,
	data_out_re_i,
	data_in_we_o,
	data_out_we_o,
	data_in_prev_sel_o,
	data_in_prev_we_o,
	state_in_sel_o,
	add_state_in_sel_o,
	add_state_out_sel_o,
	ctr_incr_o,
	ctr_ready_i,
	ctr_we_i,
	cipher_in_valid_o,
	cipher_in_ready_i,
	cipher_out_valid_i,
	cipher_out_ready_o,
	cipher_crypt_o,
	cipher_crypt_i,
	cipher_dec_key_gen_o,
	cipher_dec_key_gen_i,
	cipher_prng_reseed_o,
	cipher_prng_reseed_i,
	cipher_key_clear_o,
	cipher_key_clear_i,
	cipher_data_out_clear_o,
	cipher_data_out_clear_i,
	key_init_sel_o,
	key_init_we_o,
	iv_sel_o,
	iv_we_o,
	prng_data_req_o,
	prng_data_ack_i,
	prng_reseed_req_o,
	prng_reseed_ack_i,
	start_we_o,
	key_iv_data_in_clear_we_o,
	data_out_clear_we_o,
	prng_reseed_o,
	prng_reseed_we_o,
	idle_o,
	idle_we_o,
	stall_o,
	stall_we_o,
	output_lost_i,
	output_lost_o,
	output_lost_we_o,
	output_valid_o,
	output_valid_we_o,
	input_ready_o,
	input_ready_we_o
);
	parameter [0:0] SecMasking = 0;
	input wire clk_i;
	input wire rst_ni;
	input wire ctrl_qe_i;
	output reg ctrl_we_o;
	input wire ctrl_phase_i;
	input wire ctrl_err_storage_i;
	localparam signed [31:0] aes_pkg_AES_OP_WIDTH = 2;
	input wire [1:0] op_i;
	localparam signed [31:0] aes_pkg_AES_MODE_WIDTH = 6;
	input wire [5:0] mode_i;
	input wire [1:0] cipher_op_i;
	input wire sideload_i;
	localparam signed [31:0] aes_pkg_AES_PRNGRESEEDRATE_WIDTH = 3;
	input wire [2:0] prng_reseed_rate_i;
	input wire manual_operation_i;
	input wire key_touch_forces_reseed_i;
	input wire start_i;
	input wire key_iv_data_in_clear_i;
	input wire data_out_clear_i;
	input wire prng_reseed_i;
	input wire mux_sel_err_i;
	input wire sp_enc_err_i;
	localparam signed [31:0] lc_ctrl_pkg_TxWidth = 4;
	input wire [3:0] lc_escalate_en_i;
	input wire alert_fatal_i;
	output reg alert_o;
	input wire key_sideload_valid_i;
	localparam [31:0] aes_pkg_NumSharesKey = 2;
	localparam signed [31:0] aes_reg_pkg_NumRegsKey = 8;
	input wire [(aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) - 1:0] key_init_qe_i;
	localparam signed [31:0] aes_reg_pkg_NumRegsIv = 4;
	input wire [3:0] iv_qe_i;
	localparam signed [31:0] aes_reg_pkg_NumRegsData = 4;
	input wire [3:0] data_in_qe_i;
	input wire [3:0] data_out_re_i;
	output reg data_in_we_o;
	output reg data_out_we_o;
	localparam signed [31:0] aes_pkg_Mux2SelWidth = 3;
	localparam signed [31:0] aes_pkg_DIPSelWidth = aes_pkg_Mux2SelWidth;
	output reg [2:0] data_in_prev_sel_o;
	output reg data_in_prev_we_o;
	localparam signed [31:0] aes_pkg_SISelWidth = aes_pkg_Mux2SelWidth;
	output reg [2:0] state_in_sel_o;
	localparam signed [31:0] aes_pkg_AddSISelWidth = aes_pkg_Mux2SelWidth;
	output reg [2:0] add_state_in_sel_o;
	localparam signed [31:0] aes_pkg_Mux3SelWidth = 5;
	localparam signed [31:0] aes_pkg_AddSOSelWidth = aes_pkg_Mux3SelWidth;
	output reg [4:0] add_state_out_sel_o;
	output reg ctr_incr_o;
	input wire ctr_ready_i;
	localparam [31:0] aes_pkg_SliceSizeCtr = 16;
	localparam [31:0] aes_pkg_NumSlicesCtr = 8;
	input wire [7:0] ctr_we_i;
	output reg cipher_in_valid_o;
	input wire cipher_in_ready_i;
	input wire cipher_out_valid_i;
	output reg cipher_out_ready_o;
	output reg cipher_crypt_o;
	input wire cipher_crypt_i;
	output reg cipher_dec_key_gen_o;
	input wire cipher_dec_key_gen_i;
	output reg cipher_prng_reseed_o;
	input wire cipher_prng_reseed_i;
	output reg cipher_key_clear_o;
	input wire cipher_key_clear_i;
	output reg cipher_data_out_clear_o;
	input wire cipher_data_out_clear_i;
	localparam signed [31:0] aes_pkg_KeyInitSelWidth = aes_pkg_Mux3SelWidth;
	output reg [4:0] key_init_sel_o;
	output reg [(aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey) - 1:0] key_init_we_o;
	localparam signed [31:0] aes_pkg_Mux6SelWidth = 6;
	localparam signed [31:0] aes_pkg_IVSelWidth = aes_pkg_Mux6SelWidth;
	output reg [5:0] iv_sel_o;
	output reg [7:0] iv_we_o;
	output reg prng_data_req_o;
	input wire prng_data_ack_i;
	output reg prng_reseed_req_o;
	input wire prng_reseed_ack_i;
	output wire start_we_o;
	output wire key_iv_data_in_clear_we_o;
	output wire data_out_clear_we_o;
	output wire prng_reseed_o;
	output wire prng_reseed_we_o;
	output wire idle_o;
	output wire idle_we_o;
	output wire stall_o;
	output wire stall_we_o;
	input wire output_lost_i;
	output wire output_lost_o;
	output wire output_lost_we_o;
	output wire output_valid_o;
	output wire output_valid_we_o;
	output wire input_ready_o;
	output wire input_ready_we_o;
	localparam signed [31:0] aes_pkg_CtrlStateWidth = 6;
	reg [5:0] aes_ctrl_ns;
	wire [5:0] aes_ctrl_cs;
	reg prng_reseed_done_d;
	reg prng_reseed_done_q;
	reg key_init_clear;
	wire key_init_new;
	wire key_init_new_pulse;
	reg key_init_load;
	reg key_init_arm;
	wire key_init_ready;
	wire key_sideload;
	wire [7:0] iv_qe;
	reg iv_clear;
	reg iv_load;
	reg iv_arm;
	wire iv_ready;
	wire [3:0] data_in_new_d;
	reg [3:0] data_in_new_q;
	wire data_in_new;
	reg data_in_load;
	wire [3:0] data_out_read_d;
	reg [3:0] data_out_read_q;
	wire data_out_read;
	reg output_valid_q;
	wire cfg_valid;
	wire no_alert;
	wire cipher_op_err;
	wire start_common;
	wire start_ecb;
	wire start_cbc;
	wire start_cfb;
	wire start_ofb;
	wire start_ctr;
	wire start;
	reg start_core;
	wire finish;
	wire crypt;
	reg cipher_out_done;
	wire doing_cbc_enc;
	wire doing_cbc_dec;
	wire doing_cfb_enc;
	wire doing_cfb_dec;
	wire doing_ofb;
	wire doing_ctr;
	reg ctrl_we_q;
	wire clear_in_out_status;
	wire clear_on_fatal;
	reg start_we;
	reg key_iv_data_in_clear_we;
	reg data_out_clear_we;
	reg prng_reseed_we;
	reg idle;
	reg idle_we;
	reg stall;
	reg stall_we;
	wire output_lost;
	wire output_lost_we;
	wire output_valid;
	wire output_valid_we;
	wire input_ready;
	wire input_ready_we;
	wire block_ctr_expr;
	reg block_ctr_decr;
	assign iv_qe = {iv_qe_i[3], iv_qe_i[3], iv_qe_i[2], iv_qe_i[2], iv_qe_i[1], iv_qe_i[1], iv_qe_i[0], iv_qe_i[0]};
	function automatic [5:0] sv2v_cast_BC361;
		input reg [5:0] inp;
		sv2v_cast_BC361 = inp;
	endfunction
	assign cfg_valid = ~((mode_i == sv2v_cast_BC361(6'b100000)) | ctrl_err_storage_i);
	assign no_alert = ~alert_fatal_i;
	function automatic [1:0] sv2v_cast_E41EB;
		input reg [1:0] inp;
		sv2v_cast_E41EB = inp;
	endfunction
	assign cipher_op_err = ~((cipher_op_i == sv2v_cast_E41EB(2'b01)) || (cipher_op_i == sv2v_cast_E41EB(2'b10)));
	assign start_common = (key_init_ready & data_in_new) & (sideload_i ? key_sideload_valid_i : 1'b1);
	assign start_ecb = mode_i == sv2v_cast_BC361(6'b000001);
	assign start_cbc = (mode_i == sv2v_cast_BC361(6'b000010)) & iv_ready;
	assign start_cfb = (mode_i == sv2v_cast_BC361(6'b000100)) & iv_ready;
	assign start_ofb = (mode_i == sv2v_cast_BC361(6'b001000)) & iv_ready;
	assign start_ctr = ((mode_i == sv2v_cast_BC361(6'b010000)) & iv_ready) & ctr_ready_i;
	assign start = (cfg_valid & no_alert) & (manual_operation_i ? start_i : ((((start_ecb | start_cbc) | start_cfb) | start_ofb) | start_ctr) & start_common);
	assign finish = (cfg_valid & no_alert) & (manual_operation_i ? 1'b1 : ~output_valid_q | data_out_read);
	assign crypt = cipher_crypt_o | cipher_crypt_i;
	assign doing_cbc_enc = ((mode_i == sv2v_cast_BC361(6'b000010)) && (op_i == sv2v_cast_E41EB(2'b01))) & crypt;
	assign doing_cbc_dec = ((mode_i == sv2v_cast_BC361(6'b000010)) && (op_i == sv2v_cast_E41EB(2'b10))) & crypt;
	assign doing_cfb_enc = ((mode_i == sv2v_cast_BC361(6'b000100)) && (op_i == sv2v_cast_E41EB(2'b01))) & crypt;
	assign doing_cfb_dec = ((mode_i == sv2v_cast_BC361(6'b000100)) && (op_i == sv2v_cast_E41EB(2'b10))) & crypt;
	assign doing_ofb = (mode_i == sv2v_cast_BC361(6'b001000)) & crypt;
	assign doing_ctr = (mode_i == sv2v_cast_BC361(6'b010000)) & crypt;
	function automatic [3:0] sv2v_cast_144AA;
		input reg [3:0] inp;
		sv2v_cast_144AA = inp;
	endfunction
	function automatic lc_ctrl_pkg_lc_tx_test_true_loose;
		input reg [3:0] val;
		lc_ctrl_pkg_lc_tx_test_true_loose = sv2v_cast_144AA(4'b1010) != val;
	endfunction
	function automatic [2:0] sv2v_cast_0397F;
		input reg [2:0] inp;
		sv2v_cast_0397F = inp;
	endfunction
	function automatic [2:0] sv2v_cast_47617;
		input reg [2:0] inp;
		sv2v_cast_47617 = inp;
	endfunction
	function automatic [2:0] sv2v_cast_1C745;
		input reg [2:0] inp;
		sv2v_cast_1C745 = inp;
	endfunction
	function automatic [2:0] sv2v_cast_9C95F;
		input reg [2:0] inp;
		sv2v_cast_9C95F = inp;
	endfunction
	function automatic [4:0] sv2v_cast_F4B48;
		input reg [4:0] inp;
		sv2v_cast_F4B48 = inp;
	endfunction
	function automatic [4:0] sv2v_cast_870A5;
		input reg [4:0] inp;
		sv2v_cast_870A5 = inp;
	endfunction
	function automatic [4:0] sv2v_cast_C3037;
		input reg [4:0] inp;
		sv2v_cast_C3037 = inp;
	endfunction
	function automatic [5:0] sv2v_cast_8208B;
		input reg [5:0] inp;
		sv2v_cast_8208B = inp;
	endfunction
	function automatic [5:0] sv2v_cast_23662;
		input reg [5:0] inp;
		sv2v_cast_23662 = inp;
	endfunction
	function automatic [5:0] sv2v_cast_9BBFB;
		input reg [5:0] inp;
		sv2v_cast_9BBFB = inp;
	endfunction
	always @(*) begin : aes_ctrl_fsm
		data_in_prev_sel_o = sv2v_cast_47617(sv2v_cast_0397F(3'b100));
		data_in_prev_we_o = 1'b0;
		state_in_sel_o = sv2v_cast_1C745(sv2v_cast_0397F(3'b100));
		add_state_in_sel_o = sv2v_cast_9C95F(sv2v_cast_0397F(3'b011));
		add_state_out_sel_o = sv2v_cast_870A5(sv2v_cast_F4B48(5'b01110));
		ctr_incr_o = 1'b0;
		cipher_in_valid_o = 1'b0;
		cipher_out_ready_o = 1'b0;
		cipher_out_done = 1'b0;
		cipher_crypt_o = 1'b0;
		cipher_dec_key_gen_o = 1'b0;
		cipher_prng_reseed_o = 1'b0;
		cipher_key_clear_o = 1'b0;
		cipher_data_out_clear_o = 1'b0;
		key_init_sel_o = (sideload_i ? sv2v_cast_C3037(sv2v_cast_F4B48(5'b11000)) : sv2v_cast_C3037(sv2v_cast_F4B48(5'b01110)));
		key_init_we_o = {aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey {1'b0}};
		iv_sel_o = sv2v_cast_23662(sv2v_cast_8208B(6'b011101));
		iv_we_o = {aes_pkg_NumSlicesCtr {1'b0}};
		ctrl_we_o = 1'b0;
		alert_o = 1'b0;
		prng_data_req_o = 1'b0;
		prng_reseed_req_o = 1'b0;
		start_we = 1'b0;
		key_iv_data_in_clear_we = 1'b0;
		data_out_clear_we = 1'b0;
		prng_reseed_we = 1'b0;
		idle = 1'b0;
		idle_we = 1'b0;
		stall = 1'b0;
		stall_we = 1'b0;
		data_in_load = 1'b0;
		data_in_we_o = 1'b0;
		data_out_we_o = 1'b0;
		key_init_clear = 1'b0;
		key_init_load = 1'b0;
		key_init_arm = 1'b0;
		iv_clear = 1'b0;
		iv_load = 1'b0;
		iv_arm = 1'b0;
		block_ctr_decr = 1'b0;
		aes_ctrl_ns = aes_ctrl_cs;
		start_core = 1'b0;
		prng_reseed_done_d = prng_reseed_done_q | prng_reseed_ack_i;
		case (aes_ctrl_cs)
			sv2v_cast_9BBFB(6'b001001): begin
				start_core = ((start | key_iv_data_in_clear_i) | data_out_clear_i) | prng_reseed_i;
				idle = ~(start_core | (prng_reseed_o & prng_reseed_we_o));
				idle_we = 1'b1;
				start_we = start_i & ((mode_i == sv2v_cast_BC361(6'b100000)) | ~manual_operation_i);
				if (!start_core) begin
					key_init_we_o = (sideload_i ? {aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey {key_sideload}} : key_init_qe_i);
					iv_we_o = iv_qe;
					ctrl_we_o = (!ctrl_err_storage_i ? ctrl_qe_i : 1'b0);
					key_init_clear = ctrl_we_o;
					iv_clear = ctrl_we_o;
				end
				if (prng_reseed_i) begin
					if (!SecMasking) begin
						prng_reseed_done_d = 1'b0;
						aes_ctrl_ns = sv2v_cast_9BBFB(6'b010000);
					end
					else begin
						cipher_prng_reseed_o = 1'b1;
						cipher_in_valid_o = 1'b1;
						if (cipher_in_ready_i) begin
							prng_reseed_done_d = 1'b0;
							aes_ctrl_ns = sv2v_cast_9BBFB(6'b010000);
						end
					end
				end
				else if (key_iv_data_in_clear_i || data_out_clear_i)
					aes_ctrl_ns = sv2v_cast_9BBFB(6'b111101);
				else if (start) begin
					cipher_crypt_o = 1'b1;
					cipher_prng_reseed_o = block_ctr_expr;
					cipher_dec_key_gen_o = (cipher_op_i == sv2v_cast_E41EB(2'b10) ? key_init_new : 1'b0);
					data_in_prev_sel_o = (doing_cbc_dec ? sv2v_cast_47617(sv2v_cast_0397F(3'b011)) : (doing_cfb_enc ? sv2v_cast_47617(sv2v_cast_0397F(3'b011)) : (doing_cfb_dec ? sv2v_cast_47617(sv2v_cast_0397F(3'b011)) : (doing_ofb ? sv2v_cast_47617(sv2v_cast_0397F(3'b011)) : (doing_ctr ? sv2v_cast_47617(sv2v_cast_0397F(3'b011)) : sv2v_cast_47617(sv2v_cast_0397F(3'b100)))))));
					data_in_prev_we_o = (((doing_cbc_dec | doing_cfb_enc) | doing_cfb_dec) | doing_ofb) | doing_ctr;
					state_in_sel_o = (doing_cfb_enc ? sv2v_cast_1C745(sv2v_cast_0397F(3'b011)) : (doing_cfb_dec ? sv2v_cast_1C745(sv2v_cast_0397F(3'b011)) : (doing_ofb ? sv2v_cast_1C745(sv2v_cast_0397F(3'b011)) : (doing_ctr ? sv2v_cast_1C745(sv2v_cast_0397F(3'b011)) : sv2v_cast_1C745(sv2v_cast_0397F(3'b100))))));
					add_state_in_sel_o = (doing_cbc_enc ? sv2v_cast_9C95F(sv2v_cast_0397F(3'b100)) : (doing_cfb_enc ? sv2v_cast_9C95F(sv2v_cast_0397F(3'b100)) : (doing_cfb_dec ? sv2v_cast_9C95F(sv2v_cast_0397F(3'b100)) : (doing_ofb ? sv2v_cast_9C95F(sv2v_cast_0397F(3'b100)) : (doing_ctr ? sv2v_cast_9C95F(sv2v_cast_0397F(3'b100)) : sv2v_cast_9C95F(sv2v_cast_0397F(3'b011)))))));
					cipher_in_valid_o = 1'b1;
					if (cipher_in_ready_i) begin
						start_we = ~cipher_dec_key_gen_o;
						aes_ctrl_ns = sv2v_cast_9BBFB(6'b100011);
					end
				end
			end
			sv2v_cast_9BBFB(6'b100011): begin
				key_init_load = cipher_dec_key_gen_i;
				key_init_arm = ~cipher_dec_key_gen_i;
				iv_load = ~cipher_dec_key_gen_i & (((((doing_cbc_enc | doing_cbc_dec) | doing_cfb_enc) | doing_cfb_dec) | doing_ofb) | doing_ctr);
				data_in_load = ~cipher_dec_key_gen_i;
				ctr_incr_o = doing_ctr;
				aes_ctrl_ns = (!cipher_dec_key_gen_i ? sv2v_cast_9BBFB(6'b111101) : sv2v_cast_9BBFB(6'b100100));
			end
			sv2v_cast_9BBFB(6'b111101): begin
				iv_sel_o = (doing_ctr ? sv2v_cast_23662(sv2v_cast_8208B(6'b111110)) : sv2v_cast_23662(sv2v_cast_8208B(6'b011101)));
				iv_we_o = (doing_ctr ? ctr_we_i : {aes_pkg_NumSlicesCtr {1'b0}});
				prng_data_req_o = 1'b1;
				if (prng_data_ack_i) begin
					if (cipher_crypt_i)
						aes_ctrl_ns = sv2v_cast_9BBFB(6'b100100);
					else if (key_iv_data_in_clear_i || data_out_clear_i) begin
						cipher_key_clear_o = key_iv_data_in_clear_i;
						cipher_data_out_clear_o = data_out_clear_i;
						cipher_in_valid_o = 1'b1;
						if (cipher_in_ready_i)
							aes_ctrl_ns = sv2v_cast_9BBFB(6'b111010);
					end
					else
						aes_ctrl_ns = sv2v_cast_9BBFB(6'b001001);
				end
			end
			sv2v_cast_9BBFB(6'b010000): begin
				prng_reseed_req_o = ~prng_reseed_done_q;
				if (!SecMasking) begin
					if (prng_reseed_done_q) begin
						prng_reseed_we = 1'b1;
						prng_reseed_done_d = 1'b0;
						aes_ctrl_ns = sv2v_cast_9BBFB(6'b001001);
					end
				end
				else begin
					cipher_out_ready_o = prng_reseed_done_q;
					if (cipher_out_ready_o && cipher_out_valid_i) begin
						prng_reseed_we = 1'b1;
						prng_reseed_done_d = 1'b0;
						aes_ctrl_ns = sv2v_cast_9BBFB(6'b001001);
					end
				end
			end
			sv2v_cast_9BBFB(6'b100100):
				if (cipher_dec_key_gen_i) begin
					cipher_out_ready_o = 1'b1;
					if (cipher_out_valid_i) begin
						block_ctr_decr = 1'b1;
						aes_ctrl_ns = sv2v_cast_9BBFB(6'b001001);
					end
				end
				else begin
					cipher_out_ready_o = finish;
					cipher_out_done = (((finish & cipher_out_valid_i) & ~mux_sel_err_i) & ~sp_enc_err_i) & ~cipher_op_err;
					stall = ~finish & cipher_out_valid_i;
					stall_we = 1'b1;
					add_state_out_sel_o = (doing_cbc_dec ? sv2v_cast_870A5(sv2v_cast_F4B48(5'b11000)) : (doing_cfb_enc ? sv2v_cast_870A5(sv2v_cast_F4B48(5'b00001)) : (doing_cfb_dec ? sv2v_cast_870A5(sv2v_cast_F4B48(5'b00001)) : (doing_ofb ? sv2v_cast_870A5(sv2v_cast_F4B48(5'b00001)) : (doing_ctr ? sv2v_cast_870A5(sv2v_cast_F4B48(5'b00001)) : sv2v_cast_870A5(sv2v_cast_F4B48(5'b01110)))))));
					iv_sel_o = (doing_cbc_enc ? sv2v_cast_23662(sv2v_cast_8208B(6'b110000)) : (doing_cbc_dec ? sv2v_cast_23662(sv2v_cast_8208B(6'b000011)) : (doing_cfb_enc ? sv2v_cast_23662(sv2v_cast_8208B(6'b110000)) : (doing_cfb_dec ? sv2v_cast_23662(sv2v_cast_8208B(6'b000011)) : (doing_ofb ? sv2v_cast_23662(sv2v_cast_8208B(6'b001000)) : (doing_ctr ? sv2v_cast_23662(sv2v_cast_8208B(6'b111110)) : sv2v_cast_23662(sv2v_cast_8208B(6'b011101))))))));
					iv_we_o = ((((doing_cbc_enc || doing_cbc_dec) || doing_cfb_enc) || doing_cfb_dec) || doing_ofb ? {aes_pkg_NumSlicesCtr {cipher_out_done}} : (doing_ctr ? ctr_we_i : {aes_pkg_NumSlicesCtr {1'b0}}));
					iv_arm = (((((doing_cbc_enc | doing_cbc_dec) | doing_cfb_enc) | doing_cfb_dec) | doing_ofb) | doing_ctr) & cipher_out_done;
					if (cipher_out_done) begin
						block_ctr_decr = 1'b1;
						data_out_we_o = 1'b1;
						aes_ctrl_ns = sv2v_cast_9BBFB(6'b001001);
					end
				end
			sv2v_cast_9BBFB(6'b111010): begin
				if (key_iv_data_in_clear_i) begin
					key_init_sel_o = sv2v_cast_C3037(sv2v_cast_F4B48(5'b00001));
					key_init_we_o = {aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey {1'b1}};
					key_init_clear = 1'b1;
					iv_sel_o = sv2v_cast_23662(sv2v_cast_8208B(6'b100101));
					iv_we_o = {aes_pkg_NumSlicesCtr {1'b1}};
					iv_clear = 1'b1;
					data_in_we_o = 1'b1;
					data_in_prev_sel_o = sv2v_cast_47617(sv2v_cast_0397F(3'b100));
					data_in_prev_we_o = 1'b1;
				end
				aes_ctrl_ns = sv2v_cast_9BBFB(6'b001110);
			end
			sv2v_cast_9BBFB(6'b001110): begin
				cipher_out_ready_o = 1'b1;
				if (cipher_out_valid_i) begin
					if (cipher_key_clear_i)
						key_iv_data_in_clear_we = 1'b1;
					if (cipher_data_out_clear_i) begin
						data_out_we_o = (~mux_sel_err_i & ~sp_enc_err_i) & ~cipher_op_err;
						data_out_clear_we = 1'b1;
					end
					aes_ctrl_ns = sv2v_cast_9BBFB(6'b001001);
				end
			end
			sv2v_cast_9BBFB(6'b010111): alert_o = 1'b1;
			default: begin
				aes_ctrl_ns = sv2v_cast_9BBFB(6'b010111);
				alert_o = 1'b1;
			end
		endcase
		if (((mux_sel_err_i || sp_enc_err_i) || cipher_op_err) || lc_ctrl_pkg_lc_tx_test_true_loose(lc_escalate_en_i))
			aes_ctrl_ns = sv2v_cast_9BBFB(6'b010111);
	end
	prim_sparse_fsm_flop #(
		.Width(aes_pkg_CtrlStateWidth),
		.ResetValue(sv2v_cast_9BBFB(6'b001001)),
		.EnableAlertTriggerSVA(1)
	) u_state_regs(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.state_i(aes_ctrl_ns),
		.state_o(aes_ctrl_cs)
	);
	always @(posedge clk_i or negedge rst_ni) begin : reg_fsm
		if (!rst_ni)
			prng_reseed_done_q <= 1'b0;
		else
			prng_reseed_done_q <= prng_reseed_done_d;
	end
	assign key_sideload = ((sideload_i & key_sideload_valid_i) & ctrl_we_q) & ~ctrl_phase_i;
	aes_reg_status #(.Width(aes_pkg_NumSharesKey * aes_reg_pkg_NumRegsKey)) u_reg_status_key_init(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.we_i(key_init_we_o),
		.use_i(key_init_load),
		.clear_i(key_init_clear),
		.arm_i(key_init_arm),
		.new_o(key_init_new),
		.new_pulse_o(key_init_new_pulse),
		.clean_o(key_init_ready)
	);
	aes_reg_status #(.Width(aes_pkg_NumSlicesCtr)) u_reg_status_iv(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.we_i(iv_we_o),
		.use_i(iv_load),
		.clear_i(iv_clear),
		.arm_i(iv_arm),
		.new_o(iv_ready),
		.new_pulse_o(),
		.clean_o()
	);
	always @(posedge clk_i or negedge rst_ni) begin : reg_ctrl_we
		if (!rst_ni)
			ctrl_we_q <= 1'b0;
		else
			ctrl_we_q <= ctrl_we_o;
	end
	assign clear_in_out_status = ctrl_we_q;
	assign data_in_new_d = ((data_in_load || &data_in_qe_i) || clear_in_out_status ? {4 {1'sb0}} : data_in_new_q | data_in_qe_i);
	assign data_in_new = &data_in_new_d;
	assign data_out_read_d = (&data_out_read_q || clear_in_out_status ? {4 {1'sb0}} : data_out_read_q | data_out_re_i);
	assign data_out_read = &data_out_read_d;
	always @(posedge clk_i or negedge rst_ni) begin : reg_edge_detection
		if (!rst_ni) begin
			data_in_new_q <= 1'sb0;
			data_out_read_q <= 1'sb0;
		end
		else begin
			data_in_new_q <= data_in_new_d;
			data_out_read_q <= data_out_read_d;
		end
	end
	assign input_ready = ~data_in_new;
	assign input_ready_we = ((data_in_new | data_in_load) | data_in_we_o) | clear_in_out_status;
	assign output_valid = data_out_we_o & ~data_out_clear_we;
	assign output_valid_we = ((data_out_we_o | data_out_read) | data_out_clear_we) | clear_in_out_status;
	always @(posedge clk_i or negedge rst_ni) begin : reg_output_valid
		if (!rst_ni)
			output_valid_q <= 1'sb0;
		else if (output_valid_we)
			output_valid_q <= output_valid;
	end
	assign output_lost = (ctrl_we_o ? 1'b0 : (output_lost_i ? 1'b1 : output_valid_q & ~data_out_read));
	assign output_lost_we = ctrl_we_o | data_out_we_o;
	localparam [0:0] aes_pkg_ClearStatusOnFatalAlert = 1'b0;
	assign clear_on_fatal = (aes_pkg_ClearStatusOnFatalAlert ? alert_fatal_i : 1'b0);
	assign idle_o = (clear_on_fatal ? 1'b0 : idle);
	assign idle_we_o = (clear_on_fatal ? 1'b1 : idle_we);
	assign stall_o = (clear_on_fatal ? 1'b0 : stall);
	assign stall_we_o = (clear_on_fatal ? 1'b1 : stall_we);
	assign output_lost_o = (clear_on_fatal ? 1'b0 : output_lost);
	assign output_lost_we_o = (clear_on_fatal ? 1'b1 : output_lost_we);
	assign output_valid_o = (clear_on_fatal ? 1'b0 : output_valid);
	assign output_valid_we_o = (clear_on_fatal ? 1'b1 : output_valid_we);
	assign input_ready_o = (clear_on_fatal ? 1'b0 : input_ready);
	assign input_ready_we_o = (clear_on_fatal ? 1'b1 : input_ready_we);
	assign start_we_o = (clear_on_fatal ? 1'b1 : start_we);
	assign key_iv_data_in_clear_we_o = (clear_on_fatal ? 1'b1 : key_iv_data_in_clear_we);
	assign data_out_clear_we_o = (clear_on_fatal ? 1'b1 : data_out_clear_we);
	assign prng_reseed_o = (clear_on_fatal ? 1'b0 : (key_init_new_pulse ? 1'b1 : 1'b0));
	assign prng_reseed_we_o = (clear_on_fatal ? 1'b1 : (key_init_new_pulse ? key_touch_forces_reseed_i : prng_reseed_we));
	localparam [31:0] aes_pkg_BlockCtrWidth = 13;
	function automatic [2:0] sv2v_cast_72367;
		input reg [2:0] inp;
		sv2v_cast_72367 = inp;
	endfunction
	function automatic signed [12:0] sv2v_cast_E87F7_signed;
		input reg signed [12:0] inp;
		sv2v_cast_E87F7_signed = inp;
	endfunction
	generate
		if (SecMasking) begin : gen_block_ctr
			wire block_ctr_set;
			wire [12:0] block_ctr_d;
			reg [12:0] block_ctr_q;
			wire [12:0] block_ctr_set_val;
			wire [12:0] block_ctr_decr_val;
			assign block_ctr_expr = block_ctr_q == {13 {1'sb0}};
			assign block_ctr_set = ctrl_we_q | (block_ctr_decr & (block_ctr_expr | cipher_prng_reseed_i));
			assign block_ctr_set_val = (prng_reseed_rate_i == sv2v_cast_72367(3'b001) ? {13 {1'sb0}} : (prng_reseed_rate_i == sv2v_cast_72367(3'b010) ? sv2v_cast_E87F7_signed(63) : (prng_reseed_rate_i == sv2v_cast_72367(3'b100) ? sv2v_cast_E87F7_signed(8191) : {13 {1'sb0}})));
			assign block_ctr_decr_val = block_ctr_q - sv2v_cast_E87F7_signed(1);
			assign block_ctr_d = (block_ctr_set ? block_ctr_set_val : (block_ctr_decr ? block_ctr_decr_val : block_ctr_q));
			always @(posedge clk_i or negedge rst_ni) begin : reg_block_ctr
				if (!rst_ni)
					block_ctr_q <= 1'sb0;
				else
					block_ctr_q <= block_ctr_d;
			end
		end
		else begin : gen_no_block_ctr
			assign block_ctr_expr = 1'b0;
			wire unused_block_ctr_decr;
			wire [2:0] unused_prng_reseed_rate;
			wire unused_cipher_prng_reseed;
			assign unused_block_ctr_decr = block_ctr_decr;
			assign unused_prng_reseed_rate = prng_reseed_rate_i;
			assign unused_cipher_prng_reseed = cipher_prng_reseed_i;
		end
	endgenerate
	localparam signed [31:0] AesControlFsmSecMaskingNonDefault = (SecMasking == 1 ? 1 : 2);
	function automatic [AesControlFsmSecMaskingNonDefault - 1:0] sv2v_cast_FDA2C;
		input reg [AesControlFsmSecMaskingNonDefault - 1:0] inp;
		sv2v_cast_FDA2C = inp;
	endfunction
	always @(*) begin : sv2v_autoblock_1
		reg unused_assert_static_lint_error;
		unused_assert_static_lint_error = sv2v_cast_FDA2C(1'b1);
	end
endmodule

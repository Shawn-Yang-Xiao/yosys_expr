module aes_cipher_core_tb (
	clk_i,
	rst_ni,
	test_done_o,
	test_passed_o
);
	input wire clk_i;
	input wire rst_ni;
	output reg test_done_o;
	output reg test_passed_o;
	localparam [0:0] SecMasking = 1;
	localparam integer SecSBoxImpl = (SecMasking ? 32'sd4 : 32'sd1);
	localparam signed [31:0] NumShares = (SecMasking ? 2 : 1);
	localparam [31:0] edn_pkg_ENDPOINT_BUS_WIDTH = 32;
	localparam [31:0] EntropyWidth = edn_pkg_ENDPOINT_BUS_WIDTH;
	localparam signed [31:0] aes_pkg_Mux2SelWidth = 3;
	localparam signed [31:0] aes_pkg_Sp2VWidth = aes_pkg_Mux2SelWidth;
	wire [2:0] in_ready;
	reg [2:0] in_valid;
	wire [2:0] out_valid;
	localparam signed [31:0] aes_pkg_AES_OP_WIDTH = 2;
	reg [1:0] op;
	localparam signed [31:0] aes_pkg_AES_KEYLEN_WIDTH = 3;
	reg [2:0] key_len_d;
	reg [2:0] key_len_q;
	reg [2:0] crypt;
	reg [2:0] dec_key_gen;
	reg prng_reseed;
	localparam [31:0] aes_pkg_WidthPRDClearing = 64;
	reg [(NumShares * aes_pkg_WidthPRDClearing) - 1:0] prd_clearing;
	wire [127:0] state_mask;
	wire [(((NumShares * 4) * 4) * 8) - 1:0] state_init;
	wire [(((NumShares * 4) * 4) * 8) - 1:0] state_done;
	wire [((NumShares * 8) * 32) - 1:0] key_init;
	wire entropy_masking_req;
	reg [31:0] entropy_masking;
	wire alert;
	function automatic [2:0] sv2v_cast_0397F;
		input reg [2:0] inp;
		sv2v_cast_0397F = inp;
	endfunction
	function automatic [2:0] sv2v_cast_AC049;
		input reg [2:0] inp;
		sv2v_cast_AC049 = inp;
	endfunction
	aes_cipher_core #(
		.SecMasking(SecMasking),
		.SecSBoxImpl(SecSBoxImpl)
	) u_aes_cipher_core(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.in_valid_i(in_valid),
		.in_ready_o(in_ready),
		.out_valid_o(out_valid),
		.out_ready_i(sv2v_cast_AC049(sv2v_cast_0397F(3'b011))),
		.cfg_valid_i(1'b1),
		.op_i(op),
		.key_len_i(key_len_q),
		.crypt_i(crypt),
		.crypt_o(),
		.dec_key_gen_i(dec_key_gen),
		.dec_key_gen_o(),
		.prng_reseed_i(prng_reseed),
		.prng_reseed_o(),
		.key_clear_i(1'b0),
		.key_clear_o(),
		.data_out_clear_i(1'b0),
		.data_out_clear_o(),
		.alert_fatal_i(1'b0),
		.alert_o(alert),
		.prd_clearing_i(prd_clearing),
		.force_masks_i(1'b0),
		.data_in_mask_o(state_mask),
		.entropy_req_o(entropy_masking_req),
		.entropy_ack_i(1'b1),
		.entropy_i(entropy_masking),
		.state_init_i(state_init),
		.key_init_i(key_init),
		.state_o(state_done)
	);
	localparam signed [31:0] CipherCoreTbStateWidth = 3;
	reg [2:0] aes_cipher_core_tb_state_d;
	reg [2:0] aes_cipher_core_tb_state_q;
	wire [7:0] block_count_d;
	reg [7:0] block_count_q;
	reg block_count_increment;
	reg block_count_clear;
	reg data_in_buf_we;
	reg data_out_buf_we;
	reg check;
	wire mismatch;
	reg test_done;
	reg [127:0] data_in_rand;
	wire [127:0] data_in;
	wire [127:0] data_out;
	reg [32767:0] data_in_buf;
	reg [32767:0] data_out_buf;
	assign block_count_d = (block_count_clear ? {8 {1'sb0}} : (block_count_increment ? block_count_q + 8'h01 : block_count_q));
	always @(posedge clk_i or negedge rst_ni) begin : reg_count
		if (!rst_ni)
			block_count_q <= 1'sb0;
		else
			block_count_q <= block_count_d;
	end
	function automatic [63:0] sv2v_cast_DEA0D;
		input reg [63:0] inp;
		sv2v_cast_DEA0D = inp;
	endfunction
	function automatic [63:0] sv2v_cast_84BEE;
		input reg [63:0] inp;
		sv2v_cast_84BEE = inp;
	endfunction
	always @(posedge clk_i or negedge rst_ni) begin : reg_prd_clearing
		if (!rst_ni)
			prd_clearing <= {NumShares {sv2v_cast_DEA0D(1'sb0)}};
		else if (out_valid == sv2v_cast_AC049(sv2v_cast_0397F(3'b011)))
			prd_clearing <= {NumShares {sv2v_cast_84BEE({$urandom, $urandom})}};
	end
	always @(posedge clk_i or negedge rst_ni) begin : reg_entropy_masking
		if (!rst_ni)
			entropy_masking <= 1'sb0;
		else if (entropy_masking_req)
			entropy_masking <= $urandom;
	end
	function automatic [255:0] sv2v_cast_256;
		input reg [255:0] inp;
		sv2v_cast_256 = inp;
	endfunction
	assign key_init = {NumShares {sv2v_cast_256({8 {$urandom}})}};
	function automatic [2:0] sv2v_cast_340F2;
		input reg [2:0] inp;
		sv2v_cast_340F2 = inp;
	endfunction
	always @(posedge clk_i or negedge rst_ni) begin : reg_key_len
		if (!rst_ni)
			key_len_q <= sv2v_cast_340F2(3'b001);
		else
			key_len_q <= key_len_d;
	end
	always @(posedge clk_i or negedge rst_ni) begin : reg_data_in
		if (!rst_ni)
			data_in_rand <= 1'sb0;
		else if (block_count_increment)
			data_in_rand <= {4 {$urandom}};
	end
	function automatic [2:0] sv2v_cast_33ECF;
		input reg [2:0] inp;
		sv2v_cast_33ECF = inp;
	endfunction
	assign data_in = (aes_cipher_core_tb_state_q == sv2v_cast_33ECF(4) ? data_out_buf[8 * (4 * ((255 - block_count_q) * 4))+:128] : data_in_rand);
	generate
		if (!SecMasking) begin : gen_state_init_no_masking
			assign state_init[8 * (4 * ((NumShares - 1) * 4))+:128] = data_in;
			wire unused_bits;
			assign unused_bits = ^state_mask;
		end
		else begin : gen_state_init_masking
			assign state_init[8 * (4 * ((NumShares - 1) * 4))+:128] = data_in ^ state_mask;
			assign state_init[8 * (4 * ((NumShares - 2) * 4))+:128] = state_mask;
		end
	endgenerate
	function automatic [1:0] sv2v_cast_E41EB;
		input reg [1:0] inp;
		sv2v_cast_E41EB = inp;
	endfunction
	always @(*) begin : aes_cipher_core_tb_fsm
		in_valid = sv2v_cast_AC049(sv2v_cast_0397F(3'b100));
		op = sv2v_cast_E41EB(2'b01);
		crypt = sv2v_cast_AC049(sv2v_cast_0397F(3'b011));
		dec_key_gen = sv2v_cast_AC049(sv2v_cast_0397F(3'b100));
		prng_reseed = 1'b0;
		aes_cipher_core_tb_state_d = aes_cipher_core_tb_state_q;
		block_count_increment = 1'b0;
		block_count_clear = 1'b0;
		key_len_d = key_len_q;
		data_in_buf_we = 1'b0;
		data_out_buf_we = 1'b0;
		check = 1'b0;
		test_done = 1'b0;
		case (aes_cipher_core_tb_state_q)
			sv2v_cast_33ECF(0):
				if (in_ready == sv2v_cast_AC049(sv2v_cast_0397F(3'b011)))
					aes_cipher_core_tb_state_d = (SecMasking ? sv2v_cast_33ECF(1) : sv2v_cast_33ECF(2));
			sv2v_cast_33ECF(1): begin
				in_valid = sv2v_cast_AC049(sv2v_cast_0397F(3'b011));
				crypt = sv2v_cast_AC049(sv2v_cast_0397F(3'b100));
				prng_reseed = 1'b1;
				if (out_valid == sv2v_cast_AC049(sv2v_cast_0397F(3'b011)))
					aes_cipher_core_tb_state_d = sv2v_cast_33ECF(2);
			end
			sv2v_cast_33ECF(2): begin
				in_valid = sv2v_cast_AC049(sv2v_cast_0397F(3'b011));
				prng_reseed = 1'b1;
				if (out_valid == sv2v_cast_AC049(sv2v_cast_0397F(3'b011))) begin
					block_count_increment = 1'b1;
					data_in_buf_we = 1'b1;
					data_out_buf_we = 1'b1;
					key_len_d = (block_count_q == 8'd7 ? sv2v_cast_340F2(3'b010) : (block_count_q == 8'd15 ? sv2v_cast_340F2(3'b100) : key_len_q));
					if (block_count_q == 8'd23) begin
						block_count_clear = 1'b1;
						key_len_d = sv2v_cast_340F2(3'b001);
						aes_cipher_core_tb_state_d = sv2v_cast_33ECF(3);
					end
				end
			end
			sv2v_cast_33ECF(3): begin
				in_valid = sv2v_cast_AC049(sv2v_cast_0397F(3'b011));
				dec_key_gen = sv2v_cast_AC049(sv2v_cast_0397F(3'b011));
				prng_reseed = 1'b1;
				if (out_valid == sv2v_cast_AC049(sv2v_cast_0397F(3'b011)))
					aes_cipher_core_tb_state_d = sv2v_cast_33ECF(4);
			end
			sv2v_cast_33ECF(4): begin
				in_valid = sv2v_cast_AC049(sv2v_cast_0397F(3'b011));
				op = sv2v_cast_E41EB(2'b10);
				prng_reseed = 1'b1;
				if (out_valid == sv2v_cast_AC049(sv2v_cast_0397F(3'b011))) begin
					block_count_increment = 1'b1;
					check = 1'b1;
					if (block_count_q == 8'd7) begin
						key_len_d = sv2v_cast_340F2(3'b010);
						aes_cipher_core_tb_state_d = sv2v_cast_33ECF(3);
					end
					else if (block_count_q == 8'd15) begin
						key_len_d = sv2v_cast_340F2(3'b100);
						aes_cipher_core_tb_state_d = sv2v_cast_33ECF(3);
					end
					else if (block_count_q == 8'd23)
						aes_cipher_core_tb_state_d = sv2v_cast_33ECF(5);
				end
			end
			sv2v_cast_33ECF(5): test_done = 1'b1;
			default: aes_cipher_core_tb_state_d = sv2v_cast_33ECF(5);
		endcase
	end
	always @(posedge clk_i or negedge rst_ni) begin : reg_fsm
		if (!rst_ni)
			aes_cipher_core_tb_state_q <= sv2v_cast_33ECF(0);
		else
			aes_cipher_core_tb_state_q <= aes_cipher_core_tb_state_d;
	end
	generate
		if (!SecMasking) begin : gen_data_out_no_masking
			assign data_out = (out_valid == sv2v_cast_AC049(sv2v_cast_0397F(3'b011)) ? state_done[8 * (4 * ((NumShares - 1) * 4))+:128] : {128 {1'sb0}});
		end
		else begin : gen_data_out_masking
			assign data_out = (out_valid == sv2v_cast_AC049(sv2v_cast_0397F(3'b011)) ? state_done[8 * (4 * ((NumShares - 2) * 4))+:128] ^ state_done[8 * (4 * ((NumShares - 1) * 4))+:128] : {128 {1'sb0}});
		end
	endgenerate
	always @(posedge clk_i or negedge rst_ni) begin : reg_data_in_buf
		if (!rst_ni)
			data_in_buf <= {256 {128'b00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000}};
		else if (data_in_buf_we)
			data_in_buf[8 * (4 * ((255 - block_count_q) * 4))+:128] <= data_in;
	end
	always @(posedge clk_i or negedge rst_ni) begin : reg_data_out_buf
		if (!rst_ni)
			data_out_buf <= {256 {128'b00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000}};
		else if (data_out_buf_we)
			data_out_buf[8 * (4 * ((255 - block_count_q) * 4))+:128] <= data_out;
	end
	assign mismatch = (check ? data_out != data_in_buf[8 * (4 * ((255 - block_count_q) * 4))+:128] : 1'b0);
	always @(posedge clk_i or negedge rst_ni) begin : tb_ctrl
		test_done_o <= 1'b0;
		test_passed_o <= 1'b0;
		if (rst_ni && (aes_cipher_core_tb_state_q != sv2v_cast_33ECF(0))) begin
			if (alert) begin
				$display("\nERROR: Fatal alert condition detected.");
				test_done_o <= 1'b1;
			end
			else if (mismatch) begin
				$display("\nERROR: AES output does not match expected value.");
				test_done_o <= 1'b1;
			end
			else if (test_done) begin
				$display("\nSUCCESS: All AES ciphertexts correctly decrypted.");
				test_passed_o <= 1'b1;
				test_done_o <= 1'b1;
			end
		end
		if (block_count_q == 8'hff) begin
			$display("\nERROR: Simulation timed out.");
			test_done_o <= 1'b1;
		end
	end
endmodule

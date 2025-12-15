module aes_prng_masking (
	clk_i,
	rst_ni,
	force_masks_i,
	data_update_i,
	data_o,
	reseed_req_i,
	reseed_ack_o,
	entropy_req_o,
	entropy_ack_i,
	entropy_i
);
	localparam [31:0] aes_pkg_WidthPRDSBox = 8;
	localparam [31:0] aes_pkg_WidthPRDData = 128;
	localparam [31:0] aes_pkg_WidthPRDKey = 32;
	localparam [31:0] aes_pkg_WidthPRDMasking = aes_pkg_WidthPRDData + aes_pkg_WidthPRDKey;
	parameter [31:0] Width = aes_pkg_WidthPRDMasking;
	localparam [31:0] aes_pkg_ChunkSizePRDMasking = aes_pkg_WidthPRDMasking / 5;
	parameter [31:0] ChunkSize = aes_pkg_ChunkSizePRDMasking;
	localparam [31:0] edn_pkg_ENDPOINT_BUS_WIDTH = 32;
	parameter [31:0] EntropyWidth = edn_pkg_ENDPOINT_BUS_WIDTH;
	parameter [0:0] SecAllowForcingMasks = 0;
	parameter [0:0] SecSkipPRNGReseeding = 0;
	localparam [31:0] NumChunks = Width / ChunkSize;
	localparam signed [31:0] aes_pkg_MaskingLfsrWidth = 160;
	localparam [159:0] aes_pkg_RndCnstMaskingLfsrSeedDefault = 160'h0c132b5723c5a4cf4743b3c7c32d580f74f1713a;
	parameter [159:0] RndCnstLfsrSeed = aes_pkg_RndCnstMaskingLfsrSeedDefault;
	localparam [1279:0] aes_pkg_RndCnstMaskingLfsrPermDefault = 1280'h17261943423e4c5c03872194050c7e5f8497081d96666d406f4b6064733034698e7c721c8832471f59919e0b128f067b25622768462e554d8970815d490d7f44048c867d907a239b20220f6c79071a852d76485452189f14091b1e744e3967374f785b772b352f6550613c58130a8b104a3f28019c9a380233956b00563a512c808d419d63982a16995e0e3b57826a36718a9329452492533d83115a75316e15;
	parameter [1279:0] RndCnstLfsrPerm = aes_pkg_RndCnstMaskingLfsrPermDefault;
	input wire clk_i;
	input wire rst_ni;
	input wire force_masks_i;
	input wire data_update_i;
	output wire [Width - 1:0] data_o;
	input wire reseed_req_i;
	output wire reseed_ack_o;
	output wire entropy_req_o;
	input wire entropy_ack_i;
	input wire [EntropyWidth - 1:0] entropy_i;
	wire [NumChunks - 1:0] prng_seed_en;
	wire [(NumChunks * ChunkSize) - 1:0] prng_seed;
	wire prng_en;
	wire [(NumChunks * ChunkSize) - 1:0] prng_state;
	wire [(NumChunks * ChunkSize) - 1:0] perm;
	wire [Width - 1:0] prng_b;
	wire [Width - 1:0] perm_b;
	reg phase_q;
	localparam signed [31:0] AesSecAllowForcingMasksNonDefault = (SecAllowForcingMasks == 0 ? 1 : 2);
	function automatic [AesSecAllowForcingMasksNonDefault - 1:0] sv2v_cast_C9429;
		input reg [AesSecAllowForcingMasksNonDefault - 1:0] inp;
		sv2v_cast_C9429 = inp;
	endfunction
	always @(*) begin : sv2v_autoblock_1
		reg unused_assert_static_lint_error;
		unused_assert_static_lint_error = sv2v_cast_C9429(1'b1);
	end
	generate
		if (SecAllowForcingMasks == 0) begin : gen_unused_force_masks
			wire unused_force_masks;
			assign unused_force_masks = force_masks_i;
		end
	endgenerate
	assign prng_en = (SecAllowForcingMasks && force_masks_i ? 1'b0 : data_update_i);
	localparam signed [31:0] AesSecSkipPRNGReseedingNonDefault = (SecSkipPRNGReseeding == 0 ? 1 : 2);
	function automatic [AesSecSkipPRNGReseedingNonDefault - 1:0] sv2v_cast_D040B;
		input reg [AesSecSkipPRNGReseedingNonDefault - 1:0] inp;
		sv2v_cast_D040B = inp;
	endfunction
	always @(*) begin : sv2v_autoblock_2
		reg unused_assert_static_lint_error;
		unused_assert_static_lint_error = sv2v_cast_D040B(1'b1);
	end
	function automatic integer prim_util_pkg_vbits;
		input integer value;
		prim_util_pkg_vbits = (value == 1 ? 1 : $clog2(value));
	endfunction
	generate
		if (ChunkSize == EntropyWidth) begin : gen_counter
			localparam [31:0] ChunkIdxWidth = prim_util_pkg_vbits(NumChunks);
			wire [ChunkIdxWidth - 1:0] chunk_idx_d;
			reg [ChunkIdxWidth - 1:0] chunk_idx_q;
			wire prng_reseed_done;
			assign entropy_req_o = (SecSkipPRNGReseeding ? 1'b0 : reseed_req_i);
			assign reseed_ack_o = (SecSkipPRNGReseeding ? reseed_req_i : prng_reseed_done);
			function automatic [ChunkIdxWidth - 1:0] sv2v_cast_64FBD;
				input reg [ChunkIdxWidth - 1:0] inp;
				sv2v_cast_64FBD = inp;
			endfunction
			assign prng_reseed_done = ((chunk_idx_q == sv2v_cast_64FBD(NumChunks - 1)) & entropy_req_o) & entropy_ack_i;
			function automatic signed [ChunkIdxWidth - 1:0] sv2v_cast_64FBD_signed;
				input reg signed [ChunkIdxWidth - 1:0] inp;
				sv2v_cast_64FBD_signed = inp;
			endfunction
			assign chunk_idx_d = (prng_reseed_done ? {ChunkIdxWidth {1'sb0}} : (entropy_req_o && entropy_ack_i ? chunk_idx_q + sv2v_cast_64FBD_signed(1) : chunk_idx_q));
			always @(posedge clk_i or negedge rst_ni) begin : reg_chunk_idx
				if (!rst_ni)
					chunk_idx_q <= 1'sb0;
				else
					chunk_idx_q <= chunk_idx_d;
			end
			genvar _gv_c_1;
			for (_gv_c_1 = 0; _gv_c_1 < NumChunks; _gv_c_1 = _gv_c_1 + 1) begin : gen_seeds
				localparam c = _gv_c_1;
				assign prng_seed[c * ChunkSize+:ChunkSize] = entropy_i;
				assign prng_seed_en[c] = (c == chunk_idx_q ? entropy_req_o & entropy_ack_i : 1'b0);
			end
		end
		else begin : gen_packer
			wire [Width - 1:0] seed;
			wire seed_valid;
			assign entropy_req_o = (SecSkipPRNGReseeding ? 1'b0 : reseed_req_i & ~seed_valid);
			assign reseed_ack_o = (SecSkipPRNGReseeding ? reseed_req_i : seed_valid);
			prim_packer_fifo #(
				.InW(EntropyWidth),
				.OutW(Width),
				.ClearOnRead(1'b0)
			) u_prim_packer_fifo(
				.clk_i(clk_i),
				.rst_ni(rst_ni),
				.clr_i(1'b0),
				.wvalid_i(entropy_ack_i),
				.wdata_i(entropy_i),
				.wready_o(),
				.rvalid_o(seed_valid),
				.rdata_o(seed),
				.rready_i(1'b1),
				.depth_o()
			);
			genvar _gv_c_2;
			for (_gv_c_2 = 0; _gv_c_2 < NumChunks; _gv_c_2 = _gv_c_2 + 1) begin : gen_seeds
				localparam c = _gv_c_2;
				assign prng_seed[c * ChunkSize+:ChunkSize] = seed[c * ChunkSize+:ChunkSize];
				assign prng_seed_en[c] = (SecSkipPRNGReseeding ? 1'b0 : seed_valid);
			end
		end
	endgenerate
	genvar _gv_c_3;
	generate
		for (_gv_c_3 = 0; _gv_c_3 < NumChunks; _gv_c_3 = _gv_c_3 + 1) begin : gen_lfsrs
			localparam c = _gv_c_3;
			prim_lfsr #(
				.LfsrType("GAL_XOR"),
				.LfsrDw(ChunkSize),
				.StateOutDw(ChunkSize),
				.DefaultSeed(RndCnstLfsrSeed[c * ChunkSize+:ChunkSize]),
				.StatePermEn(1'b0),
				.NonLinearOut(1'b1)
			) u_lfsr_chunk(
				.clk_i(clk_i),
				.rst_ni(rst_ni),
				.seed_en_i(prng_seed_en[c]),
				.seed_i(prng_seed[c * ChunkSize+:ChunkSize]),
				.lfsr_en_i(prng_en),
				.entropy_i(1'sb0),
				.state_o(prng_state[c * ChunkSize+:ChunkSize])
			);
		end
	endgenerate
	assign prng_b = prng_state;
	genvar _gv_b_1;
	generate
		for (_gv_b_1 = 0; _gv_b_1 < Width; _gv_b_1 = _gv_b_1 + 1) begin : gen_perm
			localparam b = _gv_b_1;
			assign perm_b[b] = prng_b[RndCnstLfsrPerm[b * 8+:8]];
		end
	endgenerate
	assign perm = perm_b;
	assign data_o = (phase_q ? {perm[0+:ChunkSize], perm[ChunkSize * (((NumChunks - 1) >= 1 ? NumChunks - 1 : ((NumChunks - 1) + ((NumChunks - 1) >= 1 ? NumChunks - 1 : 3 - NumChunks)) - 1) - (((NumChunks - 1) >= 1 ? NumChunks - 1 : 3 - NumChunks) - 1))+:ChunkSize * ((NumChunks - 1) >= 1 ? NumChunks - 1 : 3 - NumChunks)]} : perm);
	always @(posedge clk_i or negedge rst_ni) begin : reg_phase
		if (!rst_ni)
			phase_q <= 1'sb0;
		else if (prng_en)
			phase_q <= ~phase_q;
	end
endmodule

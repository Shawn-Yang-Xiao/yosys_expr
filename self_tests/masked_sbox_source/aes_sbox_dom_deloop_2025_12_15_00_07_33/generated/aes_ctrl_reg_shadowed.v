module aes_ctrl_reg_shadowed (
	clk_i,
	rst_ni,
	rst_shadowed_ni,
	qe_o,
	we_i,
	phase_o,
	operation_o,
	mode_o,
	key_len_o,
	sideload_o,
	prng_reseed_rate_o,
	manual_operation_o,
	err_update_o,
	err_storage_o,
	reg2hw_ctrl_i,
	hw2reg_ctrl_o
);
	parameter [0:0] AES192Enable = 1;
	input wire clk_i;
	input wire rst_ni;
	input wire rst_shadowed_ni;
	output wire qe_o;
	input wire we_i;
	output wire phase_o;
	localparam signed [31:0] aes_pkg_AES_OP_WIDTH = 2;
	output wire [1:0] operation_o;
	localparam signed [31:0] aes_pkg_AES_MODE_WIDTH = 6;
	output wire [5:0] mode_o;
	localparam signed [31:0] aes_pkg_AES_KEYLEN_WIDTH = 3;
	output wire [2:0] key_len_o;
	output wire sideload_o;
	localparam signed [31:0] aes_pkg_AES_PRNGRESEEDRATE_WIDTH = 3;
	output wire [2:0] prng_reseed_rate_o;
	output wire manual_operation_o;
	output wire err_update_o;
	output wire err_storage_o;
	input wire [27:0] reg2hw_ctrl_i;
	output wire [15:0] hw2reg_ctrl_o;
	reg [15:0] ctrl_wd;
	wire [1:0] op;
	wire [5:0] mode;
	wire [2:0] key_len;
	wire [2:0] prng_reseed_rate;
	wire phase_operation;
	wire phase_mode;
	wire phase_key_len;
	wire phase_key_sideload;
	wire phase_prng_reseed_rate;
	wire phase_manual_operation;
	wire err_update_operation;
	wire err_update_mode;
	wire err_update_key_len;
	wire err_update_sideload;
	wire err_update_prng_reseed_rate;
	wire err_update_manual_operation;
	wire err_storage_operation;
	wire err_storage_mode;
	wire err_storage_key_len;
	wire err_storage_sideload;
	wire err_storage_prng_reseed_rate;
	wire err_storage_manual_operation;
	assign qe_o = ((((reg2hw_ctrl_i[25] & reg2hw_ctrl_i[17]) & reg2hw_ctrl_i[12]) & reg2hw_ctrl_i[9]) & reg2hw_ctrl_i[4]) & reg2hw_ctrl_i[1];
	function automatic [1:0] sv2v_cast_E41EB;
		input reg [1:0] inp;
		sv2v_cast_E41EB = inp;
	endfunction
	assign op = sv2v_cast_E41EB(reg2hw_ctrl_i[27-:2]);
	always @(*) begin : operation_get
		case (op)
			sv2v_cast_E41EB(2'b01): ctrl_wd[1-:aes_pkg_AES_OP_WIDTH] = sv2v_cast_E41EB(2'b01);
			sv2v_cast_E41EB(2'b10): ctrl_wd[1-:aes_pkg_AES_OP_WIDTH] = sv2v_cast_E41EB(2'b10);
			default: ctrl_wd[1-:aes_pkg_AES_OP_WIDTH] = sv2v_cast_E41EB(2'b01);
		endcase
	end
	function automatic [5:0] sv2v_cast_BC361;
		input reg [5:0] inp;
		sv2v_cast_BC361 = inp;
	endfunction
	assign mode = sv2v_cast_BC361(reg2hw_ctrl_i[23-:6]);
	always @(*) begin : mode_get
		case (mode)
			sv2v_cast_BC361(6'b000001): ctrl_wd[7-:6] = sv2v_cast_BC361(6'b000001);
			sv2v_cast_BC361(6'b000010): ctrl_wd[7-:6] = sv2v_cast_BC361(6'b000010);
			sv2v_cast_BC361(6'b000100): ctrl_wd[7-:6] = sv2v_cast_BC361(6'b000100);
			sv2v_cast_BC361(6'b001000): ctrl_wd[7-:6] = sv2v_cast_BC361(6'b001000);
			sv2v_cast_BC361(6'b010000): ctrl_wd[7-:6] = sv2v_cast_BC361(6'b010000);
			default: ctrl_wd[7-:6] = sv2v_cast_BC361(6'b100000);
		endcase
	end
	function automatic [2:0] sv2v_cast_340F2;
		input reg [2:0] inp;
		sv2v_cast_340F2 = inp;
	endfunction
	assign key_len = sv2v_cast_340F2(reg2hw_ctrl_i[15-:3]);
	always @(*) begin : key_len_get
		case (key_len)
			sv2v_cast_340F2(3'b001): ctrl_wd[10-:3] = sv2v_cast_340F2(3'b001);
			sv2v_cast_340F2(3'b100): ctrl_wd[10-:3] = sv2v_cast_340F2(3'b100);
			sv2v_cast_340F2(3'b010): ctrl_wd[10-:3] = (AES192Enable ? sv2v_cast_340F2(3'b010) : sv2v_cast_340F2(3'b100));
			default: ctrl_wd[10-:3] = sv2v_cast_340F2(3'b100);
		endcase
	end
	wire [1:1] sv2v_tmp_0A44A;
	assign sv2v_tmp_0A44A = reg2hw_ctrl_i[10];
	always @(*) ctrl_wd[11] = sv2v_tmp_0A44A;
	function automatic [2:0] sv2v_cast_72367;
		input reg [2:0] inp;
		sv2v_cast_72367 = inp;
	endfunction
	assign prng_reseed_rate = sv2v_cast_72367(reg2hw_ctrl_i[7-:3]);
	always @(*) begin : prng_reseed_rate_get
		case (prng_reseed_rate)
			sv2v_cast_72367(3'b001): ctrl_wd[14-:3] = sv2v_cast_72367(3'b001);
			sv2v_cast_72367(3'b010): ctrl_wd[14-:3] = sv2v_cast_72367(3'b010);
			sv2v_cast_72367(3'b100): ctrl_wd[14-:3] = sv2v_cast_72367(3'b100);
			default: ctrl_wd[14-:3] = sv2v_cast_72367(3'b001);
		endcase
	end
	wire [1:1] sv2v_tmp_13C9E;
	assign sv2v_tmp_13C9E = reg2hw_ctrl_i[2];
	always @(*) ctrl_wd[15] = sv2v_tmp_13C9E;
	localparam [1:0] aes_reg_pkg_AES_CTRL_SHADOWED_OPERATION_RESVAL = 2'h1;
	prim_subreg_shadow #(
		.DW(aes_pkg_AES_OP_WIDTH),
		.SwAccess(3'd2),
		.RESVAL(aes_reg_pkg_AES_CTRL_SHADOWED_OPERATION_RESVAL)
	) u_ctrl_reg_shadowed_operation(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.rst_shadowed_ni(rst_shadowed_ni),
		.re(reg2hw_ctrl_i[24]),
		.we(we_i),
		.wd({ctrl_wd[1-:aes_pkg_AES_OP_WIDTH]}),
		.de(1'b0),
		.d(1'sb0),
		.qe(),
		.q(hw2reg_ctrl_o[15-:2]),
		.qs(),
		.ds(),
		.phase(phase_operation),
		.err_update(err_update_operation),
		.err_storage(err_storage_operation)
	);
	localparam [5:0] aes_reg_pkg_AES_CTRL_SHADOWED_MODE_RESVAL = 6'h20;
	prim_subreg_shadow #(
		.DW(aes_pkg_AES_MODE_WIDTH),
		.SwAccess(3'd2),
		.RESVAL(aes_reg_pkg_AES_CTRL_SHADOWED_MODE_RESVAL)
	) u_ctrl_reg_shadowed_mode(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.rst_shadowed_ni(rst_shadowed_ni),
		.re(reg2hw_ctrl_i[16]),
		.we(we_i),
		.wd({ctrl_wd[7-:6]}),
		.de(1'b0),
		.d(1'sb0),
		.qe(),
		.q(hw2reg_ctrl_o[13-:6]),
		.qs(),
		.ds(),
		.phase(phase_mode),
		.err_update(err_update_mode),
		.err_storage(err_storage_mode)
	);
	localparam [2:0] aes_reg_pkg_AES_CTRL_SHADOWED_KEY_LEN_RESVAL = 3'h1;
	prim_subreg_shadow #(
		.DW(aes_pkg_AES_KEYLEN_WIDTH),
		.SwAccess(3'd2),
		.RESVAL(aes_reg_pkg_AES_CTRL_SHADOWED_KEY_LEN_RESVAL)
	) u_ctrl_reg_shadowed_key_len(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.rst_shadowed_ni(rst_shadowed_ni),
		.re(reg2hw_ctrl_i[11]),
		.we(we_i),
		.wd({ctrl_wd[10-:3]}),
		.de(1'b0),
		.d(1'sb0),
		.qe(),
		.q(hw2reg_ctrl_o[7-:3]),
		.qs(),
		.ds(),
		.phase(phase_key_len),
		.err_update(err_update_key_len),
		.err_storage(err_storage_key_len)
	);
	localparam [0:0] aes_reg_pkg_AES_CTRL_SHADOWED_SIDELOAD_RESVAL = 1'h0;
	prim_subreg_shadow #(
		.DW(1),
		.SwAccess(3'd2),
		.RESVAL(aes_reg_pkg_AES_CTRL_SHADOWED_SIDELOAD_RESVAL)
	) u_ctrl_reg_shadowed_sideload(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.rst_shadowed_ni(rst_shadowed_ni),
		.re(reg2hw_ctrl_i[8]),
		.we(we_i),
		.wd(ctrl_wd[11]),
		.de(1'b0),
		.d(1'sb0),
		.qe(),
		.q(hw2reg_ctrl_o[4]),
		.qs(),
		.ds(),
		.phase(phase_key_sideload),
		.err_update(err_update_sideload),
		.err_storage(err_storage_sideload)
	);
	localparam [2:0] aes_reg_pkg_AES_CTRL_SHADOWED_PRNG_RESEED_RATE_RESVAL = 3'h1;
	prim_subreg_shadow #(
		.DW(aes_pkg_AES_PRNGRESEEDRATE_WIDTH),
		.SwAccess(3'd2),
		.RESVAL(aes_reg_pkg_AES_CTRL_SHADOWED_PRNG_RESEED_RATE_RESVAL)
	) u_ctrl_reg_shadowed_prng_reseed_rate(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.rst_shadowed_ni(rst_shadowed_ni),
		.re(reg2hw_ctrl_i[3]),
		.we(we_i),
		.wd({ctrl_wd[14-:3]}),
		.de(1'b0),
		.d(1'sb0),
		.qe(),
		.q(hw2reg_ctrl_o[3-:3]),
		.qs(),
		.ds(),
		.phase(phase_prng_reseed_rate),
		.err_update(err_update_prng_reseed_rate),
		.err_storage(err_storage_prng_reseed_rate)
	);
	localparam [0:0] aes_reg_pkg_AES_CTRL_SHADOWED_MANUAL_OPERATION_RESVAL = 1'h0;
	prim_subreg_shadow #(
		.DW(1),
		.SwAccess(3'd2),
		.RESVAL(aes_reg_pkg_AES_CTRL_SHADOWED_MANUAL_OPERATION_RESVAL)
	) u_ctrl_reg_shadowed_manual_operation(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.rst_shadowed_ni(rst_shadowed_ni),
		.re(reg2hw_ctrl_i[0]),
		.we(we_i),
		.wd(ctrl_wd[15]),
		.de(1'b0),
		.d(1'sb0),
		.qe(),
		.q(hw2reg_ctrl_o[-0]),
		.qs(),
		.ds(),
		.phase(phase_manual_operation),
		.err_update(err_update_manual_operation),
		.err_storage(err_storage_manual_operation)
	);
	assign phase_o = ((((phase_operation | phase_mode) | phase_key_len) | phase_key_sideload) | phase_prng_reseed_rate) | phase_manual_operation;
	assign err_update_o = ((((err_update_operation | err_update_mode) | err_update_key_len) | err_update_sideload) | err_update_prng_reseed_rate) | err_update_manual_operation;
	assign err_storage_o = ((((err_storage_operation | err_storage_mode) | err_storage_key_len) | err_storage_sideload) | err_storage_prng_reseed_rate) | err_storage_manual_operation;
	assign operation_o = sv2v_cast_E41EB(hw2reg_ctrl_o[15-:2]);
	assign mode_o = sv2v_cast_BC361(hw2reg_ctrl_o[13-:6]);
	assign key_len_o = sv2v_cast_340F2(hw2reg_ctrl_o[7-:3]);
	assign sideload_o = hw2reg_ctrl_o[4];
	assign prng_reseed_rate_o = sv2v_cast_72367(hw2reg_ctrl_o[3-:3]);
	assign manual_operation_o = hw2reg_ctrl_o[-0];
endmodule

module prim_subreg_shadow (
	clk_i,
	rst_ni,
	rst_shadowed_ni,
	re,
	we,
	wd,
	de,
	d,
	qe,
	q,
	ds,
	qs,
	phase,
	err_update,
	err_storage
);
	parameter signed [31:0] DW = 32;
	parameter [2:0] SwAccess = 3'd0;
	parameter [DW - 1:0] RESVAL = 1'sb0;
	parameter [0:0] Mubi = 1'b0;
	input clk_i;
	input rst_ni;
	input rst_shadowed_ni;
	input re;
	input we;
	input [DW - 1:0] wd;
	input de;
	input [DW - 1:0] d;
	output wire qe;
	output wire [DW - 1:0] q;
	output wire [DW - 1:0] ds;
	output wire [DW - 1:0] qs;
	output wire phase;
	output wire err_update;
	output wire err_storage;
	localparam [2:0] InvertedSwAccess = (SwAccess == 3'd4 ? 3'd5 : (SwAccess == 3'd5 ? 3'd4 : SwAccess));
	localparam [2:0] StagedSwAccess = (SwAccess == 3'd4 ? 3'd0 : (SwAccess == 3'd5 ? 3'd0 : SwAccess));
	wire phase_clear;
	reg phase_q;
	wire staged_we;
	wire shadow_we;
	wire committed_we;
	wire staged_de;
	wire shadow_de;
	wire committed_de;
	wire committed_qe;
	wire [DW - 1:0] staged_q;
	wire [DW - 1:0] shadow_q;
	wire [DW - 1:0] committed_q;
	wire [DW - 1:0] committed_qs;
	wire wr_en;
	wire [DW - 1:0] wr_data;
	prim_subreg_arb #(
		.DW(DW),
		.SwAccess(SwAccess)
	) wr_en_data_arb(
		.we(we),
		.wd(wd),
		.de(de),
		.d(d),
		.q(q),
		.wr_en(wr_en),
		.wr_data(wr_data)
	);
	assign phase_clear = (SwAccess == 3'd1 ? 1'b0 : re);
	always @(posedge clk_i or negedge rst_ni) begin : phase_reg
		if (!rst_ni)
			phase_q <= 1'b0;
		else if (wr_en && !err_storage)
			phase_q <= ~phase_q;
		else if (phase_clear || err_storage)
			phase_q <= 1'b0;
	end
	assign staged_we = (we & ~phase_q) & ~err_storage;
	assign staged_de = (de & ~phase_q) & ~err_storage;
	prim_subreg #(
		.DW(DW),
		.SwAccess(StagedSwAccess),
		.RESVAL(~RESVAL)
	) staged_reg(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.we(staged_we),
		.wd(~wr_data),
		.de(staged_de),
		.d(~d),
		.qe(),
		.q(staged_q),
		.ds(),
		.qs()
	);
	assign shadow_we = ((we & phase_q) & ~err_update) & ~err_storage;
	assign shadow_de = ((de & phase_q) & ~err_update) & ~err_storage;
	prim_subreg #(
		.DW(DW),
		.SwAccess(InvertedSwAccess),
		.RESVAL(~RESVAL)
	) shadow_reg(
		.clk_i(clk_i),
		.rst_ni(rst_shadowed_ni),
		.we(shadow_we),
		.wd(staged_q),
		.de(shadow_de),
		.d(staged_q),
		.qe(),
		.q(shadow_q),
		.ds(),
		.qs()
	);
	assign committed_we = shadow_we;
	assign committed_de = shadow_de;
	prim_subreg #(
		.DW(DW),
		.SwAccess(SwAccess),
		.RESVAL(RESVAL)
	) committed_reg(
		.clk_i(clk_i),
		.rst_ni(rst_ni),
		.we(committed_we),
		.wd(wr_data),
		.de(committed_de),
		.d(d),
		.qe(committed_qe),
		.q(committed_q),
		.ds(ds),
		.qs(committed_qs)
	);
	assign phase = phase_q;
	assign err_update = (~staged_q != wr_data ? phase_q & wr_en : 1'b0);
	assign err_storage = ~shadow_q != committed_q;
	assign qe = committed_qe;
	assign q = committed_q;
	assign qs = committed_qs;
endmodule

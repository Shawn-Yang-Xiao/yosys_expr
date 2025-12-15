module inputs (
  input wire [0:7] data_i,
  input wire [0:7] mask_i,
  output wire [0:1] i_0,
  output wire [0:1] i_1,
  output wire [0:1] i_2,
  output wire [0:1] i_3,
  output wire [0:1] i_4,
  output wire [0:1] i_5, 
  output wire [0:1] i_6,
  output wire [0:1] i_7
);
  assign i_0[0:1] = { data_i[0], mask_i[0] };
  assign i_1[0:1] = { data_i[1], mask_i[1] };
  assign i_2[0:1] = { data_i[2], mask_i[2] };
  assign i_3[0:1] = { data_i[3], mask_i[3] };
  assign i_4[0:1] = { data_i[4], mask_i[4] };
  assign i_5[0:1] = { data_i[5], mask_i[5] };
  assign i_6[0:1] = { data_i[6], mask_i[6] };
  assign i_7[0:1] = { data_i[7], mask_i[7] };
endmodule


module outputs (
  input wire [0:7] data_o,
  input wire [0:7] mask_o,
  output wire [0:1] o_0,
  output wire [0:1] o_1,
  output wire [0:1] o_2,
  output wire [0:1] o_3,
  output wire [0:1] o_4,
  output wire [0:1] o_5,
  output wire [0:1] o_6,
  output wire [0:1] o_7
); 
  assign o_0[0:1] = { data_o[0], mask_o[0] };
  assign o_1[0:1] = { data_o[1], mask_o[1] };
  assign o_2[0:1] = { data_o[2], mask_o[2] };
  assign o_3[0:1] = { data_o[3], mask_o[3] };
  assign o_4[0:1] = { data_o[4], mask_o[4] };
  assign o_5[0:1] = { data_o[5], mask_o[5] };
  assign o_6[0:1] = { data_o[6], mask_o[6] };
  assign o_7[0:1] = { data_o[7], mask_o[7] };
endmodule


module public_inputs(
  input wire [0:1] op_i,
  output wire op_i_0,
  output wire op_i_1
); 
  assign op_i_0 = op_i[0];
  assign op_i_1 = op_i[1];
endmodule


module randoms(
  input wire [0:7] prd_i,
  output wire [0:7] prd_in
);
  assign prd_in[0:7] = prd_i[0:7];

endmodule


/** 

proc ISW_AND_2: 
  inputs: a[0:2], b[0:2]
  outputs: c[0:2]
  randoms: r01, r02, r12;

  a0b1 := a[0] * b[1];
  a1b0 := a[1] * b[0];
  a0b2 := a[0] * b[2];
  a2b0 := a[2] * b[0];
  a1b2 := a[1] * b[2];
  a2b1 := a[2] * b[1];
  r10 := r01 + a0b1;
  r10 := r10 + a1b0;
  r20 := r02 + a0b2;
  r20 := r20 + a2b0;
  r21 := r12 + a1b2;
  r21 := r21 + a2b1;
  c[0] := a[0] * b[0];
  c[0] := c[0] + r01;
  c[0] := c[0] + r02;
  c[1] := a[1] * b[1];
  c[1] := c[1] + r10;
  c[1] := c[1] + r12;
  c[2] := a[2] * b[2];
  c[2] := c[2] + r20;
  c[2] := c[2] + r21;
end

*/

module ISW_AND_2_2 (
    input [2:0] a,
    input [2:0] b,
    input r01,
    input r02,
    input r12,
    output [2:0] c
);
    wire a0b1;
    wire a1b0;
    wire a0b2;
    wire a2b0;
    wire a1b2;
    wire a2b1;
    wire r10_1, r10_2;
    wire r20_1, r20_2;
    wire r21_1, r21_2;
    wire c0_1, c0_2;
    wire c1_1, c1_2;
    wire c2_1, c2_2;

    assign a0b1 = a[0] & b[1];
    assign a1b0 = a[1] & b[0];
    assign a0b2 = a[0] & b[2];
    assign a2b0 = a[2] & b[0];
    assign a1b2 = a[1] & b[2];
    assign a2b1 = a[2] & b[1];

    assign r10_1 = r01 ^ a0b1;
    assign r10_2 = r10_1 ^ a1b0;
    assign r20_1 = r02 ^ a0b2;
    assign r20_2 = r20_1 ^ a2b0;
    assign r21_1 = r12 ^ a1b2;
    assign r21_2 = r21_1 ^ a2b1;

    assign c0_1 = a[0] & b[0];
    assign c0_2 = c0_1 ^ r01;
    assign c[0] = c0_2 ^ r02;
    assign c1_1 = a[1] & b[1];
    assign c1_2 = c1_1 ^ r10_2;
    assign c[1] = c1_2 ^ r12;
    assign c2_1 = a[2] & b[2];
    assign c2_2 = c2_1 ^ r20_2;
    assign c[2] = c2_2 ^ r21_2;

endmodule



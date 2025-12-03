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



module ISW_AND_2 (
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
    wire r10;
    wire r20;
    wire r21;

    assign a0b1 = a[0] & b[1];
    assign a1b0 = a[1] & b[0];
    assign a0b2 = a[0] & b[2];
    assign a2b0 = a[2] & b[0];
    assign a1b2 = a[1] & b[2];
    assign a2b1 = a[2] & b[1];

    assign r10 = r01 ^ a0b1 ^ a1b0;
    assign r20 = r02 ^ a0b2 ^ a2b0;
    assign r21 = r12 ^ a1b2 ^ a2b1;

    assign c[0] = (a[0] & b[0]) ^ r01 ^ r02;
    assign c[1] = (a[1] & b[1]) ^ r10 ^ r12;
    assign c[2] = (a[2] & b[2]) ^ r20 ^ r21;

endmodule


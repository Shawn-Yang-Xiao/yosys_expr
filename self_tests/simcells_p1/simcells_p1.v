
//  |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
//-
//-     $_BUF_ (A, Y)
//* group comb_simple
//-
//- A buffer. This cell type is always optimized away by the opt_clean pass.
//-
//- Truth table:    A | Y
//-                ---+---
//-                 0 | 0
//-                 1 | 1
//-
module \$_BUF_ (A, Y);
input A;
output Y;
assign Y = A;
endmodule

//  |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
//-
//-     $_NOT_ (A, Y)
//* group comb_simple
//-
//- An inverter gate.
//-
//- Truth table:    A | Y
//-                ---+---
//-                 0 | 1
//-                 1 | 0
//-
module \$_NOT_ (A, Y);
input A;
output Y;
assign Y = ~A;
endmodule

//  |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
//-
//-     $_AND_ (A, B, Y)
//* group comb_simple
//-
//- A 2-input AND gate.
//-
//- Truth table:    A B | Y
//-                -----+---
//-                 0 0 | 0
//-                 0 1 | 0
//-                 1 0 | 0
//-                 1 1 | 1
//-
module \$_AND_ (A, B, Y);
input A, B;
output Y;
assign Y = A & B;
endmodule

//  |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
//-
//-     $_NAND_ (A, B, Y)
//* group comb_simple
//-
//- A 2-input NAND gate.
//-
//- Truth table:    A B | Y
//-                -----+---
//-                 0 0 | 1
//-                 0 1 | 1
//-                 1 0 | 1
//-                 1 1 | 0
//-
module \$_NAND_ (A, B, Y);
input A, B;
output Y;
assign Y = ~(A & B);
endmodule

//  |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
//-
//-     $_OR_ (A, B, Y)
//* group comb_simple
//-
//- A 2-input OR gate.
//-
//- Truth table:    A B | Y
//-                -----+---
//-                 0 0 | 0
//-                 0 1 | 1
//-                 1 0 | 1
//-                 1 1 | 1
//-
module \$_OR_ (A, B, Y);
input A, B;
output Y;
assign Y = A | B;
endmodule

//  |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
//-
//-     $_NOR_ (A, B, Y)
//* group comb_simple
//-
//- A 2-input NOR gate.
//-
//- Truth table:    A B | Y
//-                -----+---
//-                 0 0 | 1
//-                 0 1 | 0
//-                 1 0 | 0
//-                 1 1 | 0
//-
module \$_NOR_ (A, B, Y);
input A, B;
output Y;
assign Y = ~(A | B);
endmodule

//  |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
//-
//-     $_XOR_ (A, B, Y)
//* group comb_simple
//-
//- A 2-input XOR gate.
//-
//- Truth table:    A B | Y
//-                -----+---
//-                 0 0 | 0
//-                 0 1 | 1
//-                 1 0 | 1
//-                 1 1 | 0
//-
module \$_XOR_ (A, B, Y);
input A, B;
output Y;
assign Y = A ^ B;
endmodule

//  |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
//-
//-     $_XNOR_ (A, B, Y)
//* group comb_simple
//-
//- A 2-input XNOR gate.
//-
//- Truth table:    A B | Y
//-                -----+---
//-                 0 0 | 1
//-                 0 1 | 0
//-                 1 0 | 0
//-                 1 1 | 1
//-
module \$_XNOR_ (A, B, Y);
input A, B;
output Y;
assign Y = ~(A ^ B);
endmodule

//  |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
//-
//-     $_ANDNOT_ (A, B, Y)
//* group comb_combined
//-
//- A 2-input AND-NOT gate.
//-
//- Truth table:    A B | Y
//-                -----+---
//-                 0 0 | 0
//-                 0 1 | 0
//-                 1 0 | 1
//-                 1 1 | 0
//-
module \$_ANDNOT_ (A, B, Y);
input A, B;
output Y;
assign Y = A & (~B);
endmodule

//  |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
//-
//-     $_ORNOT_ (A, B, Y)
//* group comb_combined
//-
//- A 2-input OR-NOT gate.
//-
//- Truth table:    A B | Y
//-                -----+---
//-                 0 0 | 1
//-                 0 1 | 0
//-                 1 0 | 1
//-                 1 1 | 1
//-
module \$_ORNOT_ (A, B, Y);
input A, B;
output Y;
assign Y = A | (~B);
endmodule


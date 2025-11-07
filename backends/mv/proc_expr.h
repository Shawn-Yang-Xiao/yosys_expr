
#include <memory> // support shared_ptr
// define expr

struct Var {
    int id;
    std::string name;
    Var(int i, const std::string &n) : id(i), name(n) { } 
};

enum class OpKind {
    Add,
    Mul,
    Neg,
    Other
};

struct Operator {
    int id;
    std::string name;
    bool is_bij;
    OpKind kind;
    Operator(int i, const std::string &n, bool bij, OpKind k) 
        : id(i), name(n), is_bij(bij), kind(op_kind) { }
};

struct Expr;

using ExprPtr = std::shared_ptr<Expr>;  // support shared subexpression
// QUESTION: Is this necessary?

/*
and expr_node =
| Etop
| Ernd   of rnd
| Eshare of param * int * var (* the var is the original name *)
| Epub   of var
| Eop1   of operator * expr
| Eop2   of operator * expr * expr
| Eop    of bool * operator * expr array
  (* Invariant [Eop(b,es)]
       if b is true there is no duplicate in the array es *)
| Econst of bool
*/


struct Expr {
    int e_id;
    enum class NodeType {
        ETop,
        Ernd,
        Eshare,
        Epub,
        Eop1,
        Eop2,
        Eop,
        Econst
    }type; 
    Var* var = nullptr;         // for Ernd, Epub, Eshare
    int share_index = 0;        // for Eshare
    Var* share_param = nullptr; // for Eshare
    Operator* op = nullptr;     // for Eop1, Eop2, Eop
    ExprPtr arg1, arg2;         // for Eop1, Eop2
    std::vector<ExprPtr> args;  // for Eop
    bool const_val = false;     // for Econst

};


struct Instruction {
    enum class InstructionKind {

    } kind;
};

struct ModuleExpr {
    RTLIL::IdString name;
    std::vector<RTLIL::IdString> input_ports;
    RTLIL::IdString output_port;
    std::vector<Instruction> instructions;
};

struct DesignExpr {};






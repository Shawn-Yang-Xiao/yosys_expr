
#ifndef PROC_EXPR_H
#define PROC_EXPR_H

#include <memory> // support shared_ptr
#include <cstring>
#include <vector>


#include "kernel/rtlil.h"
#include "kernel/yosys.h"

USING_YOSYS_NAMESPACE

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
        : id(i), name(n), is_bij(bij), kind(k) { }
};

struct Expr;

using ExprPtr = std::shared_ptr<Expr>;  // support shared subexpression
// QUESTION: Is this necessary?


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
        IK_subst,   // := 
        IK_glitch,  // =![ ] 
        Leak,       // leak instruction 
        MCall       // call a module
    } kind;
    /*
    in maskVerif, parsetree.ml, definition
    type instr_kind =
        | IK_subst (:=)
        | IK_hide (= [ ])
        | IK_sub (=)
        | IK_glitch (=![ ])
        | IK_noleak (<-)
    */
};

struct ModuleExpr {
    RTLIL::IdString name;
    std::vector<RTLIL::IdString> input_ports;
    RTLIL::IdString output_port;
    std::vector<Instruction> instructions;
};

struct DesignExpr {
    dict<RTLIL::IdString, ModuleExpr*> modules_;
    RTLIL::IdString top_module;
};

// For sorting cells in simcell lib modules. 
// Record predecessor and successors of a wire in cell
struct WireConn {
    RTLIL::IdString wire_name;
    RTLIL::IdString pred;
    std::vector<RTLIL::IdString> succs;
};


#endif // PROC_EXPR_H

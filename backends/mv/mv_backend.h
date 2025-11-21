#ifndef MV_BACKEND_H
#define MV_BACKEND_H


#include <memory> // support shared_ptr
#include <cstring>
#include <vector>
#include <set>


#include "kernel/rtlil.h"
#include "kernel/yosys.h"

YOSYS_NAMESPACE_BEGIN

struct MV_BACKEND::HwVar {
    enum VarKind { WIRE, CONST } kind;
    std::string wire_name;      // if kind == WIRE
    bool const_val = false;     // if kind == CONST

    explicit HwVar(std::string &w) : kind(WIRE), wire_name(w) { };
    explicit HwVar(bool b) : kind(CONST), const_val(b) { };

    static HwVar wire(const std::string &name) { return HwVar(name); }
    static HwVar constant(bool var) { return HwVar(val); }
};


enum class MV_BACKEND::Operator {
    ADD,
    MUL,
    NEG,
    OTHER
};


struct MV_BACKEND::HwExpr {
    enum class ExprType {
        VAR,
        OP1,
        OP2,
        OPN
    } type;
    std::vector<HwExpr*> args;  // valid when type != VAR
    Operator op;                // valid when type != VAR
    HwVar var;                  // valid when type == VAR 

    // Is there problem with deep copy or shallow copy? 

    static HwExpr var(const HwVar& v) {
        HwExpr ret;
        ret.type = VAR;
        ret.var = v;
        return ret;
    }
    static HwExpr unary(Operator op, HwExpr* a) {
        HwExpr ret;
        ret.type = OP1;
        ret.op = op;
        ret.args.push_back(a);
        return ret;
    }
    static HwExpr binary(Operator op, HwExpr* a, HwExpr* b) {
        HwExpr ret;
        ret.type = OP2;
        ret.op = op;
        ret.args.push_back(a);
        ret.args.push_back(b);
        return ret;
    }
    static HwExpr nary(Operator op, std::vector<HwExpr*> operands) {
        HwExpr ret;
        ret.type = OPN;
        ret.op = op;
        ret.args = operands;
        return ret;
    }
};


struct MV_BACKEND::HwInstruction {
    enum InstrKind {
        IK_subst,   // :=
        IK_glitch,  // =![]
        Leak,       // leak instruction
        MCall,      // call a module
    } kind;
    // in maskVerif, parsetree.ml, definition
    // type instr_kind = 
    //     | IK_subst (:=)
    //     | IK_hide (=[])
    //     | IK_sub (=)
    //     | IK_glitch (=![])
    //     | IK_noleak (<-)
    // for subst and glitch
    HwVar lhs;
    HwExpr rhs;
    // for leak
    std::string leak_name;
    std::set<Expr> leak_exprs;
    // for mcall
    std::string callee_module;
    std::vector< std::pair<std::string, std::HwExpr> > port_bindings;

    static HwInstruction subst(const HwVar& lhs, const HwExpr& rhs){
        HwInstruction inst;
        inst.kind = IK_subst;
        inst.lhs = lhs;
        assert(lhs.kind == HwVar::WIRE);
        inst.rhs = rhs;
        return inst;
    }

    static HwInstruction glitch(const HwVar& lhs, const HwExpr& rhs) {
        HwInstruction inst;
        inst.kind = IK_glitch;
        inst.lhs = lhs;
        assert(lhs.kind == HwVar::WIRE);
        inst.rhs = rhs;
        return inst;
    }

    static HwInstruction leak(const std::string& str, const std::set<Expr> expr_set) {
        HwInstruction inst;
        inst.kind = IK_leak;
        inst.leak_name = str;
        inst.leak_exprs = expr_set;
        return inst;
    }

    static HwInstruction mcall(cosnt std::string& mod_name, std::vector< std::pair<std::string, std::HwExpr> > bindings) {
        HwInstruction inst;
        inst.kind = IK_mcall;
        inst.callee_module = mod_name;
        inst.port_bindings = bindings;
        return inst;
    }
};




struct MV_BACKEND::HwCellDef {
    std::string cell_type;  // cell type name, like $_XNOR_
    std::vector<std::string> inputs;
    std::string output;
    std::vector<HwInstruction> instructions; 
};


// define expr
struct MV_BACKEND::Var {
    std::string name; 
    Var(const std::string &n) : name(n) { }

    bool operattor==(const Var &b) const { return name == b.name; }
    Hasher hash_into(Hasher h) const { return mkhash(h,nmae.c_str()) };
};
// FIXME: there is no const, what if a cell input is const
// how does maskVerif deal with const in Ilang file?


enum class OpKind {
    Add,
    Mul,
    Neg,
    Other
};

struct Operator {
    bool is_bij;
    OpKind kind;
    Operator( bool bij, OpKind k) 
        : is_bij(bij), kind(k) { }
};

struct Expr;

// using ExprPtr = std::shared_ptr<Expr>;  // support shared subexpression
// QUESTION: Is this necessary?


struct Expr {
    enum class Op {
        VAR,
        CONST,
        OP1,
        OP2,
        OPN
    };
    Operator op;
    Var* var; // when the expr is a var
    bool const_val = false; // when expr is a const, true or false
    std::vector<Expr*> args;
};

/*
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
*/


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
    Var lhs;       // left-hand side variable, used in IK_subst and IK_glitch
    Expr* rhs;    // right-hand side expression
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


YOSYS_NAMESPACE_END

#endif // MV_BACKEND_H
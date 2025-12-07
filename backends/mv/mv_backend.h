#ifndef MV_BACKEND_H
#define MV_BACKEND_H


#include <memory> // support shared_ptr
#include <string>
#include <vector>
#include <set>
#include <cassert>


#include "kernel/rtlil.h"
#include "kernel/yosys.h"

YOSYS_NAMESPACE_BEGIN

namespace MV_BACKEND {

struct HwVar {
    enum VarKind { WIRE, CONST } kind;
    std::string wire_name;      // if kind == WIRE
    bool with_offset = false;
    int offset;
    bool const_val = false;     // if kind == CONST

    HwVar() : kind(CONST), const_val(false) { }

    explicit HwVar(const std::string &w, bool wf, int i) : kind(WIRE), wire_name(w), with_offset(wf), offset(i) { };
    explicit HwVar(bool b) : kind(CONST), const_val(b) { };

    static HwVar make_single_wire(const std::string &name) { return HwVar(name, false, 0); }
    static HwVar make_multi_wire(const std::string &name, int i) { return HwVar(name, true, i); }
    static HwVar make_const(bool val) { return HwVar(val); }
};


enum class Operator {
    ADD,    // bit xor
    MUL,    // bit and
    NEG,
    OTHER
};


struct HwExpr {
    enum class ExprType {
        VAR,
        OP1,
        OP2,
        OPN
    } type;
    std::vector<HwExpr> args;   // valid when type != VAR
    Operator op;                // valid when type != VAR
    HwVar var;                  // valid when type == VAR 

    // Is there problem with deep copy or shallow copy? 
    // Is default construction method needed?
    HwExpr() : type(ExprType::VAR), op(Operator{}), var() { }

    explicit HwExpr(ExprType et, std::vector<HwExpr> exprVec, Operator op, HwVar hv) : type(et), args(exprVec), op(op), var(hv) { };

    static HwExpr make_var(const HwVar& v) {
        return HwExpr{
            ExprType::VAR,
            {},
            Operator {},
            v
        };
    }

    static HwExpr make_unary(Operator op, HwExpr a) {
        return HwExpr {
            ExprType::OP1,
            {a},
            op,
            HwVar{ }
        };
    }

    static HwExpr make_binary(Operator op, HwExpr a, HwExpr b) {
        return HwExpr {
            ExprType::OP2,
            {a, b},
            op,
            HwVar{ }
        };
    }

    static HwExpr make_nary(Operator op, std::vector<HwExpr> operands) {
        return HwExpr {
            ExprType::OPN,
            std::move(operands),
            op,
            HwVar{ }
        };
    }
};


struct HwInstruction {
    enum InstrKind {
        IK_subst,   // :=
        IK_glitch,  // =![]
        IK_leak,       // leak instruction
        IK_mcall,      // call a module
    } kind;
    // in maskVerif, parsetree.ml, definition
    // type instr_kind = 
    //     | IK_subst (:=)
    //     | IK_hide (=[])
    //     | IK_sub (=)
    //     | IK_glitch (=![])
    //     | IK_noleak (<-)
    std::string name; // instruction name
    // for subst and glitch
    HwVar lhs;
    HwExpr rhs;
    // for leak
    std::string leak_name;
    std::vector<HwExpr> leak_exprs; // This will likely be replaced by value expressions.
    // manually remove duplicates before assignment
    // for mcall
    std::string callee_module;
    std::vector< std::pair<std::string, HwVar> > port_bindings;

    HwInstruction() : kind(InstrKind::IK_subst), name(""), lhs(), rhs() {}

    explicit HwInstruction(InstrKind ik, std::string n, HwVar lhv, HwExpr rhe, std::string lkn, std::vector<HwExpr> lkes, std::string cem, std::vector< std::pair<std::string, HwVar> > pbs) : kind(ik), name(n), lhs(lhv), rhs(rhe), leak_name(lkn), leak_exprs(lkes), callee_module(cem), port_bindings(pbs) { };

    static HwInstruction make_subst(const std::string n, const HwVar& lhs, const HwExpr& rhs){
        assert(lhs.kind == HwVar::VarKind::WIRE);
        return HwInstruction {
            InstrKind::IK_subst,
            n,
            lhs,
            rhs,
            "",
            {},
            "",
            {}
        };
    }

    static HwInstruction make_glitch(const std::string n, const HwVar& lhs, const HwExpr& rhs) {
        assert(lhs.kind == HwVar::VarKind::WIRE);
        return HwInstruction {
            InstrKind::IK_glitch,
            n,
            lhs,
            rhs,
            "",
            {},
            "",
            {}
        };
    }

    static HwInstruction make_leak(const std::string n, const std::string& str, const std::vector<HwExpr> expr_vec) {
        return HwInstruction{
            InstrKind::IK_leak,
            n,
            {},
            {},
            str,
            std::move(expr_vec), // this should be a set, manually remove duplicates beforehead
            "",
            {}
        };
    }

    static HwInstruction make_mcall(const std::string n, const std::string& mod_name, std::vector< std::pair<std::string, HwVar> > bindings) {
        return HwInstruction {
            InstrKind::IK_mcall,
            n,
            {},
            {},
            "",
            {},
            mod_name,
            std::move(bindings)
        };
    }
};




struct HwCellDef {
    std::string cell_type;  // cell type name, like $_XNOR_
    std::vector<std::string> inputs;
    std::string output;
    std::vector<HwInstruction> instructions; 

    HwCellDef() : cell_type(""), inputs({}), output(""), instructions({}) { }

    explicit HwCellDef(std::string ct, std::vector<std::string> ins, std::string out, std::vector<HwInstruction> insts) : cell_type(ct), inputs(ins), output(out), instructions(insts) { };
    
    // construction methods
    static HwCellDef make_cell(std::string ct, std::vector<std::string> ins, std::string out, std::vector<HwInstruction> insts) {
        return HwCellDef {
            ct,
            std::move(ins),
            out,
            std::move(insts)
        };
    }

};

struct MultiBitSignal {
    // width, input, output, public, upto etc.
    std::string signal_name;
    int width;
    int start_offset;
    bool upto;
    bool input_port = false;
    bool output_port = false;


    MultiBitSignal() : signal_name(""), width(0), start_offset(0), upto(false), input_port(false), output_port(false) {};

    explicit MultiBitSignal(std::string sn, int w, int so, bool u, bool ip, bool op) : signal_name(sn), width(w), start_offset(so), upto(u), input_port(ip), output_port(op) { };

    static MultiBitSignal make_onebit(std::string sn, bool ip, bool op) {
        return MultiBitSignal {
            sn,
            1,
            0,
            false,
            ip,
            op
        };
    }

    static MultiBitSignal make_multibit(std::string sn, int w, int so, bool u, bool ip, bool op) {
        return MultiBitSignal {
            sn,
            w,
            so,
            u,
            ip,
            op
        };
    }
};


struct HwModuleDef {
    std::string module_name;
    std::vector<MultiBitSignal> inputs;
    std::vector<MultiBitSignal> outputs;
    // maybe there are public and other signals
    std::vector<HwInstruction> instructions;

    HwModuleDef() : module_name(""), inputs({}), outputs({}), instructions({}) { };

    explicit HwModuleDef(std::string mn, std::vector<MultiBitSignal> ins, std::vector<MultiBitSignal> outs, std::vector<HwInstruction> insts) : module_name(mn), inputs(ins), outputs(outs), instructions(insts) { };

    static HwModuleDef make_module(std::string mn, std::vector<MultiBitSignal> ins, std::vector<MultiBitSignal> outs, std::vector<HwInstruction> insts) {
        return HwModuleDef {
            mn,
            std::move(ins),
            std::move(outs),
            std::move(insts)
        };
    }
};



} // namespace MV_BACKEND

YOSYS_NAMESPACE_END

#endif // MV_BACKEND_H
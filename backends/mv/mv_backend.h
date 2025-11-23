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
    bool const_val = false;     // if kind == CONST

    HwVar() : kind(CONST), const_val(false) { }

    explicit HwVar(const std::string &w) : kind(WIRE), wire_name(w) { };
    explicit HwVar(bool b) : kind(CONST), const_val(b) { };

    static HwVar make_wire(const std::string &name) { return HwVar(name); }
    static HwVar make_const(bool val) { return HwVar(val); }
};


enum class Operator {
    ADD,
    MUL,
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
    std::vector<HwExpr*> args;  // valid when type != VAR
    Operator op;                // valid when type != VAR
    HwVar var;                  // valid when type == VAR 

    // Is there problem with deep copy or shallow copy? 
    // Is default construction method needed?
    HwExpr() : type(ExprType::VAR), op(Operator{}), var() { }

    explicit HwExpr(ExprType et, std::vector<HwExpr*> exprVec, Operator op, HwVar hv) : type(et), args(exprVec), op(op), var(hv) { };

    static HwExpr make_var(const HwVar& v) {
        return HwExpr{
            ExprType::VAR,
            {},
            Operator {},
            v
        };
    }

    static HwExpr make_unary(Operator op, HwExpr* a) {
        return HwExpr {
            ExprType::OP1,
            {a},
            op,
            HwVar{ }
        };
    }

    static HwExpr make_binary(Operator op, HwExpr* a, HwExpr* b) {
        return HwExpr {
            ExprType::OP2,
            {a, b},
            op,
            HwVar{ }
        };
    }

    static HwExpr make_nary(Operator op, std::vector<HwExpr*> operands) {
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
    // for subst and glitch
    HwVar lhs;
    HwExpr rhs;
    // for leak
    std::string leak_name;
    std::vector<HwExpr> leak_exprs; // This will likely be replaced by value expressions.
    // manually remove duplicates before assignment
    // for mcall
    std::string callee_module;
    std::vector< std::pair<std::string, HwExpr> > port_bindings;

    HwInstruction() : kind(InstrKind::IK_subst), lhs(), rhs() {}

    explicit HwInstruction(InstrKind ik, HwVar lhv, HwExpr rhe, std::string lkn, std::vector<HwExpr> lkes, std::string cem, std::vector< std::pair<std::string, HwExpr> > pbs) : kind(ik), lhs(lhv), rhs(rhe), leak_name(lkn), leak_exprs(lkes), callee_module(cem), port_bindings(pbs) { };

    static HwInstruction make_subst(const HwVar& lhs, const HwExpr& rhs){
        assert(lhs.kind == HwVar::VarKind::WIRE);
        return HwInstruction {
            InstrKind::IK_subst,
            lhs,
            rhs,
            "",
            {},
            "",
            {}
        };
    }

    static HwInstruction make_glitch(const HwVar& lhs, const HwExpr& rhs) {
        assert(lhs.kind == HwVar::VarKind::WIRE);
        return HwInstruction {
            InstrKind::IK_glitch,
            lhs,
            rhs,
            "",
            {},
            "",
            {}
        };
    }

    static HwInstruction make_leak(const std::string& str, const std::vector<HwExpr> expr_vec) {
        return HwInstruction{
            InstrKind::IK_leak,
            {},
            {},
            str,
            std::move(expr_vec), // this should be a set, manually remove duplicates beforehead
            "",
            {}
        };
    }

    static HwInstruction make_mcall(const std::string& mod_name, std::vector< std::pair<std::string, HwExpr> > bindings) {
        return HwInstruction {
            InstrKind::IK_mcall,
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
};


const pool<string> mv_keywords();

} // namespace MV_BACKEND

YOSYS_NAMESPACE_END

#endif // MV_BACKEND_H
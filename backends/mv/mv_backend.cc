/*
  1. Define expr 
  2. Read simcells.v into rtlil internal representation
  3. Transform into expr formation
  4. Calculate leak expr
*/


#include <cstring>
#include <vector>
#include <queue>
#include <set>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

#include "kernel/yosys.h"
#include "kernel/rtlil.h"
#include "mv_backend.h"
// #include "proc_expr.h"
// #include "proc_expr.cc"

USING_YOSYS_NAMESPACE

using namespace MV_BACKEND; 


PRIVATE_NAMESPACE_BEGIN


// print structs defined in mv_backend.h
/*
void dump_hwvar(std::ostream& f, HwVar hv) {
    if (hv.kind == HwVar::VarKind::WIRE) {
        // WIRE
        f << stringf("wire %s", hv.wire_name);
        if (hv.with_offset) {
            f << stringf(" [%d]", hv.offset);
        }
    }
    else {
        // CONST
        if (hv.const_val) {
            f << stringf("true");
        }
        else {
            f << stringf("false");
        }
    }
}


void dump_operator(std::ostream& f, Operator op) {
    if (op == Operator::ADD) {
        f << stringf("+");
    }
    else if (op == Operator::MUL) {
        f << stringf("*");
    }
    else if (op == Operator::NEG) {
        f << stringf("~");
    }
    else {
        // Operator::OTHER
        f << stringf("OTHER OPERATOR");
    }
}


void dump_hwexpr(std::ostream& f, HwExpr he) {
    if(he.type == HwExpr::ExprType::VAR) {
        dump_hwvar(f, he.var);
    }
    else if (he.type == HwExpr::ExprType::OP1) {
        dump_operator(f, he.op);
        f << stringf(" ");
        if ( (he.args[0]).type == HwExpr::ExprType::VAR ) {
            dump_hwexpr( f, he.args[0] );
        }
        else {
            f << stringf("(");
            dump_hwexpr( f, he.args[0] );
            f << stringf(")");
        }
    }
    else if (he.type == HwExpr::ExprType::OP2) {
        if ( (he.args)[0].type == HwExpr::ExprType::VAR ) {
            dump_hwexpr( f, he.args[0] );
        }
        else {
            f << stringf("(");
            dump_hwexpr( f, he.args[0] );
            f << stringf(")");
        }
        f << stringf(" ");
        dump_operator(f, he.op);
        f << stringf(" ");
        if ( (he.args[1]).type == HwExpr::ExprType::VAR ) {
            dump_hwexpr( f, he.args[1] );
        }
        else {
            f << stringf("(");
            dump_hwexpr( f, he.args[1] );
            f << stringf(")");
        }
    }
    else {
        // HwExpr::ExprType::OPN
        dump_operator(f, he.op);
        f << stringf("(");
        for (auto it = he.args.begin(); it != he.args.end(); it++) {
            if (it != he.args.begin()) {
                f << stringf(", ");
            }
            if ( (*it).type == HwExpr::ExprType::VAR ) {
                dump_hwexpr(f, *it);
            }
            else {
                f << stringf("(");
                dump_hwexpr(f, *it);
                f << stringf(")");
            }
            
        }
        f << stringf(")");
    }
}


void dump_hwinstruction(std::ostream& f, HwInstruction hi) {
    if(hi.kind == HwInstruction::InstrKind::IK_subst) {
        dump_hwvar(f, hi.lhs);
        f << stringf(" := ");
        dump_hwexpr(f, hi.rhs);
        f << stringf("; ");
        f << stringf("%s", hi.name);
    }
    else if (hi.kind == HwInstruction::InstrKind::IK_glitch) {
        dump_hwvar(f, hi.lhs);
        f << stringf(" =! [");
        dump_hwexpr(f, hi.rhs);
        f << stringf("]; ");
        f << stringf("(* %s *)", hi.name);
    }
    else if (hi.kind == HwInstruction::InstrKind::IK_leak) {
        f << stringf("leak %s(", hi.leak_name);
        for(auto it = hi.leak_exprs.begin(); it != hi.leak_exprs.end(); it++) {
            if (it != hi.leak_exprs.begin()) {
                f << stringf(", ");
            }
            if (it->type == HwExpr::ExprType::VAR) {
                dump_hwexpr(f, *it);
            }
            else{
                f << stringf("(");
                dump_hwexpr(f, *it);
                f << stringf(")");
            }
        }
        f << stringf("); ");
        f << stringf("(* %s *)", hi.name);
    }
    else {
        // IK_mcall
        f << stringf("%s: ", hi.callee_module);
        for(auto it = hi.port_bindings.begin(); it != hi.port_bindings.end(); it++) {
            if(it != hi.port_bindings.begin()) {
                f << stringf(", ");
            }
            f << stringf("port %s ", it->first);
            dump_hwvar(f, it->second);
        }
        f << stringf("; ");
        f << stringf("(* %s *)", hi.name);
    }
}


void dump_hwcelldef(std::ostream& f, HwCellDef hcd) {
    f << stringf("cell type %s:\n", hcd.cell_type);
    // print inputs
    f << stringf("    inputs: ");
    for (auto it = hcd.inputs.begin(); it != hcd.inputs.end(); it++) {
        if (it != hcd.inputs.begin()) {
            f << stringf(", ");
        }
        f << stringf("%s", *it);
    }
    f << stringf(";\n");
    // print output
    f << stringf("    output: %s;\n", hcd.output);
    // print instructions
    f << stringf("    {\n");
    for (auto it = hcd.instructions.begin(); it != hcd.instructions.end(); it++) {
        f << stringf("    ");
        dump_hwinstruction(f, *it);
        f << stringf("\n");
    }
    f << stringf("    }\n\n");
}


void dump_multibitsignal(std::ostream& f, MultiBitSignal mbs) {
    f << stringf("signal name %s, width %d, start offset %d", mbs.signal_name, mbs.width, mbs.start_offset);
    if (mbs.upto) {
        f << stringf(", upto");
    }
    else {
        f << stringf(", downto");
    }
    if (mbs.input_port) {
        f << stringf(", input port");
    }
    if (mbs.output_port) {
        f << stringf(", output port");
    }
    f << stringf("\n");
}


void dump_hwmoduledef(std::ostream& f, HwModuleDef hmd) {
    f << stringf("module name %s: \n", hmd.module_name);
    // print inputs
    f << stringf("  inputs: ");
    for (auto it = hmd.inputs.begin(); it != hmd.inputs.end(); it++) {
        if (it != hmd.inputs.begin()) {
            f << stringf(", ");
        }
        dump_multibitsignal(f, *it);
    }
    // print outputs
    f << stringf("  outputs: ");
    for (auto it = hmd.outputs.begin(); it != hmd.outputs.end(); it++) {
        if (it != hmd.outputs.begin()) {
            f << stringf(", ");
        }
        dump_multibitsignal(f, *it);
    }
    // print instructions
    f << stringf("  instructions: \n");
    for( auto it = hmd.instructions.begin(); it != hmd.instructions.end(); it++) {
        f << stringf("    ");
        dump_hwinstruction(f, *it);
        f << stringf("\n");
    }

    f << stringf("endmodule\n");
}
*/
// TODO: print into mv program, beware of the format

std::string to_hex(unsigned char c) {
    const char hex_chars[] = "0123456789abcdef";
    std::string ret = std::string(1, hex_chars[(c >> 4) & 0x0F]) + std::string(1, hex_chars[c & 0x0F]);
    return ret;
}


void nonescaped_identifier(std::ostream& f, std::string id) {
    size_t start = 0;
    if (!id.empty() && id[0] == '\\') {
        start = 1;
    }
    for (size_t i = start; i < id.size(); i++) {
        char c = id[i];
        if (isalnum(c) || c == '_') {
            f << c;
        }
        else {
            f << "_0x" << to_hex(c) << "_";
        }
    }
}


void dump_mv_hwvar(std::ostream& f, HwVar hv) {
    if (hv.kind == HwVar::VarKind::WIRE) {
        std::string wire_name = (hv.wire_name);
        nonescaped_identifier(f, wire_name);
        if (hv.with_offset) {
            f << stringf("[%d]", hv.offset);
        }
    }
    else {
        // CONST
        if (hv.const_val) {
            f << stringf("(0b1 : bool)");
        }
        else {
            f << stringf("(0b0 : bool)");
        }
    }
}


void dump_mv_operator(std::ostream& f, Operator op) {
    if (op == Operator::ADD) {
        f << stringf("+");
    }
    else if (op == Operator::MUL) {
        f << stringf("*");
    }
    else if (op == Operator::NEG) {
        f << stringf("~");
    }
    else if (op == Operator::MUX) {
        f << stringf("#_MUX_");
    }
    else if (op == Operator::MUX4) {
        f << stringf("#_MUX4_");
    }
    else if (op == Operator::MUX8) {
        f << stringf("#_MUX8_");
    }
    else if (op == Operator::MUX16) {
        f << stringf("#_MUX16_");
    }
    // EXTEND: to extend new operators
    else if (op == Operator::OTHER) {
        f << stringf("OTHER");
    }
    else {
        // unkown operator
        f << stringf("FAULT: UNSET OPERATOR PRINT METHOD\n");
    }
}


void dump_mv_hwexpr(std::ostream& f, HwExpr he) {
    if(he.type == HwExpr::ExprType::VAR) {
        dump_mv_hwvar(f, he.var);
    }
    else if (he.type == HwExpr::ExprType::OP1) {
        dump_mv_operator(f, he.op);
        f << stringf(" ");
        if ( (he.args[0]).type == HwExpr::ExprType::VAR ) {
            dump_mv_hwexpr( f, he.args[0] );
        }
        else {
            f << stringf("(");
            dump_mv_hwexpr( f, he.args[0] );
            f << stringf(")");
        }
    }
    else if (he.type == HwExpr::ExprType::OP2) {
        if ( (he.args)[0].type == HwExpr::ExprType::VAR ) {
            dump_mv_hwexpr( f, he.args[0] );
        }
        else {
            f << stringf("(");
            dump_mv_hwexpr( f, he.args[0] );
            f << stringf(")");
        }
        f << stringf(" ");
        dump_mv_operator(f, he.op);
        f << stringf(" ");
        if ( (he.args[1]).type == HwExpr::ExprType::VAR ) {
            dump_mv_hwexpr( f, he.args[1] );
        }
        else {
            f << stringf("(");
            dump_mv_hwexpr( f, he.args[1] );
            f << stringf(")");
        }
    }
    else {
        // HwExpr::ExprType::OPN
        dump_mv_operator(f, he.op);
        f << stringf("(");
        for (auto it = he.args.begin(); it != he.args.end(); it++) {
            if (it != he.args.begin()) {
                f << stringf(", ");
            }
            if ( (*it).type == HwExpr::ExprType::VAR ) {
                dump_mv_hwexpr(f, *it);
            }
            else {
                f << stringf("(");
                dump_mv_hwexpr(f, *it);
                f << stringf(")");
            }
            
        }
        f << stringf(")");
    }
}


void dump_mv_hwinstruction(std::ostream& f, HwInstruction hi) {
    if(hi.kind == HwInstruction::InstrKind::IK_subst) {
        dump_mv_hwvar(f, hi.lhs);
        f << stringf(" := ");
        dump_mv_hwexpr(f, hi.rhs);
        f << stringf("; ");
        f << stringf("(* %s *)", hi.name);
    }
    else if (hi.kind == HwInstruction::InstrKind::IK_glitch) {
        dump_mv_hwvar(f, hi.lhs);
        f << stringf(" =! [");
        dump_mv_hwexpr(f, hi.rhs);
        f << stringf("]; ");
        f << stringf("(* %s *)", hi.name);
    }
    else if (hi.kind == HwInstruction::InstrKind::IK_leak) {
        f << stringf("leak ");
        nonescaped_identifier(f, hi.leak_name);
        f << stringf("(");
        for(auto it = hi.leak_exprs.begin(); it != hi.leak_exprs.end(); it++) {
            if (it != hi.leak_exprs.begin()) {
                f << stringf(", ");
            }
            if (it->type == HwExpr::ExprType::VAR) {
                dump_mv_hwexpr(f, *it);
            }
            else{
                f << stringf("(");
                dump_mv_hwexpr(f, *it);
                f << stringf(")");
            }
        }
        f << stringf("); ");
        f << stringf("(* %s *)", hi.name);
    }
    else {
        // IK_mcall
        log("UNEXPECTED OCCASION: IK_mcall in MV backend\n");
    }
}


void dump_mv_hwmoduledef(std::ostream& f, HwModuleDef hmd) {
    f << stringf("proc ");
    nonescaped_identifier(f, hmd.module_name);
    f << stringf(":\n");
    // not to print ports, they are to be added manually
    // print instructions
    for (auto it = hmd.instructions.begin(); it != hmd.instructions.end(); it++) {
        f << stringf("  ");
        dump_mv_hwinstruction(f, *it);
        f << stringf("\n");
    }
    f << stringf("end\n");
}

/*
std::string hwinstruction_to_string(HwInstruction hi) {
    std::ostringstream oss;
    dump_hwinstruction(oss, hi);
    return oss.str();
}

std::string hwcelldef_to_string(HwCellDef hcd) {
    std::ostringstream oss;
    dump_hwcelldef(oss, hcd);
    return oss.str();
}

std::string hwmoduledef_to_string(HwModuleDef hmd) {
    std::ostringstream oss;
    dump_hwmoduledef(oss, hmd);
    return oss.str();
}
*/


/*
std::string hwmoduledef_to_mv(HwModuleDef hmd) {
    std::ostringstream oss;
    dump_mv_hwmoduledef(oss, hmd);
    return oss.str();
}
*/

// print RTLIL structs

void print_const(RTLIL::Const data, int width, int offset) {
    if (width < 0) {
        width = data.size() - offset;
    }
    if ( (data.flags & RTLIL::CONST_FLAG_STRING) == 0 || width != (int)data.size() ) {
        if (width == 32) {
            int32_t val = 0;
			for (int i = 0; i < width; i++) {
				log_assert(offset+i < (int)data.size());
				switch (data[offset+i]) {
				case State::S0: break;
				case State::S1: val |= 1 << i; break;
				default: val = -1; break;
				}
			}
			if (val >= 0) {
				log("%d", val);
				return;
			}       
        }
        log("%d'", width);
        if (data.flags & RTLIL::CONST_FLAG_SIGNED) {
			log("s");
		}
        if (data.is_fully_undef_x_only()) {
			log("x");
		} else {
            for (int i = offset+width-1; i >= offset; i--) {
				log_assert(i < (int)data.size());
				switch (data[i]) {
				case State::S0: log("0"); break;
				case State::S1: log("1"); break;
				case RTLIL::Sx: log("x"); break;
				case RTLIL::Sz: log("z"); break;
				case RTLIL::Sa: log("-"); break;
				case RTLIL::Sm: log("m"); break;
				}
			}
        }
    } else {
        log("\"");
		std::string str = data.decode_string();
		for (size_t i = 0; i < str.size(); i++) {
			if (str[i] == '\n')
				log("\\n");
			else if (str[i] == '\t')
				log("\\t");
			else if (str[i] < 32)
				log("\\%03o", (unsigned char)str[i]);
			else if (str[i] == '"')
				log("\\\"");
			else if (str[i] == '\\')
				log("\\\\");
			else
				log(str[i]);
		}
		log("\"");
    }
}


void print_sigchunk(const RTLIL::SigChunk chunk){
    if(chunk.wire == NULL) { // distinguish between const and wire
        print_const(chunk.data, chunk.width, chunk.offset);
    }
    else {
        if (chunk.width == chunk.wire->width && chunk.offset == 0) { 
            // one bit signal, should be the only result in cell
            log("%s", chunk.wire->name);
        }
        else if (chunk.width == 1) { // possible in module to connect wire and cell ports
            log("%s [%d]", chunk.wire->name, chunk.offset);
        }
        else {
            // multi bit signal
            log("PRINT A MULTI-BIT SIGNAL, CURRENTLY NOT DEALED, KEEP MONITORING.");
        }
    }
    log("  Width %d ", chunk.width);
}


void print_sigspec(RTLIL::SigSpec sig) {
    if (sig.is_chunk()) {
        print_sigchunk(sig.as_chunk());
    } else {
        log("{ ");
        for (const auto& chunk : sig.chunks() ) { //reversed(sig.chunks())
            print_sigchunk(chunk);
            log(" ");
        }
        log("}");
    }
    log(" Width %d ", sig.size());
}


void print_bit_info(RTLIL::IdString module_name, RTLIL::SigSpec sig) {
    log("In module %s, SigSpec info: ", module_name.c_str());
    print_sigspec(sig);
    log("\n");
}


void print_cell_bit_info(RTLIL::IdString module_name, RTLIL::IdString cell_name, RTLIL::SigSpec sig) {
    log("In module %s, cell %s, SigSpec info: ", module_name.c_str(), cell_name.c_str());
    print_sigspec(sig);
    log("\n");
}


void print_conn(RTLIL::SigSpec left, RTLIL::SigSpec right) {
    log("    connect " );
    print_sigspec(left);
    log(" ");
    print_sigspec(right);
    log("\n");
    

}


void print_cell(const RTLIL::Cell *cell) {
    log("    cell name %s, type %s\n", cell->name.c_str(), cell->type.c_str());
    dict<RTLIL::IdString, RTLIL::SigSpec> conn_of_cell = cell->connections_;
    for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn_pair : conn_of_cell) {
        log("      port %s: sigspec ", conn_pair.first.c_str());
        if (conn_pair.second.is_chunk()) {
            // dump sigchunk
            print_sigchunk(conn_pair.second.as_chunk());
        }
        else {
            log("{ ");
            for (const RTLIL::SigChunk &chunk : conn_pair.second.chunks()) {
                print_sigchunk(chunk);
                log(" ");
            }
            log("}");
        }
        log("\n");
    }
    log("\n");
}


void print_SigSig(std::pair<const RTLIL::SigSpec, RTLIL::SigSpec> sig_pair) {
    log("      first: %s", sig_pair.first.as_string());
    log("      second: %s", sig_pair.second.as_string());
    log("\n");
}



void print_wire(const RTLIL::Wire *wire) {
    log("  wire name %s,  ", wire->name.c_str());
    log("width %d, id %d, offset %d", wire->width, wire->port_id, wire->start_offset);
    if (wire->upto) {
        log(", upto true");
    }
    else {
        log(", upto false");
    }
    if (wire->port_input) {
        log(", input port");
    }
    if (wire->port_output) {
        log(", output port");
    }
    log("\n");
}


void print_module(const RTLIL::Module *module) {
    log("Module name: %s.\n", module->name.c_str());
    dict<RTLIL::IdString, RTLIL::Wire *> wires_of_module = module->wires_;
    dict<RTLIL::IdString, RTLIL::Cell *> cells_of_module = module->cells_;
    std::vector<RTLIL::SigSig> connections_of_module = module->connections_;
    // print wires in each module
    log("  Wires:\n");
    for (std::pair<const RTLIL::IdString, RTLIL::Wire *> wire_pair : wires_of_module) {
        log("    wire id %s ", wire_pair.first.c_str());
        print_wire(wire_pair.second);
    }
    // print connections in each module
    // log("  Connections:\n");
    // for (std::pair<RTLIL::SigSpec, RTLIL::SigSpec> conn_pair : connections_of_module) {
        // 
        // print_SigSig(conn_pair);
    // }
    log("  Connections:\n");
    for (const auto& [lhs, rhs] : module->connections()) { // borrow from rtlil backend
        RTLIL::SigSpec sigs = lhs;
        sigs.append(rhs);
        print_conn(lhs, rhs);
        log("\n");
    }
    // just use connection 
    log("  Cells:\n");
    for (std::pair<RTLIL::IdString, RTLIL::Cell *> cell_pair : cells_of_module) {
        log("    cell id %s ", cell_pair.first.c_str());
        print_cell(cell_pair.second);
    }
}


void print_design(const RTLIL::Design *design){
    dict<RTLIL::IdString, RTLIL::Module*> modules_of_simcells = design->modules_;
    for(std::pair<const RTLIL::IdString, RTLIL::Module*>module_pair : modules_of_simcells) {
        log("Module id: %s, ", module_pair.first.c_str());
        print_module(module_pair.second);
    }

}



std::string hwvar_distinguish_name(HwVar hv) {
    std::string ret;
    ret = stringf("%s_%d", hv.wire_name, hv.offset);
    return ret;
}



struct HwInstrInfo {
    std::string name;
    HwInstruction instruction;
    std::set<std::string> pred_var_names;
    std::string succ_var_name;
};

/*
void dump_hwinstrinfo(std::ostream& f, HwInstrInfo hii) {
    f << stringf("HwInstr %s:\n", hii.name);
    f << stringf("  ");
    dump_hwinstruction(f, hii.instruction);
    f << stringf("\n  preds: ");
    for (auto pvn_it = hii.pred_var_names.begin(); pvn_it != hii.pred_var_names.end(); pvn_it++) {
        if (pvn_it != hii.pred_var_names.begin()) {
            f << stringf(", ");
        }
        f << stringf("%s", *pvn_it);
    }
    f << stringf("\n  succ: ");
    f << stringf("%s\n", hii.succ_var_name);
}
*/

std::vector<HwInstrInfo> connect_to_instruction(RTLIL::SigSig conn) {
    std::vector<HwInstrInfo> ret;
    // generate instruction(s) from the connection
    // extract bits of left hand side
    std::vector<RTLIL::SigBit> lhs_bits = conn.first.bits();
    std::vector<RTLIL::SigBit> rhs_bits = conn.second.bits();
    log_assert(lhs_bits.size() == rhs_bits.size());
    for (int i = 0; i < (int)(lhs_bits.size()); i++) { 
        HwInstruction hi;
        // left hand side
        RTLIL::SigBit lhs_bit = lhs_bits[i];
        log_assert(lhs_bit.wire != NULL); // should be wire
        HwVar hv_lhs;
        // check width of the wire
        if (lhs_bit.wire->width == 1) {
            hv_lhs = HwVar::make_single_wire(lhs_bit.wire->name.c_str());
        }
        else if (lhs_bit.wire->width >= 2) {
            // multi bit wire
            hv_lhs = HwVar::make_multi_wire(lhs_bit.wire->name.c_str(), lhs_bit.offset);
        }
        else {
            log("UNEXPECTED OCCASION: connect LHS wire has invalid width.\n");
        }
        
        // right hand side
        RTLIL::SigBit rhs_bit = rhs_bits[i];
        // log_assert(rhs_bit.wire != NULL); // should be wire
        HwVar hv_rhs;
        // pred var names
        std::set<std::string> pred_names;
        // TODO: add support for const 
        if (rhs_bit.wire == NULL) {
            // const
            if (rhs_bit.data == RTLIL::State::S0) {
                hv_rhs = HwVar::make_const(false);
            }
            else if (rhs_bit.data == RTLIL::State::S1) {
                hv_rhs = HwVar::make_const(true);
            }
            else {
                log("UNEXPECTED OCCASION: connect RHS const has invalid value.\n");
            }
        }
        else {
            if (rhs_bit.wire->width == 1) {
                hv_rhs = HwVar::make_single_wire(rhs_bit.wire->name.c_str());
            }
            else if (rhs_bit.wire->width >= 2) {
                // multi bit wire
                hv_rhs = HwVar::make_multi_wire(rhs_bit.wire->name.c_str(), rhs_bit.offset);
            }
            else {
                log("UNEXPECTED OCCASION: connect RHS wire has invalid width.\n");
            }
            pred_names.insert( hwvar_distinguish_name(hv_rhs) );
        }
        std::string instr_name = hwvar_distinguish_name(hv_lhs);
        hi = HwInstruction::make_subst(instr_name, hv_lhs, HwExpr::make_var(hv_rhs));
        // generate name for the instruction, use lhs wire name and index
        
        // succ var name
        std::string succ_name = hwvar_distinguish_name(hv_lhs);

        ret.push_back( {instr_name, hi, pred_names, succ_name} );
    }
    // name of assignment instruction is correlated to lhs signal name and index (if multi bit)
    return ret;
}


HwVar get_hwvar_from_sigbit(RTLIL::SigBit sigbit, std::string cell_name, std::string port_name) {
    HwVar ret;
    if (sigbit.wire == NULL) {
        if (sigbit.data == RTLIL::State::S0) {
            ret = HwVar::make_const(false);
        }
        else if (sigbit.data == RTLIL::State::S1) {
            ret = HwVar::make_const(true);
        }
        else {
            log("UNEXPECTED OCCASION: connect RHS const has invalid value.\n");
        }
    }
    else {
        // sigbit.wire != NULL
        if (sigbit.wire->width == 1) {
            ret = HwVar::make_single_wire(sigbit.wire->name.c_str());
        }
        else if (sigbit.wire->width >= 2) {
            // multi bit wire
            ret = HwVar::make_multi_wire(sigbit.wire->name.c_str(), sigbit.offset);
        }
        else {
            log("UNEXPECTED OCCASION: port %s of cell %s has invalid width.\n", port_name.c_str(), cell_name.c_str());
        }
    }
    
    return ret;
}


HwInstrInfo simcell_to_instruction(RTLIL::Cell* cell) {
    HwInstrInfo ret;
    ret.name = cell->name.str();
    // generate an instruction from the expr
    RTLIL::IdString cell_type = cell->type;
    // distinguish between different cells, each give different result
    if (cell_type == ID($_BUF_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit rhs_bit = conn.second.bits()[0];
                if (rhs_bit.wire != NULL) {
                    HwVar rhs_var;
                    if (rhs_bit.wire->width == 1) {
                        rhs_var = HwVar::make_single_wire(rhs_bit.wire->name.c_str());
                    }
                    else if (rhs_bit.wire->width >= 2) {
                        rhs_var = HwVar::make_multi_wire(rhs_bit.wire->name.c_str(), rhs_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: RHS of BUF cell has invalid width.\n");
                    }
                    rhs = HwExpr::make_var(rhs_var);
                    ret.pred_var_names.insert(hwvar_distinguish_name(rhs_var));
                }
                else {
                    HwVar rhs_var;
                    rhs_var = get_hwvar_from_sigbit(rhs_bit,"$_BUF_", "\\A");
                    rhs = HwExpr::make_var(rhs_var);
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                if (lhs_bit.wire != NULL) {
                    if (lhs_bit.wire->width == 1) {
                        lhs = HwVar::make_single_wire(lhs_bit.wire->name.c_str());
                    }
                    else if (lhs_bit.wire->width >= 2) {
                        lhs = HwVar::make_multi_wire(lhs_bit.wire->name.c_str(), lhs_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: LHS of BUF cell has invalid width.\n");
                    }
                    ret.succ_var_name = hwvar_distinguish_name(lhs);
                }
                else {
                    log("UNEXPECTED OCCASION: LHS of BUF cell is not wire.\n");
                }
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_NOT_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit rhs_bit = conn.second.bits()[0];
                if (rhs_bit.wire != NULL) {
                    HwVar rhs_var;
                    if (rhs_bit.wire->width == 1) {
                        rhs_var = HwVar::make_single_wire(rhs_bit.wire->name.c_str());
                    }
                    else if (rhs_bit.wire->width >= 2) {
                        rhs_var = HwVar::make_multi_wire(rhs_bit.wire->name.c_str(), rhs_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: RHS of NOT cell has invalid width.\n");
                    }
                    rhs = HwExpr::make_unary(Operator::NEG, HwExpr::make_var(rhs_var));
                    ret.pred_var_names.insert(hwvar_distinguish_name(rhs_var));
                }
                else {
                    HwVar rhs_var;
                    rhs_var = get_hwvar_from_sigbit(rhs_bit,"$_NOT_", "\\A");
                    rhs = HwExpr::make_var(rhs_var);
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                if (lhs_bit.wire != NULL) {
                    if (lhs_bit.wire->width == 1) {
                        lhs = HwVar::make_single_wire(lhs_bit.wire->name.c_str());
                    }
                    else if (lhs_bit.wire->width >= 2) {
                        lhs = HwVar::make_multi_wire(lhs_bit.wire->name.c_str(), lhs_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: LHS of NOT cell has invalid width.\n");
                    }
                    ret.succ_var_name = hwvar_distinguish_name(lhs);
                }
                else {
                    log("UNEXPECTED OCCASION: LHS of NOT cell is not wire.\n");
                }
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_AND_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                if (var_a_bit.wire != NULL) {
                    if (var_a_bit.wire->width == 1) {
                        var_a = HwVar::make_single_wire(var_a_bit.wire->name.c_str());
                    }
                    else if (var_a_bit.wire->width >= 2) {
                        var_a = HwVar::make_multi_wire(var_a_bit.wire->name.c_str(), var_a_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: input A of AND cell has invalid width.\n");
                    }
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
                else {
                    var_a = get_hwvar_from_sigbit(var_a_bit,"$_AND_", "\\A");
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                if (var_b_bit.wire != NULL) {
                    if (var_b_bit.wire->width == 1) {
                        var_b = HwVar::make_single_wire(var_b_bit.wire->name.c_str());
                    }
                    else if (var_b_bit.wire->width >= 2) {
                        var_b = HwVar::make_multi_wire(var_b_bit.wire->name.c_str(), var_b_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: input B of AND cell has invalid width.\n");
                    }
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
                else {
                    var_b = get_hwvar_from_sigbit(var_b_bit,"$_AND_", "\\B");
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                if (lhs_bit.wire != NULL) {
                    if (lhs_bit.wire->width == 1) {
                        lhs = HwVar::make_single_wire(lhs_bit.wire->name.c_str());
                    }
                    else if (lhs_bit.wire->width >= 2) {
                        lhs = HwVar::make_multi_wire(lhs_bit.wire->name.c_str(), lhs_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: LHS of AND cell has invalid width.\n");
                    }
                    ret.succ_var_name = hwvar_distinguish_name(lhs);
                }
                else {
                    log("UNEXPECTED OCCASION: LHS of AND cell is not wire.\n");
                }
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            rhs = HwExpr::make_binary(Operator::MUL, HwExpr::make_var(var_a), HwExpr::make_var(var_b));
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_NAND_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                if (var_a_bit.wire != NULL) {
                    if (var_a_bit.wire->width == 1) {
                        var_a = HwVar::make_single_wire(var_a_bit.wire->name.c_str());
                    }
                    else if (var_a_bit.wire->width >= 2) {
                        var_a = HwVar::make_multi_wire(var_a_bit.wire->name.c_str(), var_a_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: input A of NAND cell has invalid width.\n");
                    }
                }
                else {
                    var_a = get_hwvar_from_sigbit(var_a_bit,"$_NAND_", "\\A");
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                if (var_b_bit.wire != NULL) {
                    if (var_b_bit.wire->width == 1) {
                        var_b = HwVar::make_single_wire(var_b_bit.wire->name.c_str());
                    }
                    else if (var_b_bit.wire->width >= 2) {
                        var_b = HwVar::make_multi_wire(var_b_bit.wire->name.c_str(), var_b_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: input B of NAND cell has invalid width.\n");
                    }
                }
                else {
                    var_b = get_hwvar_from_sigbit(var_b_bit,"$_NAND_", "\\B");
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                if (lhs_bit.wire != NULL) {
                    if (lhs_bit.wire->width == 1) {
                        lhs = HwVar::make_single_wire(lhs_bit.wire->name.c_str());
                    }
                    else if (lhs_bit.wire->width >= 2) {
                        lhs = HwVar::make_multi_wire(lhs_bit.wire->name.c_str(), lhs_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: LHS of NAND cell has invalid width.\n");
                    }
                    ret.succ_var_name = hwvar_distinguish_name(lhs);
                }
                else {
                    log("UNEXPECTED OCCASION: LHS of NAND cell is not wire.\n");
                }
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            rhs = HwExpr::make_unary(Operator::NEG, HwExpr::make_binary(Operator::MUL, HwExpr::make_var(var_a), HwExpr::make_var(var_b)));
        }
    }
    else if (cell_type == ID($_OR_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                if (var_a_bit.wire != NULL) {
                    if (var_a_bit.wire->width == 1) {
                        var_a = HwVar::make_single_wire(var_a_bit.wire->name.c_str());
                    }
                    else if (var_a_bit.wire->width >= 2) {
                        var_a = HwVar::make_multi_wire(var_a_bit.wire->name.c_str(), var_a_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: input A of OR cell has invalid width.\n");
                    }
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
                else {
                    var_a = get_hwvar_from_sigbit(var_a_bit,"$_OR_", "\\A");
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                if (var_b_bit.wire != NULL) {
                    if (var_b_bit.wire->width == 1) {
                        var_b = HwVar::make_single_wire(var_b_bit.wire->name.c_str());
                    }
                    else if (var_b_bit.wire->width >= 2) {
                        var_b = HwVar::make_multi_wire(var_b_bit.wire->name.c_str(), var_b_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: input B of OR cell has invalid width.\n");
                    }
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
                else {
                    var_b = get_hwvar_from_sigbit(var_b_bit,"$_OR_", "\\B");
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                if (lhs_bit.wire != NULL) {
                    if (lhs_bit.wire->width == 1) {
                        lhs = HwVar::make_single_wire(lhs_bit.wire->name.c_str());
                    }
                    else if (lhs_bit.wire->width >= 2) {
                        lhs = HwVar::make_multi_wire(lhs_bit.wire->name.c_str(), lhs_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: LHS of OR cell has invalid width.\n");
                    }
                    ret.succ_var_name = hwvar_distinguish_name(lhs);
                }
                else {
                    log("UNEXPECTED OCCASION: LHS of OR cell is not wire.\n");
                }
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            rhs = HwExpr::make_unary( Operator::NEG, HwExpr::make_binary( Operator::MUL, HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_a)), HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_b)) ) );
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_NOR_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                var_a = get_hwvar_from_sigbit(var_a_bit, "$_NOR_", "\\A");
                if (var_a_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                var_b = get_hwvar_from_sigbit(var_b_bit, "$_NOR_", "\\B");
                if (var_b_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, "$_NOR_", "\\Y");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            rhs = HwExpr::make_binary( Operator::MUL, HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_a)), HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_b)) );
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_XOR_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                if (var_a_bit.wire != NULL) {
                    if (var_a_bit.wire->width == 1) {
                        var_a = HwVar::make_single_wire(var_a_bit.wire->name.c_str());
                    }
                    else if (var_a_bit.wire->width >= 2) {
                        var_a = HwVar::make_multi_wire(var_a_bit.wire->name.c_str(), var_a_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: input A of XOR cell has invalid width.\n");
                    }
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
                else {
                    var_a = get_hwvar_from_sigbit(var_a_bit,"$_XOR_", "\\A");
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                if (var_b_bit.wire != NULL) {
                    if (var_b_bit.wire->width == 1) {
                        var_b = HwVar::make_single_wire(var_b_bit.wire->name.c_str());
                    }
                    else if (var_b_bit.wire->width >= 2) {
                        var_b = HwVar::make_multi_wire(var_b_bit.wire->name.c_str(), var_b_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: input B of XOR cell has invalid width.\n");
                    }
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
                else {
                    var_b = get_hwvar_from_sigbit(var_b_bit,"$_XOR_", "\\B");
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                if (lhs_bit.wire != NULL) {
                    if (lhs_bit.wire->width == 1) {
                        lhs = HwVar::make_single_wire(lhs_bit.wire->name.c_str());
                    }
                    else if (lhs_bit.wire->width >= 2) {
                        lhs = HwVar::make_multi_wire(lhs_bit.wire->name.c_str(), lhs_bit.offset);
                    }
                    else {
                        log("UNEXPECTED OCCASION: LHS of XOR cell has invalid width.\n");
                    }
                    ret.succ_var_name = hwvar_distinguish_name(lhs);
                }
                else {
                    log("UNEXPECTED OCCASION: LHS of XOR cell is not wire.\n");
                }
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            rhs = HwExpr::make_binary(Operator::ADD, HwExpr::make_var(var_a), HwExpr::make_var(var_b));
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_XNOR_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                var_a = get_hwvar_from_sigbit(var_a_bit, "$_XNOR_", "\\A");
                if (var_a_bit.wire != NULL) { 
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a)); 
                } 
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                var_b = get_hwvar_from_sigbit(var_b_bit, "$_XNOR_", "\\B");
                if (var_b_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, "$_XNOR_", "\\Y");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            rhs = HwExpr::make_unary( Operator::NEG, HwExpr::make_binary(Operator::ADD, HwExpr::make_var(var_a), HwExpr::make_var(var_b)) );
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_ANDNOT_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                var_a = get_hwvar_from_sigbit(var_a_bit, "$_ANDNOT_", "\\A");
                if (var_a_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                var_b = get_hwvar_from_sigbit(var_b_bit, "$_ANDNNOT_", "\\B");
                if (var_b_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, "$_ANDNOT_", "\\Y");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            rhs = HwExpr::make_binary( Operator::MUL, HwExpr::make_var(var_a), HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_b)) );
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_ORNOT_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                var_a = get_hwvar_from_sigbit(var_a_bit, "$_ORNOT_", "\\A");
                if (var_a_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                var_b = get_hwvar_from_sigbit(var_b_bit, "$_ORNOT_", "\\B");
                if (var_b_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, "$_ORNOT_", "\\Y");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            rhs = HwExpr::make_unary( Operator::NEG, HwExpr::make_binary( Operator::MUL, HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_a)), HwExpr::make_var(var_b) ));
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_MUX_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        HwVar var_s_mux;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                var_a = get_hwvar_from_sigbit(var_a_bit, "$_MUX_", "\\A");
                if (var_a_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                var_b = get_hwvar_from_sigbit(var_b_bit, "$_MUX_", "\\B");
                if (var_b_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
            }
            else if (conn.first == "\\S") {
                RTLIL::SigBit var_s_bit = conn.second.bits()[0];
                var_s_mux = get_hwvar_from_sigbit(var_s_bit, "$_MUX_", "\\S");
                if (var_s_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_s_mux));
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, "$_MUX_", "\\Y");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            /*
            std::vector<HwExpr> rhs_operands = { HwExpr::make_var(var_a), HwExpr::make_var(var_b), HwExpr::make_var(var_s_mux) };
            rhs = HwExpr::make_nary(Operator::MUX, rhs_operands);
            */
            HwExpr nSandB = HwExpr::make_unary(Operator::NEG, HwExpr::make_binary(Operator::MUL, HwExpr::make_var(var_b), HwExpr::make_var(var_s_mux))); // ~(S & B)
            HwExpr nnSandA = HwExpr::make_unary(Operator::NEG, HwExpr::make_binary( Operator::MUL, HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_s_mux)), HwExpr::make_var(var_a) )); // ~(~S & A)
            rhs = HwExpr::make_unary(Operator::NEG, HwExpr::make_binary(Operator::MUL, nSandB, nnSandA)); // ~(nSandB & nnSandA)
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_NMUX_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        HwVar var_s_mux;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                var_a = get_hwvar_from_sigbit(var_a_bit, "$_NMUX_", "\\A");
                if (var_a_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                var_b = get_hwvar_from_sigbit(var_b_bit, "$_NMUX_", "\\B");
                if (var_b_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
            }
            else if (conn.first == "\\S") {
                RTLIL::SigBit var_s_bit = conn.second.bits()[0];
                var_s_mux = get_hwvar_from_sigbit(var_s_bit, "$_NMUX_", "\\S");
                if (var_s_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_s_mux));
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, "$_NMUX_", "\\Y");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            /*
            std::vector<HwExpr> rhs_operands = { HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_a)), HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_b)), HwExpr::make_var(var_s_mux) };
            rhs = HwExpr::make_nary(Operator::MUX, rhs_operands);
            */
            HwExpr nSandnB = HwExpr::make_unary(Operator::NEG, HwExpr::make_binary( Operator::MUL, HwExpr::make_var(var_s_mux), HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_b)) )); // ~(S & ~B)
            HwExpr nnSandnA = HwExpr::make_unary(Operator::NEG, HwExpr::make_binary( Operator::MUL, HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_s_mux)), HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_a)) )); // ~(~S & ~A)
            rhs = HwExpr::make_unary(Operator::NEG, HwExpr::make_binary(Operator::MUL, nSandnB, nnSandnA)); // ~(nSandnB & nnSandnA)
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_MUX4_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        HwVar var_c;
        HwVar var_d;
        HwVar var_s_mux;
        HwVar var_t_mux;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                var_a = get_hwvar_from_sigbit(var_a_bit, "$_MUX4_", "\\A");
                if (var_a_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                var_b = get_hwvar_from_sigbit(var_b_bit, "$_MUX4_", "\\B");
                if (var_b_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
            }
            else if (conn.first == "\\C") {
                RTLIL::SigBit var_c_bit = conn.second.bits()[0];
                var_c = get_hwvar_from_sigbit(var_c_bit, "$_MUX4_", "\\C");
                if (var_c_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_c));
                }
            }
            else if (conn.first == "\\D") {
                RTLIL::SigBit var_d_bit = conn.second.bits()[0];
                var_d = get_hwvar_from_sigbit(var_d_bit, "$_MUX4_", "\\D");
                if (var_d_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_d));
                }
            }
            else if (conn.first == "\\S") {
                RTLIL::SigBit var_s_bit = conn.second.bits()[0];
                var_s_mux = get_hwvar_from_sigbit(var_s_bit, "$_MUX4_", "\\S");
                if (var_s_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_s_mux));
                }
            }
            else if (conn.first == "\\T") {
                RTLIL::SigBit var_t_bit = conn.second.bits()[0];
                var_t_mux = get_hwvar_from_sigbit(var_t_bit, "$_MUX4_", "\\T");
                if (var_t_bit.wire != NULL){
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_t_mux));
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, "$_MUX4_", "\\Y");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            std::vector<HwExpr> rhs_operands = { HwExpr::make_var(var_a), HwExpr::make_var(var_b), HwExpr::make_var(var_c), HwExpr::make_var(var_d), HwExpr::make_var(var_s_mux), HwExpr::make_var(var_t_mux) };
            rhs = HwExpr::make_nary(Operator::MUX4, rhs_operands);
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_MUX8_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        HwVar var_c;
        HwVar var_d;
        HwVar var_e;
        HwVar var_f;
        HwVar var_g;
        HwVar var_h;
        HwVar var_s_mux;
        HwVar var_t_mux;
        HwVar var_u_mux;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                var_a = get_hwvar_from_sigbit(var_a_bit, "$_MUX8_", "\\A");
                if (var_a_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                var_b = get_hwvar_from_sigbit(var_b_bit, "$_MUX8_", "\\B");
                if (var_b_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
            }
            else if (conn.first == "\\C") {
                RTLIL::SigBit var_c_bit = conn.second.bits()[0];
                var_c = get_hwvar_from_sigbit(var_c_bit, "$_MUX8_", "\\C");
                if (var_c_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_c));
                }
            }
            else if (conn.first == "\\D") {
                RTLIL::SigBit var_d_bit = conn.second.bits()[0];
                var_d = get_hwvar_from_sigbit(var_d_bit, "$_MUX8_", "\\D");
                if (var_d_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_d));
                }
            }
            else if (conn.first == "\\E") {
                RTLIL::SigBit var_e_bit = conn.second.bits()[0];
                var_e = get_hwvar_from_sigbit(var_e_bit, "$_MUX8_", "\\E");
                if (var_e_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_e));
                }
            }
            else if (conn.first == "\\F") {
                RTLIL::SigBit var_f_bit = conn.second.bits()[0];
                var_f = get_hwvar_from_sigbit(var_f_bit, "$_MUX8_", "\\F");
                if (var_f_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_f));
                }
            }
            else if (conn.first == "\\G") {
                RTLIL::SigBit var_g_bit = conn.second.bits()[0];
                var_g = get_hwvar_from_sigbit(var_g_bit, "$_MUX8_", "\\G");
                if (var_g_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_g));
                }
            }
            else if (conn.first == "\\H") {
                RTLIL::SigBit var_h_bit = conn.second.bits()[0];
                var_h = get_hwvar_from_sigbit(var_h_bit, "$_MUX8_", "\\H");
                if (var_h_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_h));
                }
            }
            else if (conn.first == "\\S") {
                RTLIL::SigBit var_s_bit = conn.second.bits()[0];
                var_s_mux = get_hwvar_from_sigbit(var_s_bit, "$_MUX8_", "\\S");
                if (var_s_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_s_mux));
                }
            }
            else if (conn.first == "\\T") {
                RTLIL::SigBit var_t_bit = conn.second.bits()[0];
                var_t_mux = get_hwvar_from_sigbit(var_t_bit, "$_MUX8_", "\\T");
                if (var_t_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_t_mux));
                }
            }
            else if (conn.first == "\\U") {
                RTLIL::SigBit var_u_bit = conn.second.bits()[0];
                var_u_mux = get_hwvar_from_sigbit(var_u_bit, "$_MUX8_", "\\U");
                if (var_u_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_u_mux));
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, "$_MUX8_", "\\Y");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            std::vector<HwExpr> rhs_operands = { HwExpr::make_var(var_a), HwExpr::make_var(var_b), HwExpr::make_var(var_c), HwExpr::make_var(var_d), HwExpr::make_var(var_e), HwExpr::make_var(var_f), HwExpr::make_var(var_g), HwExpr::make_var(var_h), HwExpr::make_var(var_s_mux), HwExpr::make_var(var_t_mux), HwExpr::make_var(var_u_mux) };
            rhs = HwExpr::make_nary(Operator::MUX8, rhs_operands);
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_MUX16_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        HwVar var_c;
        HwVar var_d;
        HwVar var_e;
        HwVar var_f;
        HwVar var_g;
        HwVar var_h;
        HwVar var_i;
        HwVar var_j;
        HwVar var_k;
        HwVar var_l;
        HwVar var_m;
        HwVar var_n;
        HwVar var_o;
        HwVar var_p;
        HwVar var_s_mux;
        HwVar var_t_mux;
        HwVar var_u_mux;
        HwVar var_v_mux;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                var_a = get_hwvar_from_sigbit(var_a_bit, "$_MUX16_", "\\A");
                if (var_a_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                var_b = get_hwvar_from_sigbit(var_b_bit, "$_MUX16_", "\\B");
                if (var_b_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
            }
            else if (conn.first == "\\C") {
                RTLIL::SigBit var_c_bit = conn.second.bits()[0];
                var_c = get_hwvar_from_sigbit(var_c_bit, "$_MUX16_", "\\C");
                if (var_c_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_c));
                }
            }
            else if (conn.first == "\\D") {
                RTLIL::SigBit var_d_bit = conn.second.bits()[0];
                var_d = get_hwvar_from_sigbit(var_d_bit, "$_MUX16_", "\\D");
                if (var_d_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_d));
                }
            }
            else if (conn.first == "\\E") {
                RTLIL::SigBit var_e_bit = conn.second.bits()[0];
                var_e = get_hwvar_from_sigbit(var_e_bit, "$_MUX16_", "\\E");
                if (var_e_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_e));
                }
            } 
            else if (conn.first == "\\F") {
                RTLIL::SigBit var_f_bit = conn.second.bits()[0];
                var_f = get_hwvar_from_sigbit(var_f_bit, "$_MUX16_", "\\F");
                if (var_f_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_f));
                }
            }
            else if (conn.first == "\\G") {
                RTLIL::SigBit var_g_bit = conn.second.bits()[0];
                var_g = get_hwvar_from_sigbit(var_g_bit, "$_MUX16_", "\\G");
                if (var_g_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_g));
                }
            }
            else if (conn.first == "\\H") {
                RTLIL::SigBit var_h_bit = conn.second.bits()[0];
                var_h = get_hwvar_from_sigbit(var_h_bit, "$_MUX16_", "\\H");
                if (var_h_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_h));
                }
            }
            else if (conn.first == "\\I") {
                RTLIL::SigBit var_i_bit = conn.second.bits()[0];
                var_i = get_hwvar_from_sigbit(var_i_bit, "$_MUX16_", "\\I");
                if (var_i_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_i));
                }
            }
            else if (conn.first == "\\J") {
                RTLIL::SigBit var_j_bit = conn.second.bits()[0];
                var_j = get_hwvar_from_sigbit(var_j_bit, "$_MUX16_", "\\J");
                if (var_j_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_j));
                }
            }
            else if (conn.first == "\\K") {
                RTLIL::SigBit var_k_bit = conn.second.bits()[0];
                var_k = get_hwvar_from_sigbit(var_k_bit, "$_MUX16_", "\\K");
                if (var_k_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_k));
                }
            }
            else if (conn.first == "\\L") {
                RTLIL::SigBit var_l_bit = conn.second.bits()[0];
                var_l = get_hwvar_from_sigbit(var_l_bit, "$_MUX16_", "\\L");
                if (var_l_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_l));
                }
            }
            else if (conn.first == "\\M") {
                RTLIL::SigBit var_m_bit = conn.second.bits()[0];
                var_m = get_hwvar_from_sigbit(var_m_bit, "$_MUX16_", "\\M");
                if (var_m_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_m));
                }
            }
            else if (conn.first == "\\N") {
                RTLIL::SigBit var_n_bit = conn.second.bits()[0];
                var_n = get_hwvar_from_sigbit(var_n_bit, "$_MUX16_", "\\N");
                if (var_n_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_n));
                }
            }
            else if (conn.first == "\\O") {
                RTLIL::SigBit var_o_bit = conn.second.bits()[0];
                var_o = get_hwvar_from_sigbit(var_o_bit, "$_MUX16_", "\\O");
                if (var_o_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_o));
                }
            }
            else if (conn.first == "\\P") {
                RTLIL::SigBit var_p_bit = conn.second.bits()[0];
                var_p = get_hwvar_from_sigbit(var_p_bit, "$_MUX16_", "\\P");
                if (var_p_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_p));
                }
            }
            else if (conn.first == "\\S") {
                RTLIL::SigBit var_s_bit = conn.second.bits()[0];
                var_s_mux = get_hwvar_from_sigbit(var_s_bit, "$_MUX16_", "\\S");
                if (var_s_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_s_mux));
                }
            }
            else if (conn.first == "\\T") {
                RTLIL::SigBit var_t_bit = conn.second.bits()[0];
                var_t_mux = get_hwvar_from_sigbit(var_t_bit, "$_MUX16_", "\\T");
                if (var_t_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_t_mux));
                }
            }
            else if (conn.first == "\\U") {
                RTLIL::SigBit var_u_bit = conn.second.bits()[0];
                var_u_mux = get_hwvar_from_sigbit(var_u_bit, "$_MUX16_", "\\U");
                if (var_u_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_u_mux));
                }
            }
            else if (conn.first == "\\V") {
                RTLIL::SigBit var_v_bit = conn.second.bits()[0];
                var_v_mux = get_hwvar_from_sigbit(var_v_bit, "$_MUX16_", "\\V");
                if (var_v_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_v_mux));
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, "$_MUX16_", "\\Y");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            std::vector<HwExpr> rhs_operands = { HwExpr::make_var(var_a), HwExpr::make_var(var_b), HwExpr::make_var(var_c), HwExpr::make_var(var_d), HwExpr::make_var(var_e), HwExpr::make_var(var_f), HwExpr::make_var(var_g), HwExpr::make_var(var_h), HwExpr::make_var(var_i), HwExpr::make_var(var_j), HwExpr::make_var(var_k), HwExpr::make_var(var_l), HwExpr::make_var(var_m), HwExpr::make_var(var_n), HwExpr::make_var(var_o), HwExpr::make_var(var_p), HwExpr::make_var(var_s_mux), HwExpr::make_var(var_t_mux), HwExpr::make_var(var_u_mux), HwExpr::make_var(var_v_mux) };
            rhs = HwExpr::make_nary(Operator::MUX16, rhs_operands);
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_AOI3_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        HwVar var_c;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                var_a = get_hwvar_from_sigbit(var_a_bit, "$_AOI3_", "\\A");
                if (var_a_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                var_b = get_hwvar_from_sigbit(var_b_bit, "$_AOI3_", "\\B");
                if (var_b_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
            }
            else if (conn.first == "\\C") {
                RTLIL::SigBit var_c_bit = conn.second.bits()[0];
                var_c = get_hwvar_from_sigbit(var_c_bit, "$_AOI3_", "\\C");
                if (var_c_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_c));
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, "$_AOI3_", "\\Y");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            HwExpr nAandB = HwExpr::make_unary( Operator::NEG, HwExpr::make_binary( Operator::MUL, HwExpr::make_var(var_a), HwExpr::make_var(var_b) ) );
            rhs = HwExpr::make_binary( Operator::MUL, nAandB, HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_c)) );
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_OAI3_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        HwVar var_c;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                var_a = get_hwvar_from_sigbit(var_a_bit, "$_OAI3_", "\\A");
                if (var_a_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                var_b = get_hwvar_from_sigbit(var_b_bit, "$_OAI3_", "\\B");
                if (var_b_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
            }
            else if (conn.first == "\\C") {
                RTLIL::SigBit var_c_bit = conn.second.bits()[0];
                var_c = get_hwvar_from_sigbit(var_c_bit, "$_OAI3_", "\\C");
                if (var_c_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_c));
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, "$_OAI3_", "\\Y");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            HwExpr nnAandnB = HwExpr::make_unary( Operator::NEG, HwExpr::make_binary( Operator::MUL, HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_a)), HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_b)) ) );
            rhs = HwExpr::make_unary( Operator::NEG, HwExpr::make_binary(Operator::MUL, nnAandnB, HwExpr::make_var(var_c)) );
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_AOI4_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        HwVar var_c;
        HwVar var_d;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                var_a = get_hwvar_from_sigbit(var_a_bit, "$_AOI4_", "\\A");
                if (var_a_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                var_b = get_hwvar_from_sigbit(var_b_bit, "$_AOI4_", "\\B");
                if (var_b_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
            }
            else if (conn.first == "\\C") {
                RTLIL::SigBit var_c_bit = conn.second.bits()[0];
                var_c = get_hwvar_from_sigbit(var_c_bit, "$_AOI4_", "\\C");
                if (var_c_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_c));
                }
            }
            else if (conn.first == "\\D") {
                RTLIL::SigBit var_d_bit = conn.second.bits()[0];
                var_d = get_hwvar_from_sigbit(var_d_bit, "$_AOI4_", "\\D");
                if (var_d_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_d));
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, "$_AOI4_", "\\Y");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            HwExpr nAandB = HwExpr::make_unary( Operator::NEG, HwExpr::make_binary(Operator::MUL, HwExpr::make_var(var_a), HwExpr::make_var(var_b)) ); // ~(A&B)
            HwExpr nCandD = HwExpr::make_unary( Operator::NEG, HwExpr::make_binary(Operator::MUL, HwExpr::make_var(var_c), HwExpr::make_var(var_d)) ); // ~(C&D)
            rhs = HwExpr::make_binary(Operator::MUL, nAandB, nCandD); // ~(A&B) & ~(C&D)
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_OAI4_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        HwVar var_b;
        HwVar var_c;
        HwVar var_d;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                var_a = get_hwvar_from_sigbit(var_a_bit, "$_OAI4_", "\\A");
                if (var_a_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
            }
            else if (conn.first == "\\B") {
                RTLIL::SigBit var_b_bit = conn.second.bits()[0];
                var_b = get_hwvar_from_sigbit(var_b_bit, "$_OAI4_", "\\B");
                if (var_b_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_b));
                }
            }
            else if (conn.first == "\\C") {
                RTLIL::SigBit var_c_bit = conn.second.bits()[0];
                var_c = get_hwvar_from_sigbit(var_c_bit, "$_OAI4_", "\\C");
                if (var_c_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_c));
                }
            }
            else if (conn.first == "\\D") {
                RTLIL::SigBit var_d_bit = conn.second.bits()[0];
                var_d = get_hwvar_from_sigbit(var_d_bit, "$_OAI4_", "\\D");
                if (var_d_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_d));
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, "$_OAI4_", "\\Y");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            HwExpr nnAandnB = HwExpr::make_unary( Operator::NEG, HwExpr::make_binary( Operator::MUL, HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_a)), HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_b)) ) ); // ~( (~A)&(~B) )
            HwExpr nnCandnD = HwExpr::make_unary( Operator::NEG, HwExpr::make_binary( Operator::MUL, HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_c)), HwExpr::make_unary(Operator::NEG, HwExpr::make_var(var_d)) ) ); // ~( (~C)&(~D) )
            rhs = HwExpr::make_unary( Operator::NEG, HwExpr::make_binary( Operator::MUL, nnAandnB, nnCandnD ) ); // ~ ( nnAandnB & nnCandnD )
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if (cell_type == ID($_TBUF_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_a;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\A") {
                RTLIL::SigBit var_a_bit = conn.second.bits()[0];
                var_a = get_hwvar_from_sigbit(var_a_bit, "$_TBUF_", "\\A");
                if (var_a_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_a));
                }
            }
            else if (conn.first == "\\Y") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, "$_TBUF_", "\\Y");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
            rhs = HwExpr::make_var(var_a);
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_subst(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    // trigger units
    // TODO: they generates empty instruction, maybe need extra process to eliminate the output signal
    // else if ( cell_type == ID($_SR_NN_) || cell_type == ID($_SR_NP_) || cell_type == ID($_SR_PN_) || cell_type == ID($_SR_PP_) ) {} 
    else if ( cell_type == ID($_FF_) || cell_type == ID($_DFF_N_) || cell_type == ID($_DFF_P_) || cell_type == ID($_DFFE_NN_) || cell_type == ID($_DFFE_NP_) || cell_type == ID($_DFFE_PN_) || cell_type == ID($_DFFE_PP_) || cell_type == ID($_DFF_NN0_) || cell_type == ID($_DFF_NN1_) || cell_type == ID($_DFF_NP0_) || cell_type == ID($_DFF_NP1_) || cell_type == ID($_DFF_PN0_) || cell_type == ID($_DFF_PN1_) || cell_type == ID($_DFF_PP0_) || cell_type == ID($_DFF_PP1_) || cell_type == ID($_DFFE_NN0N_) || cell_type == ID($_DFFE_NN0P_) || cell_type == ID($_DFFE_NN1N_) || cell_type == ID($_DFFE_NN1P_) || cell_type == ID($_DFFE_NP0N_) || cell_type == ID($_DFFE_NP0P_) || cell_type == ID($_DFFE_NP1N_) || cell_type == ID($_DFFE_NP1P_) || cell_type == ID($_DFFE_PN0N_) || cell_type == ID($_DFFE_PN0P_) || cell_type == ID($_DFFE_PN1N_) || cell_type == ID($_DFFE_PN1P_) || cell_type == ID($_DFFE_PP0N_) || cell_type == ID($_DFFE_PP0P_) || cell_type == ID($_DFFE_PP1N_) || cell_type == ID($_DFFE_PP1P_) ) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_d;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\D") {
                RTLIL::SigBit var_d_bit = conn.second.bits()[0];
                var_d = get_hwvar_from_sigbit(var_d_bit, cell_type.str(), "\\D");
                if (var_d_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_d));
                }
            }
            else if (conn.first == "\\Q") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, cell_type.str(), "\\Q");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            rhs = HwExpr::make_var(var_d);
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_glitch(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if ( cell_type == ID($_ALDFF_NN_) || cell_type == ID($_ALDFF_PN_) || cell_type == ID($_ALDFF_PP_) || cell_type == ID($_ALDFFE_NNN_) || cell_type == ID($_ALDFFE_NNP_) || cell_type == ID($_ALDFFE_NPN_) || cell_type == ID($_ALDFFE_NPP_) || cell_type == ID($_ALDFFE_PNN_) || cell_type == ID($_ALDFFE_PNP_) || cell_type == ID($_ALDFFE_PPN_) || cell_type == ID($_ALDFFE_PPP_) ) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_d;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\D") {
                RTLIL::SigBit var_d_bit = conn.second.bits()[0];
                var_d = get_hwvar_from_sigbit(var_d_bit, cell_type.str(), "\\D");
                if (var_d_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_d));
                }
            }
            else if (conn.first == "\\Q") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, cell_type.str(), "\\Q");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            rhs = HwExpr::make_var(var_d);
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_glitch(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if ( cell_type == ID($_DFFSR_NNN_) || cell_type == ID($_DFFSR_NNP_) || cell_type == ID($_DFFSR_NPN_) || cell_type == ID($_DFFSR_NPP_) || cell_type == ID($_DFFSR_PNN_) || cell_type == ID($_DFFSR_PNP_) || cell_type == ID($_DFFSR_PPN_) || cell_type == ID($_DFFSR_PPP_) || cell_type == ID($_DFFSRE_NNNN_) || cell_type == ID($_DFFSRE_NNNP_) || cell_type == ID($_DFFSRE_NNPN_) || cell_type == ID($_DFFSRE_NNPP_) || cell_type == ID($_DFFSRE_NPNN_) || cell_type == ID($_DFFSRE_NPNP_) || cell_type == ID($_DFFSRE_NPPN_) || cell_type == ID($_DFFSRE_NPPP_) || cell_type == ID($_DFFSRE_PNNN_) || cell_type == ID($_DFFSRE_PNNP_) || cell_type == ID($_DFFSRE_PNPN_) || cell_type == ID($_DFFSRE_PNPP_) || cell_type == ID($_DFFSRE_PPNN_) || cell_type == ID($_DFFSRE_PPNP_) || cell_type == ID($_DFFSRE_PPPN_) || cell_type == ID($_DFFSRE_PPPP_) ) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_d;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\D") {
                RTLIL::SigBit var_d_bit = conn.second.bits()[0];
                var_d = get_hwvar_from_sigbit(var_d_bit, cell_type.str(), "\\D");
                if (var_d_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_d));
                }
            }
            else if (conn.first == "\\Q") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, cell_type.str(), "\\Q");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            rhs = HwExpr::make_var(var_d);
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_glitch(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if ( cell_type == ID($_SDFF_NN0_) || cell_type == ID($_SDFF_NN1_) || cell_type == ID($_SDFF_NP0_) || cell_type == ID($_SDFF_NP1_) || cell_type == ID($_SDFF_PN0_) || cell_type == ID($_SDFF_PN1_) || cell_type == ID($_SDFF_PP0_) || cell_type == ID($_SDFF_PP0_) || cell_type == ID($_SDFF_PP1_) || cell_type == ID($_SDFFE_NN0N_) || cell_type == ID($_SDFFE_NN0P_) || cell_type == ID($_SDFFE_NN1N_) || cell_type == ID($_SDFFE_NN1P_) || cell_type == ID($_SDFFE_NP0N_) || cell_type == ID($_SDFFE_NP0P_) || cell_type == ID($_SDFFE_NP1N_) || cell_type == ID($_SDFFE_NP1P_) || cell_type == ID($_SDFFE_PN0N_) || cell_type == ID($_SDFFE_PN0P_) || cell_type == ID($_SDFFE_PN1N_) || cell_type == ID($_SDFFE_PN1P_) || cell_type == ID($_SDFFE_PP0N_) || cell_type == ID($_SDFFE_PP0P_) || cell_type == ID($_SDFFE_PP1N_) || cell_type == ID($_SDFFE_PP1P_) || cell_type == ID($_SDFFCE_NN0N_) || cell_type == ID($_SDFFCE_NN0P_) || cell_type == ID($_SDFFCE_NN1N_) || cell_type == ID($_SDFFCE_NN1P_) || cell_type == ID($_SDFFCE_NP0N_) || cell_type == ID($_SDFFCE_NP0P_) || cell_type == ID($_SDFFCE_NP1N_) || cell_type == ID($_SDFFCE_PN0N_) || cell_type == ID($_SDFFCE_PN0P_) || cell_type == ID($_SDFFCE_PN1N_) || cell_type == ID($_SDFFCE_PN1P_) || cell_type == ID($_SDFFCE_PP0N_) || cell_type == ID($_SDFFCE_PP0P_) || cell_type == ID($_SDFFCE_PP1N_) || cell_type == ID($_SDFFCE_PP1P_) ) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_d;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\D") {
                RTLIL::SigBit var_d_bit = conn.second.bits()[0];
                var_d = get_hwvar_from_sigbit(var_d_bit, cell_type.str(), "\\D");
                if (var_d_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_d));
                }
            }
            else if (conn.first == "\\Q") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, cell_type.str(), "\\Q");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            rhs = HwExpr::make_var(var_d);
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_glitch(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else if ( cell_type == ID($_DLATCH_N_) || cell_type == ID($_DLATCH_P_) || cell_type == ID($_DLATCH_NN0_) || cell_type == ID($_DLATCH_NN1_) || cell_type == ID($_DLATCH_NP0) || cell_type == ID($_DLATCH_NP1_) || cell_type == ID($_DLATCH_PN0_) || cell_type == ID($_DLATCH_PN1_) || cell_type == ID($_DLATCH_PP0_) || cell_type == ID($_DLATCH_PP1_) || cell_type == ID($_DLATCHSR_NNN_) || cell_type == ID($_DLATCHSR_NNP_) || cell_type == ID($_DLATCHSR_NPN_) || cell_type == ID($_DLATCHSR_NPP_) || cell_type == ID($_DLATCHSR_PNN_) || cell_type == ID($_DLATCHSR_PNP_) || cell_type == ID($_DLATCHSR_PPN_) || cell_type == ID($_DLATCHSR_PPP_) ) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        HwVar var_d;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first == "\\D") {
                RTLIL::SigBit var_d_bit = conn.second.bits()[0];
                var_d = get_hwvar_from_sigbit(var_d_bit, cell_type.str(), "\\D");
                if (var_d_bit.wire != NULL) {
                    ret.pred_var_names.insert(hwvar_distinguish_name(var_d));
                }
            }
            else if (conn.first == "\\Q") {
                RTLIL::SigBit lhs_bit = conn.second.bits()[0];
                lhs = get_hwvar_from_sigbit(lhs_bit, cell_type.str(), "\\Q");
                ret.succ_var_name = hwvar_distinguish_name(lhs);
            }
            rhs = HwExpr::make_var(var_d);
        }
        std::string instr_name = cell->name.c_str();
        hi = HwInstruction::make_glitch(instr_name, lhs, rhs);
        ret.instruction = hi;
    }
    else {
        log("UNEXPECTED OCCASION: UNKNOWN CELL TYPE %s.\n", cell_type.c_str());
    }


    return ret;
}


struct WireConnInfo{
    std::string wire_name; // plain name
    bool with_offset;
    int offset;
    std::string driver_inst;
    std::vector<std::string> user_insts;
};

/*
void dump_wireconninfo(std::ostream& f, WireConnInfo wci) {
    f << stringf("WireConnInfo of wire %s", wci.wire_name);
    if (wci.with_offset) {
        f << stringf("[%d]", wci.offset);
    }
    f << stringf(", driven by %s, used by ", wci.driver_inst);
    for (auto it = wci.user_insts.begin(); it != wci.user_insts.end(); it++) {
        if (it != wci.user_insts.begin()) {
            f << stringf(", ");
        }
        f << stringf("%s", *it);
    }
    f << stringf("\n");
}
*/



std::vector<WireConnInfo> wire_to_wire_conn_info_base(RTLIL::Wire* wire) {
    std::vector<WireConnInfo> ret;
    // for every bit in the wire, generate a WireConnInfo
    if (wire->width == 1) {
        WireConnInfo wci;
        wci.wire_name = wire->name.c_str();
        wci.with_offset = false;
        wci.offset = 0;
        // driver_inst and user_insts to be filled later
        ret.push_back(wci);
    }
    else if (wire->width >= 2) {
        for (int i=0; i < wire->width; i++) {
            WireConnInfo wci;
            wci.wire_name = wire->name.c_str();
            wci.with_offset = true;
            if (wire->upto) {
                wci.offset = wire->start_offset - wire->width;
            }
            else {
                wci.offset = wire->start_offset + i;
            }
            ret.push_back(wci);
        }
    }
    else {
        log("UNEXPECTED OCCASION: wire has invalid width.\n");
    }
    return ret;
}


// used to implement topological sorting
struct InstrNode {
    std::string instr_name;
    std::vector<std::string> descend_instrs_name; // instrs that use the output of this instr as input
    int remain_driver;
    // std::vector<std::string> pred_var_names; // multiple input signals, each corr to a driver instr
};


void dump_instrnode(std::ostream& f, InstrNode inode) {
    f << stringf("InstrNode %s: remain_driver %d, descend_instrs ", inode.instr_name, inode.remain_driver);
    for (auto it = inode.descend_instrs_name.begin(); it != inode.descend_instrs_name.end(); it++) {
        if (it != inode.descend_instrs_name.begin()) {
            f << stringf(", ");
        }
        f << stringf("%s", *it);
    }
    f << stringf("\n");
}
/*
void dump_instr_cycles(dict<std::string, InstrNode>& instr_node) {
    // int all_instr_size = instr_node.size();
    dict<std::string, int> eliminate_instrs;    
    // if in sequential program or already classified loop, assign 0, elsewhire assign 1 
    for (std::pair<std::string, InstrNode> instr_node_ele : instr_node) {
        if (instr_node_ele.second.remain_driver == 0) {
            eliminate_instrs[instr_node_ele.first] = 0;
        }
        else {
            eliminate_instrs[instr_node_ele.first] = 1;
        }
    }
    while (true) {
        // if all instrs are eliminated, end loop
        vector<std::string> cycle_instrs = {};
        bool all_eliminated = true;
        for (std::pair<std::string, int> eliminate_instr_ele : eliminate_instrs) {
            if (eliminate_instr_ele.second == 1) {
                cycle_instrs.push_back(eliminate_instr_ele.first);
                all_eliminated = false;
                break;
            }
        }
        if (all_eliminated) {
            break;
        }

    }
    return;
}
*/


HwModuleDef module_to_hwmoduledef(RTLIL::Module *module) {
    HwModuleDef ret;
    ret.module_name = module->name.str();


    // std::vector<HwInstruction> instrs; // used as library of instructions
    dict<std::string, HwInstruction> instrs_dict; // used as library of instructions

    std::vector<WireConnInfo> wire_conn_infos;
    dict<std::string, WireConnInfo> wire_conn_info_dict;
    // for every wire, generate a wire connection info
    for (std::pair<RTLIL::IdString, RTLIL::Wire *> wire_pair : module->wires_) {
        RTLIL::Wire* wire = wire_pair.second;
        // add info(s) into the vector
        std::vector<WireConnInfo> tmp_wire_conn_infos = wire_to_wire_conn_info_base(wire);
        wire_conn_infos.insert(wire_conn_infos.end(), tmp_wire_conn_infos.begin(), tmp_wire_conn_infos.end()); // TODO: to be deprecated
        for (auto wci : tmp_wire_conn_infos) {
            std::string wci_distinguish_name;
            if (wci.with_offset) {
                wci_distinguish_name = stringf("%s_%d", wci.wire_name.c_str(), wci.offset);
            }
            else {
                wci_distinguish_name = stringf("%s_0", wci.wire_name.c_str());
            }
            wire_conn_info_dict[wci_distinguish_name] = wci;
        }
    }

    // log("Generated wire connection infos:\n");
    // for(auto wci : wire_conn_infos) {
    //     std::ostringstream oss;
    //     dump_wireconninfo(oss, wci);
    //     log("%s\n", oss.str());
    // } // driver by and used by info currently not filled

    // generate instructions from cells and connections, to be sorted later
    /*
    for (std::pair<RTLIL::IdString, RTLIL::Cell *> cell_pair : module->cells_) {
        RTLIL::Cell* cell = cell_pair.second;
        HwInstruction hi = simcell_to_instruction(cell);
        instrs.push_back(hi);
    }
    for (RTLIL::SigSig conn : module->connections()) {
        std::vector<HwInstruction> conn_instrs = connect_to_instruction(conn);
        instrs.insert(instrs.end(), conn_instrs.begin(), conn_instrs.end());
    }
    */
    // generate instructions from cells and connections, store into dict

    log("Generated instructions in module %s\n", module->name.c_str());


    for (std::pair<RTLIL::IdString, RTLIL::Cell *> cell_pair : module->cells_) {
        RTLIL::Cell* cell = cell_pair.second;

        // jump $scopeinfo cells
        if (cell->type.str() == "$scopeinfo") {
            continue;
        }

        auto instr_info = simcell_to_instruction(cell);
        if (instrs_dict.count(instr_info.name) != 0) {
            log("UNEXPECTED OCCASION: Instruction %s already exists in module %s, overwriting.\n", instr_info.name, module->name.c_str());
        }
        instrs_dict[instr_info.name] = instr_info.instruction;

        // use pred and succ wire info, add to wire_conn_infos
        // as to preds wires of the instr, the instr is their user insts
        for (auto pred_var_name : instr_info.pred_var_names) {
            if (wire_conn_info_dict.count(pred_var_name) != 0) {
                wire_conn_info_dict[pred_var_name].user_insts.push_back( instr_info.name );
            }
            else {
                log("UNEXPECTED OCCASION: pred var name %s not found in wire conn info dict.\n", pred_var_name.c_str());
            }
        }
        // as to succ wire of the instr, the instr is its driver inst
        if (wire_conn_info_dict.count(instr_info.succ_var_name) != 0) {
            wire_conn_info_dict[instr_info.succ_var_name].driver_inst = instr_info.name;
        }
        else {
            log("UNEXPECTED OCCASION: succ var name %s not found in wire conn info dict.\n", instr_info.succ_var_name.c_str());
        }

        // std::ostringstream oss;
        // dump_hwinstrinfo(oss, instr_info);
        // log("%s\n", oss.str());

    }
    // generate instrucstions from connections
    for (RTLIL::SigSig conn : module->connections()) {
        auto conn_instrs_infos = connect_to_instruction(conn);
        for (auto instr_info : conn_instrs_infos) { 
            if (instrs_dict.count(instr_info.name) != 0) {
                log("UNEXPECTED OCCASION: Instruction %s already exists in module %s, overwriting.\n", instr_info.name, module->name.c_str());
            }
            instrs_dict[instr_info.name] = instr_info.instruction;

            // use pred and succ wire info, add to wire_conn_infos
            // as to preds wires of the instr, the instr is their user insts
            for (auto pred_var_name : instr_info.pred_var_names) {
                if (wire_conn_info_dict.count(pred_var_name) != 0) {
                    wire_conn_info_dict[pred_var_name].user_insts.push_back( instr_info.name );
                }   
                else {
                    log("UNEXPECTED OCCASION: pred var name %s not found in wire conn info dict.\n", pred_var_name.c_str());
                }
            }
            // as to succ wire of the instr, the instr is its driver inst
            if (wire_conn_info_dict.count(instr_info.succ_var_name) != 0) {
                wire_conn_info_dict[instr_info.succ_var_name].driver_inst = instr_info.name;
            }
            else {
                log("UNEXPECTED OCCASION: succ var name %s not found in wire conn info dict.\n", instr_info.succ_var_name.c_str());
            }
            // std::ostringstream oss;
            // dump_hwinstrinfo(oss, instr_info);
            // log("%s\n", oss.str());

        }
    }


    // log("Filled wire connection infos:\n");
    // for(auto wcid : wire_conn_info_dict) {
    //     std::ostringstream oss;
    //     dump_wireconninfo(oss, wcid.second);
    //     log("%s\n", oss.str());
    // }

    // log("Generated instructions in module %s:\n", module->name.c_str());
    // for (auto inst_ele : instrs_dict) {
    //     std::ostringstream oss;
    //     dump_hwinstruction(oss, inst_ele.second);
    //     log("%s\n", oss.str());
    // }

    /*
    std::vector<InstrNode> instr_nodes;
    dict<std::string, InstrNode*> instr_node_dict;
    // build instr nodes for topological sorting 

    for (auto inst_dict_ele : instrs_dict) {
        InstrNode inode;
        inode.instr_name = inst_dict_ele.first;
        inode.remain_driver = 0; // to be filled
        instr_nodes.push_back(inode);
        instr_node_dict[inst_dict_ele.first] = &instr_nodes.back();
    }

    for (auto wcid: wire_conn_info_dict){
        std::string driver_inst = wcid.second.driver_inst;
        for (auto user_inst : wcid.second.user_insts) {
            // add edge from driver_inst to user_inst
            if (instr_node_dict.count(driver_inst) != 0 && instr_node_dict.count(user_inst) != 0) {
                instr_node_dict[driver_inst]->descend_instrs_name.push_back(user_inst);
                instr_node_dict[user_inst]->remain_driver += 1;
            }
        }
    }
    
    for (auto inode_ele : instr_node_dict) {
        std::ostringstream oss;
        dump_instrnode(oss, *(inode_ele.second));
        log("%s", oss.str());
    }
    */

    dict<std::string, InstrNode> instr_node;

    for (auto inst_dict_ele : instrs_dict) {
        InstrNode inode;
        inode.instr_name = inst_dict_ele.first;
        inode.remain_driver = 0; // to be filled
        instr_node[inst_dict_ele.first] = inode;
    }

    for (auto wcid: wire_conn_info_dict) {
        std::string driver_inst = wcid.second.driver_inst;
        for (auto user_inst : wcid.second.user_insts) {
            // add edge from driver_inst to user_inst
            if (instr_node.count(driver_inst) != 0 && instr_node.count(user_inst) != 0) {
                instr_node[driver_inst].descend_instrs_name.push_back(user_inst);
                instr_node[user_inst].remain_driver += 1;
            }
        }
    }

    // log("Instruction nodes for topological sorting:\n");
    // for (auto inode_ele : instr_node) {
    //     std::ostringstream oss;
    //     dump_instrnode(oss, inode_ele.second);
    //     log("%s", oss.str());
    // }

    // topo sort
    std::queue<std::string> ready_queue;

    for (auto inode_ele : instr_node) {
        if (inode_ele.second.remain_driver == 0) {
            ready_queue.push(inode_ele.first);
        }
    }

    while (!ready_queue.empty()) {
        std::string curr_instr_name = ready_queue.front();
        ready_queue.pop();
        // add to instructions of module def
        ret.instructions.push_back( instrs_dict[curr_instr_name] );
        // update descend instrs
        for (std::string desc_instr_name : instr_node[curr_instr_name].descend_instrs_name) {
            instr_node[desc_instr_name].remain_driver -= 1;
            if (instr_node[desc_instr_name].remain_driver == 0) {
                ready_queue.push(desc_instr_name);
            }
        }
    }

    int instr_dict_len = instrs_dict.size();
    int result_instr_len = ret.instructions.size();

    if (instr_dict_len != result_instr_len) {
        log("ERROR POSSIBLE: LIKELY LOOP EXIST IN INSTRUCTION DESCENDENT RELATIONS.\n");
        // dump remaining instructions
        for (std::pair<std::string, InstrNode> instr_dict_ele : instr_node ) {
            if (instr_dict_ele.second.remain_driver != 0) {
                std::ostringstream oss;
                dump_instrnode(oss, instr_dict_ele.second);
                log("%s\n", oss.str());
            }
        }

        // TODO: get remaining instructions, find cycles in the node map 
        
        // dump_instr_cycles();

    }

    return ret;
}




struct MvBackend : public Backend {
    MvBackend() : Backend("mv", "generate MV file from RTLIL design for verification") { }
    void help() override {
        // TODO: add help message
        log("\n");
        log("    write_mv [filename]\n");
        log("\n");
        log("Generate MV file from RTLIL design for verification.\n");
        log("\n");

    }
    void execute(std::ostream *&f, std::string filename, std::vector<std::string> args, RTLIL::Design *design) override {
        log_header(design, "Executing MV backend.\n");

        // TODO: Process arguments


        size_t argidx;

        for (argidx = 1; argidx < args.size(); argidx++) {
            std::string arg = args[argidx];
            // EXTEND: Arguments 
            break;
        }
        

        extra_args(f, filename, args, argidx); // write file content to ostream f 

        log("Output filename: %s\n", filename);
        
        *f << stringf("(* Generated by Yosys_expr based on %s *)\n", yosys_maybe_version());

        
        // TODO: write framework from design to HwModuleDef
        // there is only one module in simcells design

        RTLIL::Module* top_module = design->top_module();
        HwModuleDef hmd = module_to_hwmoduledef(top_module);

        dump_mv_hwmoduledef(*f, hmd);


    }
} MvBackend;

PRIVATE_NAMESPACE_END

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

#include "kernel/yosys.h"
#include "kernel/rtlil.h"
#include "mv_backend.h"
// #include "proc_expr.h"
// #include "proc_expr.cc"

USING_YOSYS_NAMESPACE

using namespace MV_BACKEND; 

const pool<string> MV_BACKEND::mv_keywords() {
    static const pool<string> res = {
        // TODO: fill in mv keywords 
    };
    return res;
}

PRIVATE_NAMESPACE_BEGIN


// initialize variables
bool noglitch;

// print structs defined in mv_backend.h

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
    }
    else if (hi.kind == HwInstruction::InstrKind::IK_glitch) {
        dump_hwvar(f, hi.lhs);
        f << stringf(" =! [");
        dump_hwexpr(f, hi.rhs);
        f << stringf("]; ");
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

// TODO: print methods for structs defined in mv_backend.h


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

/*
void print_sigwidth(RTLIL::SigSpec sig) {
    log("SigSpec %s, width %d\n", sig.wire->name.c_str(), sig.size());
}
*/

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
        for (const auto& chunk : sig.chunks() /*reversed(sig.chunks())*/) {
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

/*
void print_SigSig(std::pair<const RTLIL::SigSpec, RTLIL::SigSpec> sig_pair) {
    log("      first: %s", sig_pair.first.as_string());
    log("      second: %s", sig_pair.second.as_string());
    log("\n");
}
*/


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
    /*
    log("  Connections:\n");
    for (std::pair<RTLIL::SigSpec, RTLIL::SigSpec> conn_pair : connections_of_module) {
        // 
        print_SigSig(conn_pair);
    }
    */
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


std::vector<HwInstrInfo> connect_to_instruction(RTLIL::SigSig conn) {
    HwInstrInfo ret;
    // generate instruction(s) from the connection
    // extract bits of left hand side
    // RTLIL::SigSpec lhs_bits = conn.first.unpack();
    // RTLIL::SigSpec rhs_bits = conn.second.unpack();
    // log_assert(lhs_bits.size() == rhs_bits.size());
    for (int i = 0; i < lhs_bits.size(); i++) { 
        HwInstruction hi;
        // left hand side
        RTLIL::SigBit lhs_bit = conn.first.unpack()[i];
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
        RTLIL::SigBit rhs_bit = conn.second.unpack()[i];
        log_assert(rhs_bit.wire != NULL); // should be wire
        HwVar hv_rhs;
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
        hi = HwInstruction::make_subst(hv_lhs, HwExpr::make_var(hv_rhs));
        // generate name for the instruction, use lhs wire name and index
        std::string instr_name;
        instr_name = hwvar_distinguish_name(hv_lhs);
        // pred var names
        std::set<std::string> pred_names;
        pred_names.insert( hwvar_distinguish_name(hv_rhs) );
        // succ var name
        std::string succ_name = hwvar_distinguish_name(hv_lhs);

        ret.push_back( {instr_name, hi, pred_names, succ_name} );
    }
    // name of assignment instruction is correlated to lhs signal name and index (if multi bit)
    return ret;
}


/*
HwInstruction simcell_to_instruction(RTLIL::Cell* cell) { // Add a vector of input signals, and output signal,
    // generate an instruction from the expr
    HwInstruction ret;
    RTLIL::IdString cell_type = cell->type;
    // find input ports and output port
    for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
    }
    return ret;
}
*/

HwInstrInfo simcell_to_instruction(RTLIL::Cell* cell) {
    HwInstrInfo ret;
    ret.first = cell->name.str();
    // generate an instruction from the expr
    RTLIL::IdString cell_type = cell->type;
    // distinguish between different cells, each give different result
    if (cell_type == ID($_BUF_)) {
        HwInstruction hi;
        HwVar lhs;
        HwExpr rhs;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
            if (conn.first.c_str() == "\A") {
                // 
                rhs = HwExpr::make_var( HwVar:: );
                ret.pred_var_names.insert();
            }
            else if (conn.first.c_str() == "\Y") {

            }
            else {
                log("UNEXPECTED OCCASION: UNKNOWN PORT %s IN CELL TYPE %s.\n", conn.first.c_str(), cell_type.c_str());
            }
        }
        hi = HwInstruction::make_subst(lhs, rhs);
    }
    // find input ports and output port
    for (std::pair<RTLIL::IdString, RTLIL::SigSpec> conn : cell->connections_) {
        RTLIL::IdString port_name = conn.first;
        RTLIL::SigSpec sig = conn.second;
        if (cell->input(port_name)) {

        }
        else {

        }
    }
}


struct WireConnInfo{
    std::string wire_name;
    bool with_bool;
    int offset;
    std::string driver_inst;
    std::vector<std::string> user_insts;
};


std::vector<WireConnInfo> wire_to_wire_conn_info_base(RTLIL::Wire* wire) {
    std::vector<WireConnInfo> ret;
    // for every bit in the wire, generate a WireConnInfo
    for (int i=0; i < wire->width; i++) {
        WireConnInfo wci;
        
    }
}


HwModuleDef module_to_hwmoduledef(RTLIL::Module *module) {
    HwModuleDef ret;
    ret.module_name = module->name.str();


    // std::vector<HwInstruction> instrs; // used as library of instructions
    dict<std::string, HwInstruction> instrs_dict; // used as library of instructions

    std::vector<WireConnInfo> wire_conn_infos;
    // for every wire, generate a wire connection info
    for (std::pair<RTLIL::IdString, RTLIL::Wire *> wire_pair : module->wires_) {
        RTLIL::Wire* wire = wire_pair.second;
        // add info(s) into the vector
        std::vector<WireConnInfo> tmp_wire_conn_infos = wire_to_wire_conn_info_base(wire);
        wire_conn_infos.insert(wire_conn_infos.end(), tmp_wire_conn_infos.begin(), tmp_wire_conn_infos.end());
    }


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
    for (std::pair<RTLIL::IdString, RTLIL::Cell *> cell_pair : module->cells_) {
        RTLIL::Cell* cell = cell_pair.second;
        auto instr_info = simcell_to_instruction(cell);
        if (instrs_dict.count(instr_info.name) != 0) {
            log("UNEXPECTED OCCASION: Instruction %s already exists in module %s, overwriting.\n", instr_info.name, module->name.c_str());
        }
        instrs_dict[instr_info.name] = instr_info.instruction;


    }
    for (RTLIL::SigSig conn : module->connections()) {
        auto conn_instrs_infos = connect_to_instruction(conn);
        for (auto instr_info : conn_instrs_infos) { 
            if (instrs_dict.count(instr_info.name) != 0) {
                log("UNEXPECTED OCCASION: Instruction %s already exists in module %s, overwriting.\n", instr_info.name, module->name.c_str());
            }
            instrs_dict[instr_info.name] = instr_info.instruction;
        }
    }

    




}




struct MvBackend : public Backend {
    MvBackend() : Backend("mv", "generate MV file from RTLIL design for verification") { }
    void help() override {
        // TODO: add help message
        log("\n");
        log("    write_mv [options] [filename]\n");
        log("\n");
        log("Generate MV file from RTLIL design for verification.\n");
        log("\n");
        log("    -noglitch\n");
        log("        Disable glitch modeling when generating MV file.\n");
        log("    -clib <filename>\n");
        log("        Use the specified Verilog file as the cells library (default: built-in simcells.v).");
        log("\n");

    }
    void execute(std::ostream *&f, std::string filename, std::vector<std::string> args, RTLIL::Design *design) override {
        log_header(design, "Executing MV backend.\n");

        // TODO: Process arguments

        noglitch = false;

        size_t argidx;
        std::string celllib_file;

        for (argidx = 1; argidx < args.size(); argidx++) {
            std::string arg = args[argidx];
            if (arg == "-noglitch") {
                noglitch = true;
                continue;
            }
            if (arg == "-clib" && argidx+1 < args.size()) {
                celllib_file = args[++argidx];
                continue;
            }
            // EXTEND: Arguments 
            cmd_error(args, argidx, "Unknown option or option in arguments.");
        }
        

        extra_args(f, filename, args, argidx); // write file content to ostream f 
        
        *f << stringf("/* Generated by Yosys_expr based on %s */\n", yosys_maybe_version());

        if (celllib_file.empty()) {
            get_simcells_expr();
        }
        else {
            get_celllib_expr(celllib_file);
        }
        
        // TODO: write framework from design to HwModuleDef
        // there is only one module in simcells design

        RTLIL::Module* top_moudle = design->top_module();
        module_to_hwmoduledef(top_module);



    }
} MvBackend;

PRIVATE_NAMESPACE_END

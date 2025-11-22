/*
  1. Define expr 
  2. Read simcells.v into rtlil internal representation
  3. Transform into expr formation
  4. Calculate leak expr
*/


#include <cstring>
#include <vector>
#include <queue>

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

// 

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
            log("PRINT A MULTI-BIT SIGNAL, CURRENTLY NOT SOLVED, KEEP MONITORING.");
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


struct PortInfo{
    RTLIL::IdString cell_type;
    RTLIL::IdString cell_id;
    RTLIL::IdString port_name;

    bool operator==(const PortInfo &other) const {
        return (cell_type == other.cell_type) && (cell_id == other.cell_id) && (port_name == other.port_name);
    }

    Hasher hash_into(Hasher h) const {
        h = cell_type.hash_into(h);
        h = cell_id.hash_into(h);
        h = port_name.hash_into(h);
        return h;
    }
};

struct PortCorrespond{
    RTLIL::IdString port_name;
    RTLIL::SigBit sig;
};

/*
Expr elim_const_in_expr(Expr ex) {
    // if there is a const in expr, transform biop into uniop
    Expr ret;

}
*/

// TODO: finish the function
/*
bool sigbit_to_bool(RTLIL::SigBit sig) {
    bool ret;
    if (data.flags) {
        

    }
    return ret;
}
    */


Instruction cell_to_expr(RTLIL::IdString cell_type, std::vector<PortCorrespond> inputs, std::vector<PortCorrespond> output ) { // Add a vector of input signals, and output signal,
    // generate an instruction from the expr
    Instruction ret;
    std::string cell_type_name = cell_type.c_str();
    std::string lhs_name = output.sig->wire.name.c_str();
    // fetch input signals


    switch (cell_type_name)
        case "$not" : 
            HwVar tmpLhs = wire(lhs_name);
            PortCorrespond input_corr = *inputs.begin();
            // if input port connects to a bit wire
            if (input_corr.sig.is_wire()) {
                HwExpr rhs_var = var( wire(sig->wire.name) );
                HwExpr unary(Operator::ADD, &rhs_var);
            }
            // if input port connects to a bool const
            else {
                // get bool value from sig. presumably it is a const
                // TODO: finish the case 
            }
            
            break;
        case "$pos" :
            break;
        case "$neg" :
            break;
        default:
            log("UNEXPECTED OCCASION: CELL %s IN MODULE.\n", cell_type_name);
    
    return ret;
}



    void print_wire_conn(dict<RTLIL::Wire*, RTLIL::SigBit> wire_conn) {
        log("Print wire connection:\n");
        for (std::pair<RTLIL::Wire*, RTLIL::SigBit> wc_pair : wire_conn) {
            log("  wire %s connected to sig ", wc_pair.first->name.c_str());
            print_sigspec(RTLIL::SigSpec(wc_pair.second));
            log("\n");
        }
    }

    void print_outport_conn(dict<RTLIL::Wire*, PortInfo> outport_conn) {
        log("Print outport connection:\n");
        for (std::pair<RTLIL::Wire*, PortInfo> opc_pair : outport_conn) {
            log("  wire %s connected to port %s of cell %s of type %s\n", 
                opc_pair.first->name.c_str(),
                opc_pair.second.port_name.c_str(),
                opc_pair.second.cell_id.c_str(),
                opc_pair.second.cell_type.c_str()
            );
        }
    }

    void print_inport_conn(dict<PortInfo, RTLIL::SigBit> inport_conn) {
        log("Print inport connection:\n");
        for (std::pair<PortInfo, RTLIL::SigBit> ipc_pair : inport_conn) {
            log("  port %s of cell %s of type %s connected to sig ", 
                ipc_pair.first.port_name.c_str(),
                ipc_pair.first.cell_id.c_str(),
                ipc_pair.first.cell_type.c_str()
            );
            print_sigspec(RTLIL::SigSpec(ipc_pair.second));
            log("\n");
        }
    }

    void print_cellinport_conn(dict<RTLIL::IdString, std::vector< std::pair<RTLIL::IdString, RTLIL::SigBit> > > cellinport_conn) {
        log("Print cell inport connection:\n");
        for (std::pair<RTLIL::IdString, std::vector< std::pair<RTLIL::IdString, RTLIL::SigBit> > > cic_pair : cellinport_conn) {
            log("  cell %s inport connections:\n", cic_pair.first.c_str());
            for (std::pair<RTLIL::IdString, RTLIL::SigBit> port_sig : cic_pair.second) {
                log("    port %s connected to sig ", port_sig.first.c_str());
                print_sigspec(RTLIL::SigSpec(port_sig.second));
                log("\n");
            }
        }
    }

ModuleExpr module_to_expr(const RTLIL::Module *module) {
    // transform basic module definition into expression form
    // this transformation is used for simcell lib modules
    // so there can be multiple bit signals

    std::vector<Instruction> insts; // store instructions in the module, generate in reversed order

    // first collect wires, find input and output wires
    std::vector<RTLIL::IdString> input_wires;
    RTLIL::IdString output_wire;
    std::vector<RTLIL::IdString> inner_wires;

    for (std::pair<RTLIL::IdString, RTLIL::Wire*> w : module->wires_) {
        if (w.second->port_input == true) {
            input_wires.push_back(w.first);
        } else if (w.second->port_output == true) {
            output_wire = w.first;
        }
        else {
            inner_wires.push_back(w.first); // neither input wires nor output wire
        }
    }
    // Traverse connection to link input and output wires
    // wire/const -- wire, wire_conn[wire] = wire/const
    dict<RTLIL::Wire*, RTLIL::SigBit> wire_conn;
    // port -- wire , outport_conn[wire] = port
    dict<RTLIL::Wire*, PortInfo> outport_conn;
    // wire/const -- port inport_conn[port] = wire/const
    // dict<PortInfo, RTLIL::SigBit> inport_conn;

    // ANOTHER INPORT CONNECTION SEARCH DICT, THE INDEX IS CELL ID, maybe include cell type name also
    dict<RTLIL::IdString, std::vector< std::pair<RTLIL::IdString, RTLIL::SigBit> > > cellinport_conn;
    // Traverse connections, add to wire_conn
    for (std::pair<const RTLIL::SigSpec, RTLIL::SigSpec> c : module->connections_) {
        // connect left right, right -> left
        // so generate wire_conn[left] = right
        RTLIL::SigSpec lhs = c.first;
        RTLIL::SigSpec rhs = c.second;
        if (lhs.is_wire()) {
            if (rhs.is_wire() || rhs.is_fully_const()) {
                RTLIL::Wire *lhs_wire = lhs.as_wire();
                // print_bit_info(module->name, rhs);
                RTLIL::SigBit rhs_sigbit = rhs.as_bit();
                wire_conn[lhs_wire] = rhs_sigbit;
            } else {
                log("UNEXPECTED OCCATION: RIGHT HAND SIDE OF CONNECTION IS NOT WIRE/CONST.\n");
            }
        } else {
            log("UNEXPECTED OCCATION: LEFT HAND SIDE OF CONNECTION IS NOT WIRE.\n");
        }
    }
    /*
    // Traverse cells, add to inport_conn and outport_conn
    for (std::pair<RTLIL::IdString, RTLIL::Cell*> c : module->cells_) {
        // input port
        dict<RTLIL::IdString, RTLIL::SigSpec> conns = c.second->connections_;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> p : conns) {
            RTLIL::IdString port_name = p.first;
            RTLIL::SigSpec sig = p.second;
            if (c.second->input(port_name)) {
                // add{ into inport_conn
                if (sig.is_fully_const() || sig.is_wire()) {
                    // print_cell_bit_info(module->name, c.second->name, sig);
                    RTLIL::SigBit tmpSigbit = sig.as_bit();
                    inport_conn[{c.second->type, c.second->name, port_name}] = tmpSigbit;
                } else {
                    log("UNEXPECTED OCCATION: INPUT PORT CONNECTING SIGNAL OTHER THAN WIRE/CONST.\n");
                }
            } else if (c.second->output(port_name)) {
                // add into outport_conn
                if (sig.is_wire()) {
                    RTLIL::Wire *tmpWire = sig.as_wire();
                    outport_conn[tmpWire] = {c.second->type, c.second->name, port_name};
                } else {
                    log("UNEXPECTED OCCATION: OUTPUT PORT CONNECTING SIGNAL OTHER THAN WIRE.\n");
                }
            } else {
                log("UNEXPECTED OCCATION: PORT NEITHER INPUT NOR OUTPUT.\n");
            }
        }
    }
    */
    // generate cell inport connection dict, same usage as inport conn 
    // (along with outport connection) 
    for (std::pair<RTLIL::IdString, RTLIL::Cell*> c : module->cells_) {
        dict<RTLIL::IdString, RTLIL::SigSpec> conns = c.second->connections_;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> p : conns) {
            RTLIL::IdString port_name = p.first;
            RTLIL::SigSpec sig = p.second;
            if (c.second->input(port_name)) {
                if (sig.is_fully_const() || sig.is_wire()) {
                    // print_cell_bit_info(module->name, c.second->name, sig);
                    RTLIL::SigBit tmpSigbit = sig.as_bit();
                    cellinport_conn[c.second->name].push_back( {port_name, tmpSigbit} );
                } else {
                    log("UNEXPECTED OCCATION: INPUT PORT CONNECTING SIGNAL OTHER THAN WIRE/CONST.\n");
                }
            } else if (c.second->output(port_name)) {
                // add into outport_conn
                if (sig.is_wire()) {
                    RTLIL::Wire *tmpWire = sig.as_wire();
                    outport_conn[tmpWire] = {c.second->type, c.second->name, port_name};
                } else {
                    log("UNEXPECTED OCCATION: OUTPUT PORT CONNECTING SIGNAL OTHER THAN WIRE.\n");
                }
            } else {
                log("UNEXPECTED OCCATION: PORT NEITHER INPUT NOR OUTPUT.\n");
            }
        }
    }


    /*
    // TODO: maybe print three dicts to debug
    */
    print_wire_conn(wire_conn);
    print_outport_conn(outport_conn);
    // print_inport_conn(inport_conn);
    print_cellinport_conn(cellinport_conn);




    // use a queue for dfs traversal
    queue<RTLIL::Wire*> wire_queue;
    wire_queue.push(output_wire);
    // first traverse wire_conn and outport_conn to build exprs for output ports
    // if is outport_conn, then build expr for the cell
    while (wire_queue.empty() == false) {
        RTLIL::Wire *curr_wire = wire_queue.front();
        wire_queue.pop();
        // search curr_wire in wire_conn
        RTLIL::SigBit pred_sigbit = wire_conn[*curr_wire];
        if (pred_sigbit != NULL) {
            // pred_sigbit is wire/const
            if (pred_sigbit.is_wire()) {
                RTLIL::Wire *pred_wire = pred_sigbit.wire;
                wire_queue.push(pred_wire);
                // TODO: generate instruction of wire assignment from pred_wire to curr_wire
                // then add to insts
                

            } else if (pred_sigbit.is_fully_const()) {
                RTLIL::Const pred_const = pred_sigbit.as_const();

            } else {
                log("UNEXPECTED OCCATION: PREDECESSOR SIGBIT IS NEITHER WIRE NOR CONST.\n");
            }
        } else {
            // pred_sigbit is NULL, so pred is not in wire queue, search if it is in outport conn
            PortInfo pred_port = outport_conn[*curr_wire];
            if (pred_port != NULL) {
                // build expr for the cell

                RTLIL::IdString tmpCellName = pred_port.cell_type;
                RTLIL::IdString tmpCellId = pred_port.cell_id;
                PortCorrespond outCorr = {pred_port.port_name; curr_wire->as_bit()};

                cell_to_expr(tmpCellName, cellinport_conn.second, outCorr); // TODO curr_wire into sigbit and cellinport
            } else {
                log("UNEXPECTED OCCATION: PREDECESSOR IS NEITHER WIRE/CONST NOR OUTPORT");
            }
        }
    }
    
    return ModuleExpr(); // TODO: return module expr
}





void get_simcells_expr() {

    // get Design of simcells.v
    std::string verilog_frontend = "verilog -nooverwrite -noblackbox";

    RTLIL::Design *simcells_lib = new RTLIL::Design; 

    Frontend::frontend_call(simcells_lib, nullptr, "+/simcells.v", verilog_frontend); // read share/simcells.v

    // TODO: use Proc and opt pass to process design into what we need
    
    Pass::call(simcells_lib, "proc");
    Pass::call(simcells_lib, "opt_expr");
    Pass::call(simcells_lib, "opt_clean");

    
    log("===================== Print simcells ======================\n");
    print_design(simcells_lib);
    


    

    /*
    // print the design 
    log("===================== Print simcells ======================\n");
    dict<RTLIL::IdString, RTLIL::Module*> modules_of_simcells = simcells_lib->modules_;
    for(std::pair<const RTLIL::IdString, RTLIL::Module*>module_pair : modules_of_simcells) {
        log("Module name: %s.\n", module_pair.first.c_str()); // print module name
        dict<RTLIL::IdString, RTLIL::Wire *> wires_of_module = module_pair.second->wires_;
        dict<RTLIL::IdString, RTLIL::Cell *> cells_of_module = module_pair.second->cells_;
        std::vector<RTLIL::SigSig> connections_of_module = module_pair.second->connections_;
        // definition of RTLIL::SigSig:	typedef std::pair<SigSpec, SigSpec> SigSig;
        // print wires_ in each module
        log("  Wires:\n");
        for(std::pair<const RTLIL::IdString, RTLIL::Wire *>wire_pair : wires_of_module) {
            log("    %s\n", wire_pair.first.c_str());
        }
        // print connections_ in each module
        log("  Connections:\n");
        for (std::pair<RTLIL::SigSpec, RTLIL::SigSpec> conn_pair : connections_of_module) {
            // print .first 
            log("    first:");
            std::string s1 = conn_pair.first.as_string();
            log(" %s\n", s1);
            // print .second
            log("    second:");
            std::string s2 = conn_pair.second.as_string();
            log(" %s\n", s2);
        }
        // print cells_ in each module
        log("  Cells:\n");
        for(std::pair<const RTLIL::IdString, RTLIL::Cell *>cell_pair : cells_of_module) {
            // print cell name and cell type
            log("    cell %s, type %s\n", cell_pair.second->name.c_str(), cell_pair.second->type.c_str());
            // print cell connections
            dict<RTLIL::IdString, RTLIL::SigSpec> conn_of_cell = cell_pair.second->connections_;
            for(std::pair<RTLIL::IdString, RTLIL::SigSpec> conn_pair : conn_of_cell) {
                // print port name and sigspec
                log("      port %s: sigspec ", conn_pair.first.c_str());
                RTLIL::SigSpec sig = conn_pair.second;
                log("%s", sig.as_string());
            }
            log("\n");
            // print cell parameters 
            for(std::pair<RTLIL::IdString, RTLIL::Const> param_pair : cell_pair.second->parameters) {
                log("      parameter %s: ", param_pair.first.c_str());
                RTLIL::Const param = param_pair.second;
                std::vector<RTLIL::State> param_bits = param.to_bits();
                for(RTLIL::State b : param_bits) {
                    log("%d ", (int)b);
                }
                log("\n");
            }

        }
    }
    log("=========================================================\n");
    */
    
    // DesignExpr *simcells_design = new DesignExpr; 

    // transform into expr formation 
    /*
    for(std::pair<const RTLIL::IdString, RTLIL::Module*>module_pair : simcells_lib->modules_) {
        ModuleExpr tmp_mod_expr =  module_to_expr(module_pair.second);
        // simcells_design->modules_[module_pair.first] = new ModuleExpr(tmp_mod_expr);
    }
    */
    
    
    
        
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
        log("\n");

    }
    void execute(std::ostream *&f, std::string filename, std::vector<std::string> args, RTLIL::Design *design) override {
        log_header(design, "Executing MV backend.\n");

        // TODO: Process arguments
        // EXTEND: Arguments 

        noglitch = false;


        size_t argidx; 
        for (argidx = 1; argidx < args.size(); argidx++) {
            std::string arg = args[argidx];
            if (arg == "-noglitch") {
                noglitch = true;
                continue;
            }
            cmd_error(args, argidx, "Unknown option or option in arguments.");
        }
        extra_args(f, filename, args, argidx); // write file content to ostream f 
        
        *f << stringf("/* Generated by Yosys_expr based on %s */\n", yosys_maybe_version());

        get_simcells_expr();

        // TODO: Implement MV backend logic

        // first get simcells.v, transform into expr formation 
        // see proc expr

        /*
        std::string verilog_frontend = "verilog -nooverwrite -noblackbox";

        RTLIL::Design *simcells_lib = new RTLIL::Design; 

        Frontend::frontend_call(simcells_lib, nullptr, "+/simcells.v", verilog_frontend); // read share/simcells.v

        // print the design 
        log("===================== Print simcells ======================\n");
        dict<RTLIL::IdString, RTLIL::Module*> modules_of_simcells = simcells_lib->modules_;
        for(std::pair<const RTLIL::IdString, RTLIL::Module*>module_pair : modules_of_simcells) {
            log("Module name: %s.\n", module_pair.first.c_str());
            dict<RTLIL::IdString, RTLIL::Wire *> wires_of_module = module_pair.second->wires_;
            dict<RTLIL::IdString, RTLIL::Cell *> cells_of_module = module_pair.second->cells_;
            log("  Wires:\n");
            for(std::pair<const RTLIL::IdString, RTLIL::Wire *>wire_pair : wires_of_module) {
                log("    %s\n", wire_pair.first.c_str());
            }
            log("  Cells:\n");
            for(std::pair<const RTLIL::IdString, RTLIL::Cell *>cell_pair : cells_of_module) {
                dict<RTLIL::IdString, RTLIL::SigSpec> conn_of_cell = cell_pair.second->connections_;
                // parameters
                log("    %s: type %s, conn ", cell_pair.first.c_str(), cell_pair.second->type.c_str());
                for(std::pair<const RTLIL::IdString, RTLIL::SigSpec>conn_pair : conn_of_cell) {
                    log("%s ", conn_pair.first.c_str());
                }
                log("\n");
            }
        }
        log("=========================================================\n");
        */

        
        // then calculate leak expr and add to hardware file 

        // finally write mv program to ostream f 


    }
} MvBackend;

PRIVATE_NAMESPACE_END

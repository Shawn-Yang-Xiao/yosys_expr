/*
  1. Define expr 
  2. Read simcells.v into rtlil internal representation
  3. Transform into expr formation
  4. Calculate leak expr
*/

#include <cstring>
#include <queue>
#include <vector>

#include "proc_expr.h"
#include "kernel/yosys.h"
#include "kernel/rtlil.h"


void print_const() {
    log("PRINT A CONST, CURRENTLY NOT SOLVED, KEEP MONITORING.");
}

void print_sigchunk(const RTLIL::SigChunk chunk){
    if(chunk.wire == NULL) { // distinguish between const and wire
        print_const();
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
}


void print_sigspec (RTLIL::SigSpec sig) {
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
/*
Instruction cell_to_expr(RTLIL::IdString cell_type, std::vector<PortCorrespond> inputs, std::vector<PortCorrespond> output ) { // Add a vector of input signals, and output signal,
    // generate an instruction from the expr
    Instruction ret;
    std::string cell_type_name = cell_type.c_str();
    std::string lhs_name = output.sig->wire.name.c_str();
    // fetch input signals


    switch (cell_type_name)
        case "$not" : 
            ret.kind = IK_subst;
            ret.lhs = Var(lhs_name);
            
            break;
        case "$pos" :
            break;
        case "$neg" :
            break;
        default:
            log("UNEXPECTED OCCASION: CELL %s IN MODULE.\n", cell_type_name);
}
*/

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
    dict<RTLIL::Wire, RTLIL::SigBit> wire_conn;
    // port -- wire , outport_conn[wire] = port
    dict<RTLIL::Wire, PortInfo> outport_conn;
    // wire/const -- port inport_conn[port] = wire/const
    dict<PortInfo, RTLIL::SigBit> inport_conn;

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
                RTLIL::SigBit rhs_sigbit = rhs.as_bit();
                wire_conn[*lhs_wire] = rhs_sigbit;
            } else {
                log("UNEXPECTED OCCATION: RIGHT HAND SIDE OF CONNECTION IS NOT WIRE/CONST.\n");
            }
        } else {
            log("UNEXPECTED OCCATION: LEFT HAND SIDE OF CONNECTION IS NOT WIRE.\n");
        }
    }
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
                    RTLIL::SigBit tmpSigbit = sig.as_bit();
                    inport_conn[{c.second->type, c.second->name, port_name}] = tmpSigbit;
                } else {
                    log("UNEXPECTED OCCATION: INPUT PORT CONNECTING SIGNAL OTHER THAN WIRE/CONST.\n");
                }
            } else if (c.second->output(port_name)) {
                // add into outport_conn
                if (sig.is_wire()) {
                    RTLIL::Wire *tmpWire = sig.as_wire();
                    outport_conn[*tmpWire] = {c.second->type, c.second->name, port_name};
                } else {
                    log("UNEXPECTED OCCATION: OUTPUT PORT CONNECTING SIGNAL OTHER THAN WIRE.\n");
                }
            } else {
                log("UNEXPECTED OCCATION: PORT NEITHER INPUT NOR OUTPUT.\n");
            }
        }
    }
    // generate cell inport connection dict, same usage as inport conn 
    for (std::pair<RTLIL::IdString, RTLIL::Cell*> c : module->cells_) {
        dict<RTLIL::IdString, RTLIL::SigSpec> conns = c.second->connections_;
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> p : conns) {
            RTLIL::IdString port_name = p.first;
            RTLIL::SigSpec sig = p.second;
            if (c.second->input(port_name)) {
                if (sig.is_fully_const() || sig.is_wire()) {
                    RTLIL::SigBit tmpSigbit = sig.as_bit();
                    cellinport_conn[c.second->name].push_back( {port_name, tmpSigbit} );
                } else {
                    log("UNEXPECTED OCCATION: INPUT PORT CONNECTING SIGNAL OTHER THAN WIRE/CONST.\n");
                }
            } else if (c.second->output(port_name)) {
                // add into outport_conn
                if (sig.is_wire()) {
                    RTLIL::Wire *tmpWire = sig.as_wire();
                    outport_conn[*tmpWire] = {c.second->type, c.second->name, port_name};
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
                // Instruction cell_to_expr(RTLIL::IdString cell_type, std::vector<RTLIL::PortCorrespond> inputs, std::vector<RTLIL::PortCorrespond> output )
                
                // struct PortInfo{
                //     RTLIL::IdString cell_type;
                //     RTLIL::IdString cell_id;
                //     RTLIL::IdString port_name;
                // };
                
                RTLIL::IdString tmpCellName = pred_port.cell_type;
                RTLIL::IdString tmpCellId = pred_port.cell_id;
                // get input signal pairs
                // TODO: how to search information of cell with cell id? maybe there are given ways to search cell lib for ports 
                
                // generate output signal pair 

                cell_to_expr(tmpCellName, );
            } else {
                log("UNEXPECTED OCCATION: PREDECESSOR IS NEITHER WIRE/CONST NOR OUTPORT");
            }
        }
    }
    */
    return ModuleExpr(); // TODO: return module expr
}





void get_simcells_expr() {

    // get Design of simcells.v
    std::string verilog_frontend = "verilog -nooverwrite -noblackbox";

    RTLIL::Design *simcells_lib = new RTLIL::Design; 

    Frontend::frontend_call(simcells_lib, nullptr, "+/simcells.v", verilog_frontend); // read share/simcells.v


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
        simcells_design->modules_[module_pair.first] = new ModuleExpr(tmp_mod_expr);
        
    }
    */
        
}





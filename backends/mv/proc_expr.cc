/*
  1. Define expr 
  2. Read simcells.v into rtlil internal representation
  3. Transform into expr formation
  4. Calculate leak expr
*/

#include <cstring>

#include "proc_expr.h"
#include "kernel/yosys.h"


void print_


void print_cell(){}


void print_module(const RTLIL::Module *module) {
    log("Module name: %s.\n", module->second.c_str());
    ;
}

void print_design(const RTLIL::Design *design){
    dict<RTLIL::IdString, RTLIL::Module*> modules_of_simcells = design->modules_;
    for(std::pair<const RTLIL::IdString, RTLIL::Module*>module_pair : modules_of_simcells) {
        log("Module id: %s, ", module_pair.first.c_str());
        print_module(module_pair.second);
    }

}


/*
ModuleExpr module_to_expr(const RTLIL::Module *module) {
    // transform basic cell definition module into expression form

    // first collect wires, find input and output wires
    // 	dict<RTLIL::IdString, RTLIL::Wire*> wires_;
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
    // Traverse cells to build wire connection graph
    // generate WireConn structs for each wire 
    for (std::pair<RTLIL::IdString, RTLIL::Cell*> c : module->cells_) {
        dict<RTLIL::IdString, RTLIL::SigSpec> conns = c.second->connections_;
        // for each conn, find predecessor and successors
        // assume only one predecessor and one successor for each wire in simcells
        for (std::pair<RTLIL::IdString, RTLIL::SigSpec> p : conns) {
            RTLIL::IdString port_name = p.first;
            RTLIL::SigSpec sig = p.second;
            // for each sig, get wire name 
            for (const RTLIL::SigChunk &chunk : sig.chunks()) {
                RTLIL::IdString wire_name = chunk.wire->name;
                // if port is input port of the cell, then wire is predecessor
                // if port is output port of the cell, then wire is successor
                if (is_input_port(c.second->type, port_name)) {
                    // wire is predecessor 
                } else if (is_output_port(c.second->type, port_name)) {
                    // wire is successor
                }
            }
        }

    }
    // then topologically sort cells and transform into assign expressions
    std::vector<RTLIL::IdString> sorted_cells; // store cell names in topological order

    for () {

    }
}
*/

void get_simcells_expr() {

    // get Design of simcells.v
    std::string verilog_frontend = "verilog -nooverwrite -noblackbox";

    RTLIL::Design *simcells_lib = new RTLIL::Design; 

    Frontend::frontend_call(simcells_lib, nullptr, "+/simcells.v", verilog_frontend); // read share/simcells.v


    log("===================== Print simcells ======================\n");
    print_design(simcells_lib);

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
            /*
            // print chunks_
            log("      chunk: ");
            for (RTLIL::SigChunk fst_chunk : conn_pair.first.chunks_) {
                if (fst_chunk.wire != NULL) {
                    log("(%s, offset %d, width %d) ", fst_chunk.wire->name.c_str(), fst_chunk.offset, fst_chunk.width);
                } else {
                    log("(");
                    for (RTLIL::State s : fst_chunk.data) {
                        log("%d ", s);
                    }
                    log("width %d)", fst_chunk.width);
                }
            }
            log("\n");
            // print bits_
            log("      bits: ");
            for (RTLIL::SigBit fst_bit : conn_pair.first.bits_) {
                if (fst_bit.wire != NULL) {
                    log("(%s, offset %d) ", fst_bit.wire->name.c_str(), fst_bit.offset);
                } else {
                    log("(const %d) ", fst_bit.data);
                }
            }
            log("\n");
            */
            // print .second
            log("    second:");
            std::string s2 = conn_pair.second.as_string();
            log(" %s\n", s2);
            /*
            // print chunks_
            log("      chunk: ");
            for (RTLIL::SigChunk snd_chunk : conn_pair.second.chunks_) {
                if (snd_chunk.wire != NULL) {
                    log("(%s, offset %d, width %d)", snd_chunk.wire->name.c_str(), snd_chunk.offset, snd_chunk.width);
                }
            }
            // print bits_
            for(RTLIL::SigBit snd_bit : conn_pair.second.bits_) {
                if (snd_bit.wire != NULL) {
                    log("(%s, offset %d) ", snd_bit.wire->name.c_str(), snd_bit.offset);
                } else {
                    log("(const %d) ", snd_bit.data);
                }
                log("\n");
            }
            log("\n");
            */
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
                /*
                // print chunks_
                log("        chunk: ");
                for (RTLIL::SigChunk chunk : sig.chunks_) {
                    if (chunk.wire != NULL) {
                        log("(%s, offset %d, width %d) ", chunk.wire->name.c_str(), chunk.offset, chunk.width);
                    } else {
                        log("(");
                        for (RTLIL::State s : chunk.data) {
                            log("%d ", s);
                        }
                        log("width %d)", chunk.width);
                    }
                }
                log("\n");
                // print bits_
                log("        bits: ");
                for (RTLIL::SigBit bit : sig.bits_) {
                    if (bit.wire != NULL) {
                        log("(%s, offset %d) ", bit.wire->name.c_str(), bit.offset);
                    } else {
                        log("(const %d) ", bit.data);
                    }
                }
                log("\n");
                */
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
    
    // DesignExpr *simcells_design = new DesignExpr; 

    // transform into expr formation 
    /*
    for(std::pair<const RTLIL::IdString, RTLIL::Module*>module_pair : simcells_lib->modules_) {
        ModuleExpr tmp_mod_expr =  module_to_expr(module_pair.second);
        simcells_design->modules_[module_pair.first] = new ModuleExpr(tmp_mod_expr);
        
    }
    */
        
}





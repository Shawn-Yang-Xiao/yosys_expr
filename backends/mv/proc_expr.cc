/*
  1. Define expr 
  2. Read simcells.v into rtlil internal representation
  3. Transform into expr formation
  4. Calculate leak expr
*/


void get_simcells_expr() {

    // get Design of simcells.v
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


    
        
}
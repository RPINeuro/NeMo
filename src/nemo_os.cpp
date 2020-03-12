//
// Created by Mark Plagge on 2/27/20.
//

/** FCFS NeMo Initialization System */


#include <iostream>

#include "libs/neuro_os_config/include/neuro_os_config.h"

#include <ross.h>
#include "nemo_os_system.h"

#include "neuro/fcfs_core.hpp"
//#include "mapping.h"

#include "../libs/model_reader/include/tn_parser.hh"
//extern "C" {
//#include "IO/output.h"
//}



//#include "fcfs_logic_system.h"
//#include "neruo/fcfs_core.h"
bool DO_RANDOM_PROCESSES = true;
char NEURO_OS_CONFIG_FILE_PATH[512] = {'\0'};
size_type LPS_PER_PE;

const tw_optdef app_opt[] = {
        TWOPT_GROUP("Simulated Process Mode"),
        TWOPT_FLAG("rproc", DO_RANDOM_PROCESSES,"Randomized Processes?"),
        TWOPT_CHAR("oscfg",NEURO_OS_CONFIG_FILE_PATH,"NeuroOS config file location"),
        TWOPT_ULONGLONG("cores", CORES_IN_SIM, "number of cores in simulation"),
        TWOPT_END()
};
namespace neuro_os {

    bool is_gid_os(tw_lpid gid){
        //512 cores, 512 neurons = (512 * (512 * 2 + 1)) LPs for TN
        auto num_lps = g_tw_nlp * g_tw_npe;
        return gid == num_lps;
    }

    tw_peid neuro_os_mapping(tw_lpid gid){
        if (is_gid_os(gid)){
            return (tw_peid) gid;
        }else{
            return getPEFromGID(gid);
        }
    }
    /**
     * neuroOS type map - overrides nemo's type map
     * For FCFS, the last LP is the FCFS arbiter core
     * The rest of the LPs follow NeMo's config.
     *
     * @param gid
     * @return
     */
    tw_lpid neuro_os_lp_typemapper(tw_lpid gid){
        if (is_gid_os(gid)){
            return 3;
        }else{
            return(lpTypeMapper(gid));
        }

    }
/** NeuroOS Model LPs */
    tw_lptype nos_model_lps[] = {
            {

                    (init_f) axon_init,
                    (pre_run_f) nullptr,
                    (event_f) axon_event,
                    (revent_f) axon_reverse,
                    (commit_f) axon_commit,
                    (final_f) axon_final,
                    (map_f) neuro_os_mapping,
                    sizeof(axonState)},
            {
                    (init_f) synapse_init,
                    (pre_run_f) synapse_pre_run,
                    (event_f) synapse_event,
                    (revent_f) synapse_reverse,
                    (commit_f) nullptr,
                    (final_f) synapse_final,
                    (map_f) neuro_os_mapping,
                    sizeof(synapse_state)
            },
            {
                    (init_f) TN_init,
                    (pre_run_f) TN_pre_run,
                    (event_f) TN_forward_event,
                    (revent_f) TN_reverse_event,
                    (commit_f) TN_commit,
                    (final_f) TN_final,
                    (map_f) neuro_os_mapping,
                    sizeof(tn_neuron_state)
            },
            {
                    (init_f)    fcfs_core_init,
                    (pre_run_f) fcfs_pre_run,
                    (event_f)   fcfs_forward_event,
                    (revent_f)  fcfs_reverse_event,
                    (commit_f)  fcfs_commit_event,
                    (final_f)   fcfs_final,
                    (map_f) neuro_os_mapping,
                    sizeof(tn_neuron_state)
            },
            {0}};
    void display_nemo_os_settings(){
        printf("* \t %i Neurons per core (cmake defined), %llu cores in sim.\n", NEURONS_IN_CORE, CORES_IN_SIM);

    }
    void init_nemo_os(){
        using config::global_neuro_config;
        FILE_OUT = true;
        GRID_ENABLE = false;
        IS_SAT_NET = false;
        CORES_IN_CHIP = 1;
        CORES_PER_LAYER = CORES_IN_SIM;
        SIM_SIZE = global_neuro_config->num_chips_in_sim();


        //CORE_SIZE = SYNAPSES_IN_CORE + NEURONS_IN_CORE + AXONS_IN_CORE;
        auto CS = SYNAPSES_IN_CORE + NEURONS_IN_CORE + AXONS_IN_CORE;
        SIM_SIZE = CS * CORES_IN_SIM;
        tw_printf(TW_LOC, "Cores in sim: %i", CORES_IN_SIM);
        g_tw_nlp = (SIM_SIZE / tw_nnodes()) + 1;
        g_tw_lookahead = 0.001;
        g_tw_lp_types = nos_model_lps;
        g_tw_lp_typemap = neuro_os_lp_typemapper;
        g_tw_events_per_pe = NEURONS_IN_CORE * AXONS_IN_CORE * 128;
        LPS_PER_PE = g_tw_nlp / g_tw_npe;
        initOutFiles();
        initDataStructures(g_tw_nlp);
        NUM_CHIPS_IN_SIM = 1;
        CHIPS_PER_RANK = 1;
    }

}
using namespace neuro_os;
int nemo_os_main(int argc, char *argv[]){
    // Load the config file ## HARD CODED NOW ##
    tw_opt_add(app_opt);
    std::string fileloc;
    if (NEURO_OS_CONFIG_FILE_PATH[0] == '\0'){
        fileloc = std::string("../configs/fcfs_base.json");
    }else {
        fileloc = std::string(NEURO_OS_CONFIG_FILE_PATH);
    }
    std::cout << "Neuro OS Testing\n";
    std::ifstream i(fileloc);
    json j;
    i >> j;
    auto neuro_os_config = j.get<config::neuro_os_configuration>();
    config::global_neuro_config = std::make_unique<config::neuro_os_configuration>(neuro_os_config);
    init_nemo_os();

    return 0;
}

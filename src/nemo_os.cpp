//
// Created by Mark Plagge on 2/27/20.
//

/** FCFS NeMo Initialization System */


#include <iostream>

#include <ross.h>
#include "nemo_os_system.h"
#include "neuro/fcfs_core.h"
#include "mapping.h"
#include "nemo_main.h"

//#include "fcfs_logic_system.h"
//#include "neruo/fcfs_core.h"
bool DO_RANDOM_PROCESSES = true;
char NEURO_OS_CONFIG_FILE_PATH[512] = {'\0'};


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
        SIM_SIZE = CORE_SIZE*CORES_IN_SIM;
    }

}

int nemo_os_main(int argc, char *argv[]){
    std::cout << "Neuro OS Testing\n" ;







    return 0;
}
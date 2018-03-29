#include "util.h"
#include "inline_util.h"

void *run_client(void *arg)
{
    int i, j;
    struct thread_params params = *(struct thread_params *) arg;
    int clt_gid = (machine_id * CLIENTS_PER_MACHINE) + params.id;	/* Global ID of this client thread */
    uint16_t local_client_id = (uint16_t) (clt_gid % CLIENTS_PER_MACHINE);
    uint16_t worker_qp_i = (uint16_t) ((local_client_id + machine_id) % WORKER_NUM_UD_QPS);
    uint16_t local_worker_id = (uint16_t) ((clt_gid % LOCAL_WORKERS) + ACTIVE_WORKERS_PER_MACHINE); // relevant only if local workers are enabled
    if (local_client_id == 0 && machine_id == 0)
        cyan_printf("Size of worker req: %d, extra bytes: %d, ud req size: %d minimum worker req size %d, actual size of req_size %d  \n",
                    WORKER_REQ_SIZE, EXTRA_WORKER_REQ_BYTES, UD_REQ_SIZE, MINIMUM_WORKER_REQ_SIZE, sizeof(struct wrkr_ud_req));
    int protocol = SEQUENTIAL_CONSISTENCY;
    int *recv_q_depths, *send_q_depths;
    uint16_t remote_buf_size =  ENABLE_WORKER_COALESCING == 1 ?
                                (GRH_SIZE + sizeof(struct wrkr_coalesce_mica_op)) : UD_REQ_SIZE ;
//    printf("Remote clients buffer size %d\n", remote_buf_size);
    set_up_queue_depths(&recv_q_depths, &send_q_depths, protocol);
    struct hrd_ctrl_blk *cb = hrd_ctrl_blk_init(clt_gid,	/* local_hid */
                                                0, -1, /* port_index, numa_node_id */
                                                0, 0,	/* #conn qps, uc */
                                                NULL, 0, 0,	/* prealloc conn buf, buf size, key */
                                                CLIENT_UD_QPS, remote_buf_size + SC_CLT_BUF_SIZE,	/* num_dgram_qps, dgram_buf_size */
                                                MASTER_SHM_KEY + WORKERS_PER_MACHINE + local_client_id, /* key */
                                                recv_q_depths, send_q_depths); /* Depth of the dgram RECV Q*/

    int push_ptr = 0, pull_ptr = -1;
    struct ud_req *incoming_reqs = (struct ud_req *)(cb->dgram_buf + remote_buf_size);
//    printf("dgram pointer %llu, incoming req ptr %llu \n", cb->dgram_buf, incoming_reqs);

    /* ---------------------------------------------------------------------------
    ------------------------------MULTICAST SET UP-------------------------------
    ---------------------------------------------------------------------------*/

    struct mcast_info *mcast_data;
    struct mcast_essentials *mcast;
    // need to init mcast before sync, such that we can post recvs
    if (ENABLE_MULTICAST == 1) {
        init_multicast(&mcast_data, &mcast, local_client_id, cb, protocol);
        assert(mcast != NULL);
    }

    /* Fill the RECV queue that receives the Broadcasts, we need to do this early */
    if (WRITE_RATIO > 0 && DISABLE_CACHE == 0)
        post_coh_recvs(cb, &push_ptr, mcast, protocol, (void*)incoming_reqs);

    /* -----------------------------------------------------
    --------------CONNECT WITH WORKERS AND CLIENTS-----------------------
    ---------------------------------------------------------*/
    setup_client_conenctions_and_spawn_stats_thread(clt_gid, cb);
    if (MULTICAST_TESTING == 1) multicast_testing(mcast, clt_gid, cb);
    /* -----------------------------------------------------
    --------------DECLARATIONS------------------------------
    ---------------------------------------------------------*/
    int key_i, ret;
    struct ibv_send_wr rem_send_wr[WINDOW_SIZE], coh_send_wr[MESSAGES_IN_BCAST_BATCH],
            credit_send_wr[SC_MAX_CREDIT_WRS], *bad_send_wr;
    struct ibv_sge rem_send_sgl[WINDOW_SIZE], coh_send_sgl[MAX_BCAST_BATCH], credit_send_sgl;
    struct ibv_wc wc[MAX_REMOTE_RECV_WCS], signal_send_wc, credit_wc[SC_MAX_CREDIT_WRS], coh_wc[SC_MAX_COH_RECEIVES];
    struct ibv_recv_wr rem_recv_wr[WINDOW_SIZE], coh_recv_wr[SC_MAX_COH_RECEIVES],
            credit_recv_wr[SC_MAX_CREDIT_RECVS], *bad_recv_wr;
    struct ibv_sge rem_recv_sgl, coh_recv_sgl[SC_MAX_COH_RECEIVES], credit_recv_sgl;
    struct coalesce_inf coalesce_struct[MACHINE_NUM] = {0};

    long long remote_for_each_worker[WORKER_NUM] = {0}, rolling_iter = 0, trace_iter = 0,
            credit_tx = 0, br_tx = 0, remote_tot_tx = 0;
    uint8_t rm_id = 0, per_worker_outstanding[WORKER_NUM] = {0}, credits[SC_VIRTUAL_CHANNELS][MACHINE_NUM],
            broadcasts_seen[MACHINE_NUM] = {0};
    uint16_t wn = 0, wr_i = 0, br_i = 0, cb_i = 0, coh_i = 0, coh_buf_i = 0, rem_req_i = 0, prev_rem_req_i,
            credit_wr_i = 0, op_i = 0, next_op_i = 0, last_measured_wr_i = 0,
            ws[WORKERS_PER_MACHINE] = {0}; /* Window slot to use for LOCAL workers*/
    uint16_t previous_wr_i, worker_id = 0, print_count = 0, credit_rec_counter = 0;
    uint32_t cmd_count = 0, credit_debug_cnt = 0, outstanding_rem_reqs = 0;
    double empty_req_percentage;
    struct local_latency local_measure = {
            .measured_local_region = -1,
            .local_latency_start_polling = 0,
            .flag_to_poll = NULL,
    };
    struct latency_flags latency_info = {
            .measured_req_flag = NO_REQ,
            .last_measured_op_i = 0,
    };

    struct cache_op *update_ops;
    struct key_home *key_homes, *next_key_homes, *third_key_homes;
    struct mica_resp *resp, *next_resp, *third_resp;
    struct mica_op *coh_buf;
    struct mica_resp update_resp[BCAST_TO_CACHE_BATCH] = {0}; // not sure why this is on stack
    struct ibv_mr *ops_mr, *coh_mr;
    struct extended_cache_op *ops, *next_ops, *third_ops;

    set_up_ops(&ops, &next_ops, &third_ops, &resp, &next_resp, &third_resp,
               &key_homes, &next_key_homes, &third_key_homes);
    set_up_coh_ops(&update_ops, NULL, NULL, NULL, update_resp, NULL, &coh_buf, protocol);
    set_up_mrs(&ops_mr, &coh_mr, ops, coh_buf, cb);
    struct ibv_cq *coh_recv_cq = ENABLE_MULTICAST == 1 ? mcast->recv_cq : cb->dgram_recv_cq[BROADCAST_UD_QP_ID];
    struct ibv_qp *coh_recv_qp = ENABLE_MULTICAST == 1 ? mcast->recv_qp : cb->dgram_qp[BROADCAST_UD_QP_ID];
    uint16_t hottest_keys_pointers[HOTTEST_KEYS_TO_TRACK] = {0};

    /* ---------------------------------------------------------------------------
    ------------------------------INITIALIZE STATIC STRUCTUREs--------------------
        ---------------------------------------------------------------------------*/
    // SEND AND RECEIVE WRs
    set_up_remote_WRs(rem_send_wr, rem_send_sgl, rem_recv_wr, &rem_recv_sgl, cb, clt_gid, ops_mr, protocol);
    if (WRITE_RATIO > 0 && DISABLE_CACHE == 0) {
        set_up_credits(credits, credit_send_wr, &credit_send_sgl, credit_recv_wr, &credit_recv_sgl, cb, protocol);
        set_up_coh_WRs(coh_send_wr, coh_send_sgl, coh_recv_wr, coh_recv_sgl,
                       NULL, NULL, coh_buf, local_client_id, cb, coh_mr, mcast, protocol);
    }
    // TRACE
    struct trace_command *trace;
    trace_init(&trace, clt_gid);

    /* ---------------------------------------------------------------------------
    ------------------------------Prepost RECVS-----------------------------------
    ---------------------------------------------------------------------------*/
    /* Remote Requests */
    for(i = 0; i < WINDOW_SIZE; i++) /// Warning: take care that coalesced worker responses do not overwrite coherence
        hrd_post_dgram_recv(cb->dgram_qp[REMOTE_UD_QP_ID],
                            (void *) cb->dgram_buf, remote_buf_size, cb->dgram_buf_mr->lkey);

    memset(per_worker_outstanding, 0, WORKER_NUM);
    struct timespec start, end;
    uint32_t dbg_per_worker[3]= {0};


    /* ---------------------------------------------------------------------------
    ------------------------------START LOOP--------------------------------
    ---------------------------------------------------------------------------*/
    while (1) {
        // Swap the op buffers to facilitate correct ordering
        swap_ops(&ops, &next_ops, &third_ops,
                 &resp, &next_resp, &third_resp,
                 &key_homes, &next_key_homes, &third_key_homes);
        if (credit_debug_cnt > M_1) {
            red_printf("Client %d misses credits \n", clt_gid);
            // exit(0);
            credit_debug_cnt = 0;
        }

        /* ---------------------------------------------------------------------------
        ------------------------------Refill REMOTE  RECVS--------------------------------
        ---------------------------------------------------------------------------*/

        refill_recvs(rem_req_i, rem_recv_wr, cb);

        /* ---------------------------------------------------------------------------
        ------------------------------ POLL BROADCAST REGION--------------------------
        ---------------------------------------------------------------------------*/
        coh_i = 0;
        if (WRITE_RATIO > 0 && DISABLE_CACHE == 0) {
            poll_coherence_SC(&coh_i, incoming_reqs, &pull_ptr, update_ops, update_resp, clt_gid, local_client_id);

            // poll recv completions-send credits and post new receives
            if (coh_i > 0) {
                // poll for the receive completions for the coherence messages that were just polled
                hrd_poll_cq(coh_recv_cq, coh_i, coh_wc);
                // create the appropriate amount of credits to send back
                credit_wr_i = create_credits_SC(coh_i, coh_wc, broadcasts_seen, local_client_id,
                                                credit_send_wr, &credit_tx, cb->dgram_send_cq[FC_UD_QP_ID]);
                // send back the created credits  and post the corresponding receives for the new messages
                if (credit_wr_i > 0)
                    send_credits(credit_wr_i, coh_recv_sgl, cb, &push_ptr, coh_recv_wr, coh_recv_qp,
                                 credit_send_wr,  (uint16_t)SC_CREDITS_IN_MESSAGE, (uint32_t)SC_CLT_BUF_SLOTS,
                                 (void*)incoming_reqs);
                // push the new coherence messages to the CACHES
                cache_batch_op_sc_with_cache_op(coh_i, local_client_id, &update_ops, update_resp);
            }
        }
        /* ---------------------------------------------------------------------------
        ------------------------------PROBE THE CACHE--------------------------------------
        ---------------------------------------------------------------------------*/
        trace_iter = batch_from_trace_to_cache(trace_iter, local_client_id, trace, ops, resp,
                                               key_homes, 0, next_op_i, &latency_info, &start,
                                               hottest_keys_pointers);

        /* ---------------------------------------------------------------------------
        ------------------------------BROADCASTS--------------------------------------
        ---------------------------------------------------------------------------*/
        if (WRITE_RATIO > 0 && DISABLE_CACHE == 0) {
            perform_broadcasts_SC(ops, credits, cb, credit_wc,
                                  &credit_debug_cnt, coh_send_sgl, coh_send_wr,
                                  coh_buf, &coh_buf_i, &br_tx, credit_recv_wr,
                                  local_client_id, protocol);
        }

        /* ---------------------------------------------------------------------------
        ------------------------------ REMOTE & LOCAL REQUESTS------------------------
        ---------------------------------------------------------------------------*/
        previous_wr_i = wr_i;
        wr_i = 0;
        prev_rem_req_i = rem_req_i;
        rem_req_i = 0; next_op_i = 0;

        empty_req_percentage = c_stats[local_client_id].empty_reqs_per_trace;
        if (ENABLE_MULTI_BATCHES == 1) {
            find_responses_to_enable_multi_batching (empty_req_percentage, cb, wc,
                                                     per_worker_outstanding, &outstanding_rem_reqs, local_client_id);
        }
        else memset(per_worker_outstanding, 0, WORKER_NUM);

        // Create the Remote Requests
        wr_i = handle_cold_requests(ops, next_ops, resp,
                                    next_resp, key_homes, next_key_homes,
                                    &rem_req_i, &next_op_i, cb, rem_send_wr,
                                    rem_send_sgl, wc, remote_tot_tx, worker_qp_i,
                                    per_worker_outstanding, &outstanding_rem_reqs, remote_for_each_worker,
                                    ws, clt_gid, local_client_id, NULL, local_worker_id, protocol,
                                    &latency_info, &start, &local_measure, hottest_keys_pointers);
        /* ---------------------------------------------------------------------------
        ------------------------------ POLL & SEND--------------------------------
        ---------------------------------------------------------------------------*/
        c_stats[local_client_id].remotes_per_client += rem_req_i;
        if (ENABLE_WORKER_COALESCING == 1) {
            rem_req_i = wr_i;
        }

        poll_and_send_remotes(previous_wr_i, local_client_id, &outstanding_rem_reqs,
                              cb, wc, prev_rem_req_i, wr_i, rem_send_wr, rem_req_i, &remote_tot_tx,
                              rem_send_sgl, &latency_info, &start);

    }
    return NULL;
}

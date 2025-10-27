/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/logging/log.h>
#include "rtt_manager.h"

LOG_MODULE_REGISTER(rtt_manager, LOG_LEVEL_INF);

/* RTT storage */
#define K_MAX_PENDING 8
static struct {
    uint64_t timestamp; /* Local timestamp for RTT */
} pending_rtt[K_MAX_PENDING];
static struct k_mutex rtt_mutex;


void rtt_manager_init(void)
{
    k_mutex_init(&rtt_mutex);
    memset(pending_rtt, 0, sizeof(pending_rtt));
}

int rtt_store_timestamp(void)
{
    k_mutex_lock(&rtt_mutex, K_FOREVER);
    for (int i = 0; i < K_MAX_PENDING; i++) {
        if (pending_rtt[i].timestamp == 0) {
            pending_rtt[i].timestamp = k_uptime_get();
            k_mutex_unlock(&rtt_mutex);
            return 0;
        }
    }
    k_mutex_unlock(&rtt_mutex);
    LOG_WRN("No free RTT slots");
    return -ENOMEM;
} 

int rtt_compute_time(void)
{
    uint64_t rtt = -1;
    k_mutex_lock(&rtt_mutex, K_FOREVER);
    for (int i = 0; i < K_MAX_PENDING; i++) {
        if (pending_rtt[i].timestamp > 0) {
            rtt = k_uptime_get() - pending_rtt[i].timestamp;
            LOG_INF("RTT calculated: %llu ms", rtt);
            pending_rtt[i].timestamp = 0; // Limpia slot
            break;
        }
    }
    k_mutex_unlock(&rtt_mutex);

    return rtt;
}

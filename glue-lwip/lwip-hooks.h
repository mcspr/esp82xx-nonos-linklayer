#pragma once

#include "lwip/opt.h"

#include "lwip/netif.h"
#include "lwip/tcp.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/prot/dhcp.h"

extern void lwip_hook_dhcp_amend_options(struct netif *netif, struct dhcp *dhcp, int state, struct dhcp_msg *msg,
										 int msg_type, u16_t *option_len_ptr);
#define LWIP_HOOK_DHCP_APPEND_OPTIONS(netif, dhcp, state, msg, msg_type, option_len_ptr) { \
   /* https://github.com/esp8266/Arduino/issues/8223 */ \
   lwip_hook_dhcp_amend_options(netif, dhcp, state, msg, msg_type, option_len_ptr); \
   /* https://github.com/esp8266/Arduino/issues/8247 */ \
   if ((msg_type) == DHCP_DISCOVER) \
      *(option_len_ptr) = dhcp_option_hostname(*(option_len_ptr), (msg)->options, netif); \
}

/** after tcp_init, randomize port in tcp_new_port yet again instead of
 * simply incrementing by 1. note of lazy expansion, this gets called
 * in the middle of tcp.c */
#if defined LWIP_RAND
#define LWIP_HOOK_TCP_NEW_PORT(PORT) \
	(PORT) = TCP_ENSURE_LOCAL_PORT_RANGE(LWIP_RAND())
#endif

/**
 * port @me-no-dev time-wait tweak
 * https://github.com/esp8266/Arduino/commit/07f4d4c241df2c552899857f39a4295164f686f2#diff-f8258e71e25fb9985ca3799e3d8b88ecR399
 
 * tcp_abort(struct tcp_pcb*) N pcbs at the tail of the list. pcbs is usually &tcp_tw_limit, which is
 * sorted by increasing `tcp_ticks - pcb->tmr` i.e. oldest inactive pcbs are in the tail of the list.
 * tcp_abort() would also helpfully remove pcb from the pcbs, assume that pcb->next might be updated.
 *
 * pcb->tmr usually updated before pcb TCP_REG()'ed into the list, and whenever repeated FIN arrives.
 * see tcp_in.c tcp_process(struct tcp_pcb*) and tcp_timewait_input(struct tcp_pcb*) */
static inline
void tcp_limit_pcbs(struct tcp_pcb **pcbs, u32_t limit)
{
	if (!limit || !*pcbs)
		return;

    u32_t count = 0;
	struct tcp_pcb* tail;
	struct tcp_pcb* tmp;

	tail = NULL;

	for (tmp = *pcbs; tmp != NULL; tmp = tmp->next) {
		++count;
		if (count > limit) {
			tail = tmp;
			break;
		}
    }

	if (tail != NULL) {
		do {
			tmp = tail->next;
			if (tmp == NULL) {
				tmp = tail;
			}

            tcp_abort(tmp);
        } while (tmp != tail);
    }
}

#ifndef MEMP_NUM_TCP_PCB_TIME_WAIT
#error MEMP_NUM_TCP_PCB_TIME_WAIT must be set
#else
#if (MEMP_NUM_TCP_PCB_TIME_WAIT) < 0
#error MEMP_NUM_TCP_PCB_TIME_WAIT must be positive
#endif
#endif

/** this gets called before TCP_REG, assume theres one more pcb in the list */
#define TCP_TW_LIMIT(pcbs) tcp_limit_pcbs(pcbs, MEMP_NUM_TCP_PCB_TIME_WAIT - 1)

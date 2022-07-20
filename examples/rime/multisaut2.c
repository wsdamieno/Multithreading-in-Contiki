/**
 * \addtogroup rimemh
 * @{
 */

/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Multihop forwarding
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "net/rime/multihop.h"
#include "net/rime/route.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"

#include <string.h>

static const struct packetbuf_attrlist attributes[] =
  {
    MULTIHOP_ATTRIBUTES
    PACKETBUF_ATTR_LAST
  };
#define CHANNEL 150
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
PROCESS(example_multihop_process, "multihop example");
AUTOSTART_PROCESSES(&example_multihop_process);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void
data_packet_received2(struct unicast_conn *uc, const rimeaddr_t *from)
{
  struct multihop_conn *c = (struct multihop_conn *)uc;
  rimeaddr_t *nexthop;
  rimeaddr_t sender, receiver;

  /* Copy the packet attributes to avoid them being overwritten or
     cleared by an application program that uses the packet buffer for
     its own needs. */
  rimeaddr_copy(&sender, packetbuf_addr(PACKETBUF_ADDR_ESENDER));
  rimeaddr_copy(&receiver, packetbuf_addr(PACKETBUF_ADDR_ERECEIVER));

  PRINTF("data_packet_received from %d.%d towards %d.%d len %d\n",
	 from->u8[0], from->u8[1],
	 packetbuf_addr(PACKETBUF_ADDR_ERECEIVER)->u8[0],
	 packetbuf_addr(PACKETBUF_ADDR_ERECEIVER)->u8[1],
	 packetbuf_datalen());

  if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_ERECEIVER),
				 &rimeaddr_node_addr)) {
    PRINTF("for us!\n");
    if(c->cb->recv) {
      c->cb->recv(c, &sender, from,
		  packetbuf_attr(PACKETBUF_ATTR_HOPS));
    }
  } else {
    nexthop = NULL;
    if(c->cb->forward) {
      packetbuf_set_attr(PACKETBUF_ATTR_HOPS,
			 packetbuf_attr(PACKETBUF_ATTR_HOPS) + 1);
      nexthop = c->cb->forward(c, &sender, &receiver,
			       from, packetbuf_attr(PACKETBUF_ATTR_HOPS) - 1);
    }
    if(nexthop) {
      PRINTF("forwarding to %d.%d\n", nexthop->u8[0], nexthop->u8[1]);
      unicast_send(&c->c, nexthop);
    }
  }
}
/*---------------------------------------------------------------------------*/
static const struct multihop_callbacks data_callbacks = { data_packet_received2 };
static struct multihop_conn multihop;
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(example_multihop_process, ev, data)
{
  //PROCESS_EXITHANDLER(multihop_close(&multihop);)
    
  PROCESS_BEGIN();
		static struct etimer et;
   /* Open a multihop connection on Rime channel CHANNEL. */
  multihop_open(&multihop, CHANNEL , &data_callbacks);

  /* Activate the button sensor. We use the button to drive traffic -
     when the button is pressed, a packet is sent. */

  /* Loop forever, send a packet when the button is pressed. */
  while(1) {
    rimeaddr_t to;

    etimer_set(&et, CLOCK_SECOND * 4);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* Copy the "Hello" to the packet buffer. */
    packetbuf_copyfrom("Hello", 6);

    /* Set the Rime address of the final receiver of the packet to
       1.0. This is a value that happens to work nicely in a Cooja
       simulation (because the default simulation setup creates one
       node with address 1.0). */
    to.u8[0] = 1;
    to.u8[1] = 0;

    /* Send the packet. */
    multihop_send(&multihop, &to);

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

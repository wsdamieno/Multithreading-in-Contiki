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
 *         Testing the multihop forwarding layer (multihop) in Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 *
 *         This example shows how to use the multihop Rime module, how
 *         to use the announcement mechanism, how to manage a list
 *         with the list module, and how to allocate memory with the
 *         memb module.
 *
 *         The multihop module provides hooks for forwarding packets
 *         in a multi-hop fashion, but does not implement any routing
 *         protocol. A routing mechanism must be provided by the
 *         application or protocol running on top of the multihop
 *         module. In this case, this example program provides the
 *         routing mechanism.
 *
 *         The routing mechanism implemented by this example program
 *         is very simple: it forwards every incoming packet to a
 *         random neighbor. The program maintains a list of neighbors,
 *         which it populated through the use of the announcement
 *         mechanism.
 *
 *         The neighbor list is populated by incoming announcements
 *         from neighbors. The program maintains a list of neighbors,
 *         where each entry is allocated from a MEMB() (memory block
 *         pool). Each neighbor has a timeout so that they do not
 *         occupy their list entry for too long.
 *
 *         When a packet arrives to the node, the function forward()
 *         is called by the multihop layer. This function picks a
 *         random neighbor to send the packet to. The packet is
 *         forwarded by every node in the network until it reaches its
 *         final destination (or is discarded in transit due to a
 *         transmission error or a collision).
 *
 */

#include "contiki.h"
#include "net/rime.h"
#include "net/rime/multihop.h"
#include "lib/list.h"
#include <stdio.h>

#define CHANNEL 135


/*---------------------------------------------------------------------------*/
PROCESS(example_multihop_process, "multihop example");
AUTOSTART_PROCESSES(&example_multihop_process);
/*---------------------------------------------------------------------------*/
/*
 * This function is called to forward a packet. The function picks a
 * random neighbor from the neighbor list and returns its address. The
 * multihop layer sends the packet to this address. If no neighbor is
 * found, the function returns NULL to signal to the multihop layer
 * that the packet should be dropped.
 */
 rimeaddr_t *
forward(struct multihop_conn *c,
	const rimeaddr_t *originator, const rimeaddr_t *dest,
	const rimeaddr_t *prevhop, uint8_t hops)
{

		 printf(" %d.%d : Sending packet to %d.%d towards %d.%d hop %d \n", rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1], dest->u8[0], dest->u8[1],prevhop->u8[0], prevhop->u8[1], packetbuf_attr(PACKETBUF_ATTR_HOPS));

return dest;
}

/*
 * This function is called at the final recepient of the message.
 */
/* ------------------------------------------------------------------*/
static void
recv_uc(struct multihop_conn *c, const rimeaddr_t *sender,
     const rimeaddr_t *prevhop,
     uint8_t hops)
{
	
		rimeaddr_t final;	
		printf("multihop message received '%s' from %d.%d send by %d.%d \n", (char *)packetbuf_dataptr(),prevhop->u8[0],prevhop->u8[1],sender->u8[0], sender->u8[1]);
		//static const struct multihop_callbacks multihop_call = {recv_uc, forward};
		//static struct multihop_conn multihop2;

	

	}


static const struct multihop_callbacks multihop_call = {recv_uc, forward};
static struct multihop_conn multihop;

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_multihop_process, ev, data)
{
  //PROCESS_EXITHANDLER(multihop_close(&multihop);)
    
  PROCESS_BEGIN();

	static struct etimer et;

  /* Open a multihop connection on Rime channel CHANNEL. */
  multihop_open(&multihop, CHANNEL, &multihop_call);

  while(1) {
 
		rimeaddr_t destination;

    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 4);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

     /* Copy the "Hello" to the packet buffer. */
    packetbuf_copyfrom("Hello", 6);

    /* Set the Rime address of the final receiver of the packet to
       1.0. This is a value that happens to work nicely in a Cooja
       simulation (because the default simulation setup creates one
       node with address 1.0). */
    destination.u8[0] = 3;
    destination.u8[1] = 0;

    /* Send the packet. */
    multihop_send(&multihop, &destination);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

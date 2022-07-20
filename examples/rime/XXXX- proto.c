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
 *         Best-effort single-hop unicast example
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "sys/mt.h"
#include "net/rime/unicast.h"
#include "sys/process.h"
#include "dev/button-sensor.h"
#include "energest.h"
#include "dev/leds.h"

#include <stdio.h>
#define CHANNEL 150

/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Simulation");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/
static void
recv_uc2(struct unicast_conn *c, const rimeaddr_t *from)
{
	static int count_thread2 = 1;  
	printf("%d Thread : Final message %s received from %d.%d\n", count_thread2, 
	 (char *)packetbuf_dataptr(), from->u8[0], from->u8[1]);

	++count_thread2 ;
	
}
/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks unicast_callbacks2 = {recv_uc2};
static struct unicast_conn uc, uc2;
/*------------------------------------------------------------------------------*/
static void
recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
 
	static int count_thread = 1;
	 rimeaddr_t addr1, addr;

		addr1.u8[0]=9;
		addr1.u8[1]=0;

	printf("Message : %s received from %d.%d \n",
	 (char *)packetbuf_dataptr(), from->u8[0], from->u8[1]);

}
/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks unicast_callbacks = {recv_uc};   
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc);)
    
  PROCESS_BEGIN();

	static struct mt_thread thread;	

  static struct etimer et;

	static unsigned long rx_start_duration;
	
	rx_start_duration = energest_type_time(ENERGEST_TYPE_LISTEN);
	
  
 			mt_init();
			mt_start(&thread, unicast_open(&uc, 146, &unicast_callbacks), "");
			

  while(1) {

    rimeaddr_t addr, addr1;
  	etimer_set(&et, CLOCK_SECOND);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    packetbuf_copyfrom("Hello", 6);
   	
		unicast_open(&uc2, 147, &unicast_callbacks2);	
			
		printf("energy rx: %lu\n", energest_type_time(ENERGEST_TYPE_LISTEN) - rx_start_duration);

  }

			mt_stop(&thread);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

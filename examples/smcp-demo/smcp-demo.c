/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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

#include "smcp-task.h"
#include "net/resolv.h"
#if WEBSERVER
#include "webserver-nogui.h"
#endif

#include "watchdog.h"

PROCESS_NAME(smcp_demo);
PROCESS(smcp_demo, "SMCP Demo");

/*---------------------------------------------------------------------------*/
AUTOSTART_PROCESSES(
	&resolv_process,
	&smcp_task,
	&smcp_demo,
#if WEBSERVER
	&webserver_nogui_process,
#endif
	NULL
);
/*---------------------------------------------------------------------------*/

#include <smcp/smcp.h>
#include <smcp/smcp_node.h>
#include <smcp/smcp_timer_node.h>
#include <smcp/smcp_pairing.h>
#include <smcp/smcp_variable_node.h>
#include <smcp/url-helpers.h>
#include "lib/sensors.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifndef DISPLAY_MESSAGE
#if CONTIKI_TARGET_AVR_RAVEN
#define DISPLAY_MESSAGE(content,content_length)		raven_lcd_display_messagen(content,content_length)
#else
#define DISPLAY_MESSAGE(content,content_length)		printf("MSG: %s\n",content)
#endif
#endif

#if CONTIKI_TARGET_AVR_RAVEN
#include "raven-lcd.h"
#endif

smcp_status_t
action_beep_func(smcp_variable_node_t node,  char* content, size_t content_length,coap_content_type_t content_type) {
	PRINTF(" *** Received BEEP Action!\n");

#if CONTIKI_TARGET_AVR_RAVEN
	raven_lcd_beep();
#else
	printf("BEEP!\n\a");
#endif
	
	return 0;
}

smcp_status_t
action_msg_func(smcp_variable_node_t node,  char* content, size_t content_length,coap_content_type_t content_type) {
	PRINTF(" *** Received MSG Action! content_length = %d ",(int)content_length);
	PRINTF("content = \"%s\"\n",content);

	if(content_type==SMCP_CONTENT_TYPE_APPLICATION_FORM_URLENCODED) {
		char *key;
		char *value;
		
		// Look for the 'v' key, so we can display it's value
		while(url_form_next_value((char**)&content,&key,&value)) {
			//if(strcmp(key,"v")==0)
			if(key[0]=='v' && key[1]==0)
				break;
		}

		if(!value)
			value = "(null)";

		content = value;
		content_length = strlen(value);
	}
	
	DISPLAY_MESSAGE(content,content_length);
	return 0;
}

smcp_status_t
elapsed_time_get_func(smcp_variable_node_t node, char* content, size_t* content_length, coap_content_type_t* content_type) {
	int ret = 0;
	
	snprintf(content,*content_length,"v=%lu",clock_seconds());
	*content_length = strlen(content);
	*content_type = SMCP_CONTENT_TYPE_APPLICATION_FORM_URLENCODED;

	PRINTF("Uptime queried.\n");

	return ret;
}

smcp_variable_node_t
create_elapsed_time_variable_node(smcp_variable_node_t ret,smcp_node_t parent,const char* name) {
	ret = smcp_node_init_variable(ret,(void*)parent, name);

	ret->get_func = elapsed_time_get_func;
	
	return ret;
}

smcp_status_t
string_value_get_func(smcp_variable_node_t node, char* content, size_t* content_length, coap_content_type_t* content_type) {
	int ret = 0;
	
	if(*content_length<3) {
		ret = SMCP_STATUS_FAILURE;
		*content_length = 0;
		goto bail;
	}
	
	content[0] = 'v';
	content[1] = '=';
	url_encode_cstr(content+2,(const char*)node->node.context,*content_length-2);
		
	*content_length = strlen(content);
	*content_type = SMCP_CONTENT_TYPE_APPLICATION_FORM_URLENCODED;

	PRINTF("string_value_get_func queried.\n");
	

bail:
	return ret;
}

smcp_status_t
processes_value_get_func(smcp_variable_node_t node, char* content, size_t* content_length, coap_content_type_t* content_type) {
	int ret = 0;
	
	if(*content_length<3) {
		ret = SMCP_STATUS_FAILURE;
		*content_length = 0;
		goto bail;
	}
	
	struct process* iter = process_list;
	int size=0;
	
	while(iter) {
		size+=snprintf(content+size,*content_length-size,"%p, %u, %s\n",iter,iter->state,iter->name);
		iter = iter->next;
	}
	
	*content_length = size;
	*content_type = COAP_CONTENT_TYPE_TEXT_CSV;

bail:
	return ret;
}

smcp_status_t
etimer_list_get_func(smcp_variable_node_t node, char* content, size_t* content_length, coap_content_type_t* content_type) {
	int ret = 0;
	extern struct etimer *timerlist;
	
	if(*content_length<3) {
		ret = SMCP_STATUS_FAILURE;
		*content_length = 0;
		goto bail;
	}
	
	struct etimer* iter = timerlist;
	int size=0;
	
	while(iter) {
		size+=snprintf(content+size,*content_length-size,"%p, %u, %u, %s\n",iter,iter->timer.start,iter->timer.interval,iter->p->name);
		iter = iter->next;
	}
	
	*content_length = size;
	*content_type = COAP_CONTENT_TYPE_TEXT_CSV;

bail:
	return ret;
}

#if USE_CONTIKI_SENSOR_API
smcp_status_t
sensor_list_get_func(smcp_variable_node_t node, char* content, size_t* content_length, coap_content_type_t* content_type) {
	int ret = 0;
	
	if(*content_length<3) {
		ret = SMCP_STATUS_FAILURE;
		*content_length = 0;
		goto bail;
	}
	
	struct sensors_sensor* iter = sensors_first();
	int size=0;
	
	while(iter) {
		size+=snprintf(content+size,*content_length-size,"0x%04x, %s, %d\n",iter,iter->type,iter->value(0));
		iter = sensors_next(iter);
	}
	
	*content_length = size;
	*content_type = COAP_CONTENT_TYPE_TEXT_CSV;

bail:
	return ret;
}
smcp_variable_node_t create_sensor_list_variable_node(smcp_node_t parent,const char* name) {
	smcp_variable_node_t ret = NULL;

	ret = smcp_node_init_variable(NULL,(void*)parent, name);

	ret->get_func = sensor_list_get_func;
	
	return ret;
}
#endif //USE_CONTIKI_SENSOR_API

smcp_variable_node_t create_etimer_list_variable_node(smcp_node_t parent,const char* name) {
	smcp_variable_node_t ret = NULL;

	ret = smcp_node_init_variable(NULL,(void*)parent, name);

	ret->get_func = etimer_list_get_func;
	
	return ret;
}

smcp_variable_node_t create_process_list_variable_node(smcp_node_t parent,const char* name) {
	smcp_variable_node_t ret = NULL;

	ret = smcp_node_init_variable(NULL,(void*)parent, name);

	ret->get_func = processes_value_get_func;
	
	return ret;
}


smcp_variable_node_t create_temp_variable_node(smcp_variable_node_t ret,smcp_node_t parent,const char* name) {
#if CONTIKI_TARGET_AVR_RAVEN && WEBSERVER
	extern char sensor_temperature[12];
#else
	static const char* sensor_temperature = "N/A";
#endif

	ret = smcp_node_init_variable(ret,(void*)parent, name);

	ret->node.context = sensor_temperature;
	ret->get_func = string_value_get_func;
	
	return ret;
}

smcp_variable_node_t create_extvoltage_variable_node(smcp_variable_node_t ret,smcp_node_t parent,const char* name) {
#if CONTIKI_TARGET_AVR_RAVEN && WEBSERVER
	extern char sensor_extvoltage[12];
#else
	static const char* sensor_extvoltage = "N/A";
#endif

	ret = smcp_node_init_variable(ret,(void*)parent, name);

	ret->node.context = sensor_extvoltage;
	ret->get_func = string_value_get_func;

	return ret;
}

#if USE_RELAY
smcp_variable_node_t create_relay_variable_node(smcp_node_t parent,const char* name) {
	smcp_variable_node_t ret = NULL;
	smcp_action_node_t set_action = NULL;
	smcp_action_node_t on_action = NULL;
	smcp_action_node_t off_action = NULL;
	smcp_action_node_t toggle_action = NULL;

	ret = smcp_node_init_variable(NULL,(void*)parent, name);

	smcp_node_init_event(NULL,(void*)ret, "changed");
	set_action = smcp_node_init_action(NULL,(void*)ret, "set");
	on_action = smcp_node_init_action(NULL,(void*)ret, "set=1");
	off_action = smcp_node_init_action(NULL,(void*)ret, "set=0");
	toggle_action = smcp_node_init_action(NULL,(void*)ret, "toggle");

	//ret->get_func = string_value_get_func;
	
	return ret;
}
#endif


PROCESS_THREAD(smcp_demo, ev, data)
{
	static struct smcp_variable_node_s uptime_var;
	static struct smcp_variable_node_s temp_var;
	static struct smcp_variable_node_s extvoltage_var;
	
	PROCESS_BEGIN();
		
	{
		static struct smcp_variable_node_s node;
		smcp_node_init_variable(&node,smcp_daemon_get_root_node(smcp_daemon), "beep");
		node.post_func = &action_beep_func;
	}

	{
		static struct smcp_variable_node_s node;
		smcp_node_init_variable(&node,smcp_daemon_get_root_node(smcp_daemon), "reset");
		node.post_func = (void*)&watchdog_reboot;
	}

	{
		static struct smcp_variable_node_s node;
		smcp_node_init_variable(&node,smcp_daemon_get_root_node(smcp_daemon), "lcd-msg");
		node.post_func = &action_msg_func;
	}

	create_temp_variable_node(
		&temp_var,
		smcp_daemon_get_root_node(smcp_daemon),
		"temp"
	);

	create_extvoltage_variable_node(
		&extvoltage_var,
		smcp_daemon_get_root_node(smcp_daemon),
		"extvoltage"
	);

	create_elapsed_time_variable_node(
		&uptime_var,
		smcp_daemon_get_root_node(smcp_daemon),
		"uptime"
	);
	
	create_process_list_variable_node(
		smcp_daemon_get_root_node(smcp_daemon),
		"processes"
	);
	
	create_etimer_list_variable_node(
		smcp_daemon_get_root_node(smcp_daemon),
		"etimers"
	);
	
#if USE_CONTIKI_SENSOR_API
	create_sensor_list_variable_node(
		smcp_daemon_get_root_node(smcp_daemon),
		"sensors"
	);
#endif

	{
		static struct smcp_timer_node_s timer_node;
		smcp_timer_node_init(
			&timer_node,
			smcp_daemon,
			smcp_daemon_get_root_node(smcp_daemon),
			"timer01"
		);
	}
	
	do {
		PROCESS_WAIT_EVENT();

#if CONTIKI_TARGET_AVR_RAVEN
		if(ev == raven_lcd_updated_temp) {
			static char content[16] = "v=";
			size_t content_len = url_encode_cstr(content+2,(const char*)data,sizeof(content)-2)+2;
			
			if(content_len>=sizeof(content))
				content_len = sizeof(content)-1;
			
			PRINTF("Temp event (%s)\n",(const char*)content);

			smcp_daemon_trigger_event(
				smcp_daemon,
				"temp",
				content,
				content_len,
				SMCP_CONTENT_TYPE_APPLICATION_FORM_URLENCODED
			);
		} else if(ev == raven_lcd_updated_extvoltage) {
			char content[16] = "v=";
			size_t content_len = url_encode_cstr(content+2,(const char*)data,sizeof(content)-2)+2;
			
			if(content_len>=sizeof(content))
				content_len = sizeof(content)-1;

			PRINTF("Ext Voltage event (%s)\n",(const char*)content);
			
		} else if(ev == raven_lcd_updated_batvoltage) {
			char content[16] = "v=";
			size_t content_len = url_encode_cstr(content+2,(const char*)data,sizeof(content)-2)+2;
			
			if(content_len>sizeof(content))
				content_len = sizeof(content);

			PRINTF("Bat Voltage event (%s)\n",(const char*)content);
			
			smcp_daemon_trigger_event(
				smcp_daemon,
				"batvoltage",
				content,
				content_len,
				SMCP_CONTENT_TYPE_APPLICATION_FORM_URLENCODED
			);
		}
#endif // CONTIKI_TARGET_AVR_RAVEN

	} while(ev != PROCESS_EVENT_EXIT);

	PROCESS_END();
}


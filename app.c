/* standard library headers */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

/* BG stack headers */
#include "bg_types.h"
#include "gecko_bglib.h"

/* Own header */
#include "app.h"
//#include "dump.h"
#include "support.h"

// App booted flag
static bool appBooted = false;
static struct {
  bd_addr address;
  uint8 address_type;
  uint8 set, get, as_number, reset;
} config = { .address = { .addr = {0,0,0,0,0,0}},
	     .set = 0,
	     .get = 1,
	     .reset = 0,
	     .as_number = 0,
	     .address_type = le_gap_address_type_public,
};

void parse_address(const char *fmt,bd_addr *address) {
  char buf[3];
  int octet;
  for(uint8 i = 0; i < 6; i++) {
    memcpy(buf,&fmt[3*i],2);
    buf[2] = 0;
    sscanf(buf,"%02x",&octet);
    address->addr[5-i] = octet;
  }
}

const char *getAppOptions(void) {
  return "s<address>t<type>qnr";
}

void appOption(int option, const char *arg) {
  switch(option) {
  case 't':
    config.address_type = atoi(arg);
    break;
  case 'r':
    config.reset = 1;
    break;
  case 's':
    parse_address(arg,&config.address);
    config.set = 1;
    break;
  case 'q':
    config.get = 0;
    break;
  case 'n':
    config.as_number = 1;
    break;
  default:
    fprintf(stderr,"Unhandled option '-%c'\n",option);
    exit(1);
  }
}

void appInit(void) {
}

/***********************************************************************************************//**
 *  \brief  Event handler function.
 *  \param[in] evt Event pointer.
 **************************************************************************************************/
void appHandleEvents(struct gecko_cmd_packet *evt)
{
  if (NULL == evt) {
    return;
  }

  // Do not handle any events until system is booted up properly.
  if ((BGLIB_MSG_ID(evt->header) != gecko_evt_system_boot_id)
      && !appBooted) {
#if defined(DEBUG)
    printf("Event: 0x%04x\n", BGLIB_MSG_ID(evt->header));
#endif
    millisleep(50);
    return;
  }

  /* Handle events */
#ifdef DUMP
  switch (BGLIB_MSG_ID(evt->header)) {
  default:
    dump_event(evt);
  }
#endif
  switch (BGLIB_MSG_ID(evt->header)) {
  case gecko_evt_system_boot_id:
    appBooted = true;
    if(config.reset || config.set) {
      config.reset = config.set = 0;
      gecko_cmd_system_set_identity_address(config.address,config.address_type);
      gecko_cmd_system_reset(0);
      break;
    }
    if(config.get) {
      struct gecko_msg_system_get_bt_address_rsp_t *resp = gecko_cmd_system_get_bt_address();
      printf("BT_ADDRESS=%s",(config.as_number)?"0x":"");
      for(int i = 0; i < 6; i++) printf("%s%02x",(!config.as_number && i)?":":"",resp->address.addr[5-i]);
      printf("\n");
    }
    exit(0);
    break;
  default:
    break;
  }
}

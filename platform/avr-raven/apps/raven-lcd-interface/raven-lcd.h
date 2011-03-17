

#ifndef __RAVEN_LCD_H__
#define __RAVEN_LCD_H__

#include "contiki.h"

/** \name These are GUI to Radio Binary commands. */
/** \{ */
#define CMD_NOP  (0x00)
#define CMD_TEMP (0x80)
#define CMD_PING (0x81)
#define CMD_ADC2 (0x82)
#define CMD_BATTERY_VOLTAGE          (0x83)
#define CMD_STATUS_REQ               (0x84)
#define CMD_RELAY_ON                 (0x85)
#define CMD_RELAY_OFF                (0x86)
/** \} */

/** \name These are the Radio to GUI binary commands. */
/** \{ */
#define REPORT_PING                   (0xC0)
#define REPORT_PING_BEEP              (0xC1)
#define REPORT_TEXT_MSG               (0xC2)
#define REPORT_BEEP					(0xC3)
#define REPORT_STATUS					(0xC4)
#define REPORT_RELAY_ON				(0xC5)
#define REPORT_RELAY_OFF			(0xC6)
/** \} */

#define PING_REQUEST 0xc1
#define PING_REPLY   0xc0
#define SERIAL_CMD   0x1

int raven_lcd_serial_input(unsigned char ch);
PROCESS_NAME(raven_lcd_process);

void raven_lcd_display_message(const char* msg);
void raven_lcd_display_messagen(const char* msg,size_t len);
void raven_lcd_beep();

CCIF extern process_event_t raven_lcd_updated_temp;
CCIF extern process_event_t raven_lcd_updated_extvoltage;
CCIF extern process_event_t raven_lcd_updated_batvoltage;
CCIF extern process_event_t raven_lcd_updated_relay;

#endif
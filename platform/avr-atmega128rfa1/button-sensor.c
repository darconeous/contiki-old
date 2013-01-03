/* Dummy sensor routine */

#include "lib/sensors.h"
#include "dev/button-sensor.h"

static int status(int type);
static int value(int type);
static int configure(int type, int c);

SENSORS_SENSOR(button_sensor, BUTTON_SENSOR, value, configure, status);

static int
value(int type)
{
 return PORTF;
}

static int
configure(int type, int c)
{
	switch (type) {
	case SENSORS_ACTIVE:
		if (c) {
			if(!status(SENSORS_ACTIVE)) {
			}
		} else {
		}
		return 1;
	}
	return 0;
}

static int
status(int type)
{
	switch (type) {
	case SENSORS_ACTIVE:
	case SENSORS_READY:
		return 1;
	}
	return 0;
}


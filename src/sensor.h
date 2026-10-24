#ifndef APP_SENSOR_H
#define APP_SENSOR_H

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>


extern struct k_msgq temp_queue;

int sensor_init(void);

#endif
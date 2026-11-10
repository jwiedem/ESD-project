
#include "sensor.h"

K_MSGQ_DEFINE(temp_queue, sizeof(struct sensor_value), 4, 4);

static const struct device *const bmp280 = DEVICE_DT_GET_ANY(bosch_bme280);
static struct k_work_delayable sensor_work;

static void read_sensor(void)
{
	struct sensor_value temp;

	int err = sensor_sample_fetch(bmp280);
	if (err < 0) {
		printk("Sensor fetch failed: %d\n", err);
		return;
	}

	err = sensor_channel_get(bmp280, SENSOR_CHAN_AMBIENT_TEMP, &temp);
	if (err < 0) {
		printk("Temperature read failed: %d\n", err);
		return;
	}

	//printk("temp: %d.%02d C\n", temp.val1, temp.val2);

	err = k_msgq_put(&temp_queue, &temp, K_NO_WAIT);
	if (err != 0) {
		printk("Reading queue full; dropping reading\n");
	}
}

static void sensor_work_handler(struct k_work *work)
{
	ARG_UNUSED(work);
	read_sensor();
	k_work_reschedule(&sensor_work, K_SECONDS(2));
}

int sensor_init(void)
{
	if (bmp280 == NULL) {
		printk("No BMP280 device found\n");
		return -ENODEV;
	}

	if (!device_is_ready(bmp280)) {
		printk("BMP280 device is not ready\n");
		return -ENODEV;
	}

	k_work_init_delayable(&sensor_work, sensor_work_handler);
	k_work_schedule(&sensor_work, K_SECONDS(2));
	return 0;
}

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/services/nus.h>

#include <errno.h>
#include "button.h"
#include "sensor.h"
#include "oled.h"
#include "ble.h"


static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios),
};

int main(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		if (!gpio_is_ready_dt(&leds[i])) {
			printk("LED %u is not ready\n", (unsigned int)i);
			return 0;
		}

		int err = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
		if (err < 0) {
			printk("LED initialization failed: %d\n", err);
			return 0;
		}
	}

	int err = button_init();
	if (err < 0) {
		printk("Button initialization failed: %d\n", err);
		return 0;
	}

	err = sensor_init();
	if (err < 0) {
		printk("Sensor initialization failed: %d\n", err);
		return 0;
	}

	err = oled_init();
	if (err < 0) {
		printk("Oled initialization failed: %d\n", err);
		return 0;
	}

	err = ble_init();
	if (err < 0) {
		printk("BLE initialization failed: %d\n", err);
		return 0;
	}

	struct sensor_value temperature;
	char text[32];

	while(1) {
		err = k_msgq_get(&temp_queue, &temperature, K_FOREVER);
		if (err != 0) {
			printk("Queue receive failed: %d\n", err);
			continue;
		}

		int hundredths = 
			(int) (sensor_value_to_micro(&temperature) / 10000);
		
		const char *sign = hundredths < 0 ? "-" : "";
		int magnitude = hundredths < 0 ? -hundredths : hundredths;

		snprintf(text, sizeof(text), "%s%d.%02d C",
			 sign, magnitude / 100, magnitude % 100);

		err = write_oled(text);
		if (err < 0) {
			printk("OLED write failed: %d\n", err);
		}

		err = ble_send_text(text);
		if (err < 0 && err != -EAGAIN) {
			printk("BLE send failed: %d\n", err);
		}
	}

	k_sleep(K_FOREVER);
	return 0;
}

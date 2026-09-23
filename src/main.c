#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
#define SW0_NODE  DT_ALIAS(sw0)

static const struct gpio_dt_spec led =
	GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static const struct gpio_dt_spec button =
	GPIO_DT_SPEC_GET(SW0_NODE, gpios);

static struct gpio_callback button_cb_data;

void button_pressed(const struct device *dev,
		    struct gpio_callback *cb,
		    uint32_t pins)
{
	gpio_pin_toggle_dt(&led);
}

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	if (!gpio_is_ready_dt(&button)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(
		&button,
		GPIO_INT_EDGE_TO_ACTIVE
	);

	if (ret < 0) {
		return 0;
	}

	gpio_init_callback(
		&button_cb_data,
		button_pressed,
		BIT(button.pin)
	);

	gpio_add_callback(
		button.port,
		&button_cb_data
	);

	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include "button.h"

static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static struct gpio_callback button_cb_data;
static struct k_work_delayable button_work;

static void button_work_handler(struct k_work *work)
{
	ARG_UNUSED(work);
	gpio_pin_toggle_dt(&led1);
	printk("toggled\n");
}

static void button_isr(const struct device *port, struct gpio_callback *cb,
		       gpio_port_pins_t pins)
{
	ARG_UNUSED(port);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);
	k_work_reschedule(&button_work, K_MSEC(30));
}

int button_init(void)
{
	if (!gpio_is_ready_dt(&button0) || !gpio_is_ready_dt(&led1)) {
		return -ENODEV;
	}

	int err = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
	if (err < 0) {
		return err;
	}

	err = gpio_pin_configure_dt(&button0, GPIO_INPUT);
	if (err < 0) {
		return err;
	}

	k_work_init_delayable(&button_work, button_work_handler);
	gpio_init_callback(&button_cb_data, button_isr, BIT(button0.pin));
	err = gpio_add_callback(button0.port, &button_cb_data);
	if (err < 0) {
		return err;
	}

	err = gpio_pin_interrupt_configure_dt(&button0, GPIO_INT_EDGE_TO_ACTIVE);
	if (err < 0) {
		gpio_remove_callback(button0.port, &button_cb_data);
	}
	return err;
}

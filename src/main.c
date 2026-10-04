#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/printk.h>

#define LED0_NODE DT_ALIAS(led0)
#define SW0_NODE  DT_ALIAS(sw0)

static const struct gpio_dt_spec led =
	GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static const struct gpio_dt_spec button =
	GPIO_DT_SPEC_GET(SW0_NODE, gpios);

static struct k_work_delayable button_work;

static struct gpio_callback button_cb_data;

static uint8_t led_state = 0;

void button_pressed()
{
  k_work_reschedule(&button_work, K_MSEC(30));
}

void led_work_handler(struct k_work *work)
{
  led_state = !led_state;
	gpio_pin_set_dt(&led, led_state);

  printk("LED state: %d", led_state);
}

int main(void)
{
  k_work_init_delayable(&button_work, led_work_handler);

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
    printk("LED state: %d\n", led_state);
		k_sleep(K_MSEC(1000));
	}

	return 0;
}

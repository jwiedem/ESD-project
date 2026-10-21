#include <stdio.h>                
#include <zephyr/kernel.h>         
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>


#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

#define BUTTON0_NODE DT_ALIAS(sw0)

static struct sensor_value temp;

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec leds[] = {led0, led1, led2, led3};

static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(BUTTON0_NODE, gpios);


static const struct device *const bmp280 = DEVICE_DT_GET_ANY(bosch_bme280);


static struct gpio_callback button_cb_data;
static struct k_work_delayable button_work;
static struct k_work_delayable sensor_work;

static void read_sensor(void)
{
	if(sensor_sample_fetch(bmp280)<0)
		{
			printk("fetched failed");
			return;
		}
	sensor_channel_get(bmp280, SENSOR_CHAN_AMBIENT_TEMP, &temp);
	printk("temp: %d.%06d C\n", temp.val1, temp.val2);
}

static void button_work_handler(struct k_work *work)
{
	gpio_pin_toggle_dt(&leds[1]);
    printk("toggled\n");
}

static void sensor_work_handler(struct k_work *work)
{
	read_sensor();
	k_work_reschedule(&sensor_work, K_SECONDS(2));
}


static void button_isr(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pipe_flags)
{
	 k_work_reschedule(&button_work, K_MSEC(30));
}



int main(void)
{
	for (int i=0; i<4; i++)
	{
		if(!gpio_is_ready_dt(&leds[i]))
		{
			printf("error");
			return 0;
		}
	

	gpio_pin_configure_dt(&leds[i],GPIO_OUTPUT_INACTIVE);
	}

	if(!gpio_is_ready_dt(&button0))
	{
		printf("error");
		return 0;
	}

	gpio_pin_configure_dt(&button0, GPIO_INPUT);

	bool pressed = gpio_pin_get_dt(&button0);
	bool last_pressed = pressed;
	k_work_init_delayable (&button_work, button_work_handler);
	gpio_pin_interrupt_configure_dt(&button0, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(&button_cb_data, button_isr, BIT(button0.pin));
	gpio_add_callback(button0.port,&button_cb_data);
	k_work_init_delayable(&sensor_work, sensor_work_handler);
	k_work_schedule(&sensor_work, K_SECONDS(2)); 
	if (bmp280 == NULL)
	{
		printk("No bmp280");
		return 0;
	}

	if(!device_is_ready(bmp280))
	{
		printk("errror");
		return 0;
	}



	while(1)
	{
		// for(int i=0; i<ARRAY_SIZE(leds); i++)
		// {
		// 	gpio_pin_toggle_dt(&leds[i]);
		// 	k_msleep(150);
		// 	gpio_pin_toggle_dt(&leds[i]);
		// }

		k_sleep(K_FOREVER);
		
		
	}
	return 0;                                 
}
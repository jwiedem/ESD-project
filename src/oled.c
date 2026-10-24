#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <zephyr/sys/printk.h>
#include <stdio.h>


#include "oled.h"

static const struct device *const display =
	DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static bool initialized;

int write_oled(const char *text)
{
	if (!initialized) {
		return -ENODEV;
	}

	int ret = cfb_framebuffer_clear(display, false);
	if (ret < 0) {
		return ret;
	}

	ret = cfb_print(display, text, 0, 0);
	if (ret < 0) {
		return ret;
	}

	/* Drawing changes the RAM buffer; finalize transfers it to the OLED. */
	return cfb_framebuffer_finalize(display);
}

int oled_init(void)
{
	if (initialized) {
		return 0;
	}

	if (!device_is_ready(display)) {
		printk("OLED is not ready; check power, wiring and I2C address\n");
		return -ENODEV;
	}

	int ret = cfb_framebuffer_init(display);
	if (ret < 0) {
		printk("CFB initialization failed: %d\n", ret);
		return ret;
	}

	ret = cfb_framebuffer_set_font(display, 0);
	if (ret < 0) {
		printk("Font selection failed: %d\n", ret);
		goto cleanup;
	}

	ret = cfb_framebuffer_clear(display, true);
	if (ret < 0) {
		goto cleanup;
	}

	ret = display_blanking_off(display);
	if (ret < 0) {
		printk("Turning display on failed: %d\n", ret);
		goto cleanup;
	}

	initialized = true;
	return 0;

cleanup:
	cfb_framebuffer_deinit(display);
	return ret;
}

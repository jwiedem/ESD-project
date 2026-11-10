#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/services/nus.h>
#include <zephyr/bluetooth/conn.h>

#include <errno.h>
#include <string.h>
#include <zephyr/sys/atomic.h>
#include "ble.h"

static const struct bt_data advertising_data[] = {
    // Generally discoverable, bluetooth classic unsupported
	BT_DATA_BYTES(BT_DATA_FLAGS,
		      BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_NAME_COMPLETE,
		CONFIG_BT_DEVICE_NAME,
		sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static const struct bt_data scan_response[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

static atomic_t notifications_enabled;
static atomic_t advertising_requested;

static int start_advertising(void)
{
	int err = bt_le_adv_start(
		BT_LE_ADV_CONN_FAST_2,
		advertising_data, ARRAY_SIZE(advertising_data),
		scan_response, ARRAY_SIZE(scan_response));

	if (err == -EALREADY) {
		return 0;
	}
	if (err < 0) {
		printk("Advertising failed: %d\n", err);
		return err;
	}
	printk("BLE advertising as %s\n", CONFIG_BT_DEVICE_NAME);
	return 0;
}

static void advertising_work_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(advertising_work, advertising_work_handler);

static void advertising_work_handler(struct k_work *work)
{
	ARG_UNUSED(work);
	if (!atomic_get(&advertising_requested)) {
		return;
	}
	if (start_advertising() < 0) {
		k_work_reschedule(&advertising_work, K_SECONDS(1));
	}
}

static void recycled(void)
{
	/* The old connection slot is now free; restart from thread context. */
	if (atomic_get(&advertising_requested)) {
		k_work_reschedule(&advertising_work, K_NO_WAIT);
	}
}

static void send_enabled(enum bt_nus_send_status status)
{
	bool enabled = (status == BT_NUS_SEND_STATUS_ENABLED);

	atomic_set(&notifications_enabled, enabled);
	printk("BLE notifications %s\n", enabled ? "enabled" : "disabled");
}

static struct bt_nus_cb nus_callbacks = {
	.send_enabled = send_enabled,
};

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	ARG_UNUSED(conn);

	atomic_clear(&notifications_enabled);
	atomic_set(&advertising_requested, 1);
	printk("Disconnected: %u\n", reason);
}

static void connected(struct bt_conn *conn, uint8_t err)
{
	ARG_UNUSED(conn);

	if (err != 0) {
		printk("Connection failed: %u\n", err);
		return;
	}

	atomic_clear(&advertising_requested);
	printk("Connected\n");
}

BT_CONN_CB_DEFINE(connection_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
	.recycled = recycled,
};

int ble_init(void)
{
	int err = bt_enable(NULL);
	if (err < 0) {
		printk("Bluetooth initialization failed: %d\n", err);
		return err;
	}

	err = bt_nus_init(&nus_callbacks);
	if (err < 0) {
		printk("NUS initialization failed: %d\n", err);
		return err;
	}

	atomic_set(&advertising_requested, 1);
	return start_advertising();
}

int ble_send_text(const char *text)
{
	if (text == NULL) {
		return -EINVAL;
	}

	if (!atomic_get(&notifications_enabled)) {
		return -EAGAIN;
	}

	size_t length = strlen(text);

	/* Fits the default BLE notification payload. */
	if (length > 20) {
		return -EMSGSIZE;
	}

	return bt_nus_send(NULL, (const uint8_t *)text,
			   (uint16_t)length);
}

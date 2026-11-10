#ifndef APP_BLE_H
#define APP_BLE_H

/* Initialize Bluetooth, NUS, and connectable advertising. */
int ble_init(void);
int ble_send_text(const char *text);

#endif
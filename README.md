# Project scope

We plan to implement a simple  a temperature meter, that measures and displays the current temperature to a display. We plan to use the nRF52833, running Zephyr, and temperature sensor that have been provided to us, using a simple LCD or OLED display. If the project scope allows for it, we could expand
this project to send this data to another device to make the values readable remotely.


# Session 1:

We started our journey of working on the nRF52833 by loading one of the sample projects for the board blinky. We went through the code to understand how it and the device trees worked.
Once we understood how the code interacted with the device tree specifications we made our first changes to the code:
First we made the LED toggleable with Button 1. Since the code checked the button and toggled the LED every loop with a minimal delay of 100ms, when the button was pressed too long, the LED would start blinking on and off. To prevent this, we used interrupts to call the toggle function only once on GPIO_INT_EDGE_TO_ACTIVE, so it only triggers when the button is initially set to active.


# Session 2:

In this session, our aim was to display information to the computer over uart, because our next step with the temperature/humidity sensor would require that to be able to read the values. Our first step was to simply add a printk() to our interrupt with the value of LED, using "gpio_pin_get_dt(&led)", but for some reason we haven't figured out yet, it only prints "1" and never "0". To remedy this we set a global variable led_state and instead toggled it in the interrupt using gpio_pin_set_dt(). The text printed to the screen was frequently cut off, not fully printing. Sometimes it would not print on button press when we toggled the LED state. We thought this could be due to a debouncing issue and added a delay through the zephyr k_work_init_delayable() and then with k_work_reschedule with a delay of 30ms. This still didn't full fix our issue and we will investigate this further, but in any case our information was being sent to the screen, so our goal was met.
We read the serial interface on a laptop using "sudo screen /dev/tty/ACM0 115200"


# Session 3:

The first thing that we learned is that a sensor only exists in the program if three separate things agree: a devicetree node with the right compatible, the Kconfig symbol that compiles the driver in, and the code that references the device. We wired the bmp280 temp sensor to the standard I2C pins: SDA -> P0.26, SCL -> P0.27.
Our first build printed "No bmp280". The cause was that the overlay was in the wrong place: we had put it in a boards/nrf52833dk/ sub-folder, but Zephyr only auto-applies a flat file named after the board target, i.e. boards/nrf52833dk_nrf52833.overlay. We also learned that the same "bosch,bme280" compatible and driver serve both the BME280 and the BMP280.
We then moved the reading out of the main loop and made it a periodic task, and used k_work_delayable to run the read on the system workqueue every 2 seconds, rescheduling itself at the end of its handler. We managed to plot the temp reading over the serial console, which was the goal for this session.


# Session 4:

In this session our goal was to display the sensor values onto an OLED display. We looked up our specific OLED device, and found the respective zephyr driver to use, specifically the ssd1306 128x64 OLED display. Next we wanted to set up a second i2c line, namely i2c1. We connected the OLED's SDA to P0.30 and SCL to P0.31 as per the technical documentation. In order to enable i2c1 in the devicetree, we needed to disable the spi1 interface, as they share the same pins in the standard devicetree configuration for the board. We used zephyr's internal character frame buffer library to initialize the display and write to it.
We also refactored the code and adapted it to use a **queue** to put the sensor values into the temperature queue and then writing it from the queue to the display once the data has been received. This occurs at an interval of every 2 seconds.


# Session 5:

Next we implemented Bluetooth transmission for our project. We first enabled Bluetooth in the project config, defining the devices name as "TempSensor" and enabling Nordic UART Service (NUS). We created a new file for the Bluetooth code, in which we utilize NUS to send data via Bluetooth while emulating a UART connection. We chose NUS, since the nRF board we are using provides this service and enables us to read the data from a mobile device, using the nRF connect app, during testing and implementation. NUS keeps the implementation simple and uses UART, which is familiar from the labs and lectures. The device advertises itself via Bluetooth until a connection to the receiver is established, then waits until the receiver subscribes to the notifications. Once the notifications on NUS TX are enabled by the receiver the device starts sending it's data. This data can then be read from a mobile device using the app or any other configured Bluetooth receiver. We then used an ESP32 to read the values via Bluetooth and displayed them to a console via serial wire. If the connection is interrupted the board starts advertising itself over Bluetooth again until a connection is reestablished.


# Conclusion:

The initial scope for our project was fulfilled when displaying the temperature data to the OLED, however as a part of our learning journey we decided to implement Bluetooth aswell. We managed to send the data and read it from a phone using nRF connect, to make the project more complete we also used agentic coding for the ESP32 code for receiving the Bluetooth values to extend the project.

This project helped us understand the real time operating systems in general and specifically Zephyr. We learned how to work with device trees and how to initialize and configure hardware using it. We worked with device drivers for our peripherals and communication protocols such as I²C, UART and Bluetooth for sending and receiving data. Getting started with the project we solidified our understanding of interrupt service routines and implemented debouncing before moving on to sensor data, displays and scheduling work with work handlers and queues.

To make this project a fully realized embedded product, we would attach the screen to the receiving device, not the transmitter and have it display the received data. Instead of NUS we would consider standard BLE to save energy. The nRF and ESP boards was good for prototyping but would be much too bulky for a final product, which would require custom made PCBs.

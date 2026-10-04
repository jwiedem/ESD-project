# Humble beginnings
# Session 1:
We started our journey of working on the nRF52833 by loading one of the sample projects for the board blinky. We went through the code to understand how it and the device trees worked.
Once we understood how the code interacted with the device tree specifications we made our first changes to the code:
First we made the LED toggleable with Button 1. Since the code checked the button and toggled the LED every loop with a minimal delay of 100ms, when the button was pressed too long, the LED would start blinking on and off. To prevent this, we used interrupts to call the toggle function only once on GPIO_INT_EDGE_TO_ACTIVE, so it only triggers when the button is initially set to active.

# Session 2:
In this session, our aim was to display information to the computer over uart, because our next step with the temperature/humidity sensor would require that to be able to read the values. Our first step was to simply add a printk() to our interrupt with the value of LED, using "gpio_pin_get_dt(&led)", but for some reason we haven't figured out yet, it only prints "1" and never "0". To remedy this we set a global variable led_state and instead toggled it in the interrupt using gpio_pin_set_dt(). The text printed to the screen was frequently cut off, not fully printing. Sometimes it would not print on button press when we toggled the LED state. We thought this could be due to a debouncing issue and added a delay through the zephyr k_work_init_delayable() and then with k_work_reschedule with a delay of 30ms. This still didn't full fix our issue and we will investigate this further, but in any case our information was being sent to the screen, so our goal was met.
We read the serial interface on a laptop using "sudo screen /dev/tty/ACM0 115200"

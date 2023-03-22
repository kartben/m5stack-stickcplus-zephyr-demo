#include <zephyr.h>

#include <device.h>

#include <drivers/gpio.h>

#include <sys/printk.h>

#include <string.h>

#define LED_PORT DT_ALIAS_LED0_GPIOS_CONTROLLER

#define LED_PIN  DT_ALIAS_LED0_GPIOS_PIN

#define LED_FLAGS DT_ALIAS_LED0_GPIOS_FLAGS

void vulnerable_function(const char *input) {

    char buffer[10];

    strcpy(buffer, input);

    printk("Buffer: %s\n", buffer);

}

void main(void)

{

    int ret;

    struct device *dev = device_get_binding(LED_PORT);

    if (!dev) {

        printk("Cannot find %s!\n", LED_PORT);

        return;

    }

    ret = gpio_pin_configure(dev, LED_PIN, GPIO_OUTPUT_ACTIVE | LED_FLAGS);

    if (ret < 0) {

        printk("Failed to configure pin %d '%s': %d\n", LED_PIN, LED_PORT, ret);

        return;

    }

    while (1) {

        gpio_pin_toggle(dev, LED_PIN);

        k_sleep(K_MSEC(1000));

        printk("Hello, World! from %s\n", CONFIG_BOARD);

        vulnerable_function("This input string is too long and will cause a buffer overflow.");

        k_sleep(K_MSEC(1000));

    }

}


	

#ifndef HW_CONFIG_H_
#define HW_CONFIG_H_

#define SDA_PIN 20
#define SCL_PIN 21
#define PICO_I2C i2c0
#define I2C_SPEED 100 * 1000

#define PILOT_RDY   2
#define PILOT_CRG   3
#define PILOT_VNT   4
#define PILOT_PWM   5

#define RELAY_A     6
#define RELAY_B     7

#define LED_1       13
#define LED_2       14
#define LED_3       15

#endif
/*****************************************************************//**
 * @file main_vanilla_test.cpp
 *
 * @brief Basic test of 4 basic i/o cores
 *
 * @author p chu
 * @version v1.0: initial release
 *********************************************************************/

//#define _DEBUG
#include "chu_init.h"
#include "gpio_cores.h"
#include "xadc_core.h"
#include "sseg_core.h"

/**
 * blink once per second for 5 times.
 * provide a sanity check for timer (based on SYS_CLK_FREQ)
 * @param led_p pointer to led instance
 */
void timer_check(GpoCore *led_p) {
   int i;

   for (i = 0; i < 5; i++) {
      led_p->write(0xffff);
      sleep_ms(500);
      led_p->write(0x0000);
      sleep_ms(500);
      debug("timer check - (loop #)/now: ", i, now_ms());
   }
}

/**
 * check individual led
 * @param led_p pointer to led instance
 * @param n number of led
 */
void led_check(GpoCore *led_p, int n) {
   int i;

   for (i = 0; i < n; i++) {
      led_p->write(1, i);
      sleep_ms(200);
      led_p->write(0, i);
      sleep_ms(200);
   }
}

/**
 * leds flash according to switch positions.
 * @param led_p pointer to led instance
 * @param sw_p pointer to switch instance
 */
void sw_check(GpoCore *led_p, GpiCore *sw_p) {
   int i, s;

   s = sw_p->read();
   for (i = 0; i < 30; i++) {
      led_p->write(s);
      sleep_ms(50);
      led_p->write(0);
      sleep_ms(50);
   }
}

/**
 * uart transmits test line.
 * @note uart instance is declared as global variable in chu_io_basic.h
 */
void uart_check() {
   static int loop = 0;

   uart.disp("uart test #");
   uart.disp(loop);
   uart.disp("\n\r");
   loop++;
}

void sseg_check(SsegCore *sseg_p) {
   int i, n;
   uint8_t dp;

   //turn off led
   for (i = 0; i < 8; i++) {
      sseg_p->write_1ptn(0xff, i);
   }
   //turn off all decimal points
   sseg_p->set_dp(0x00);

   // display 0x0 to 0xf in 4 epochs
   // upper 4  digits mirror the lower 4
   for (n = 0; n < 4; n++) {
      for (i = 0; i < 4; i++) {
         sseg_p->write_1ptn(sseg_p->h2s(i + n * 4), 3 - i);
         sseg_p->write_1ptn(sseg_p->h2s(i + n * 4), 7 - i);
         sleep_ms(300);
      } // for i
   }  // for n
      // shift a decimal point 4 times
   for (i = 0; i < 4; i++) {
      bit_set(dp, 3 - i);
      sseg_p->set_dp(1 << (3 - i));
      sleep_ms(300);
   }
   //turn off led
   for (i = 0; i < 8; i++) {
      sseg_p->write_1ptn(0xff, i);
   }
   //turn off all decimal points
   sseg_p->set_dp(0x00);

}

void pwm_3color_led_check(PwmCore *pwm_p) {
   int i, n;
   double bright, duty;
   const double P20 = 1.2589;  // P20=100^(1/20); i.e., P20^20=100

   pwm_p->set_freq(50);
   for (n = 0; n < 3; n++) {
      bright = 1.0;
      for (i = 0; i < 20; i++) {
         bright = bright * P20;
         duty = bright / 100.0;
         pwm_p->set_duty(duty, n);
         pwm_p->set_duty(duty, n + 3);
         sleep_ms(100);
      }
      sleep_ms(300);
      pwm_p->set_duty(0.0, n);
      pwm_p->set_duty(0.0, n + 3);
   }
}

// instantiate switch, led
GpoCore led(get_slot_addr(BRIDGE_BASE, S2_LED));
GpiCore sw(get_slot_addr(BRIDGE_BASE, S3_SW));

XadcCore vcc(get_slot_addr(BRIDGE_BASE, S5_XDAC));
XadcCore temp(get_slot_addr(BRIDGE_BASE, S5_XDAC));
SsegCore seg(get_slot_addr(BRIDGE_BASE, S8_SSEG));
PwmCore rgb(get_slot_addr(BRIDGE_BASE, S6_PWM));

void voltageLED(XadcCore *vcc, GpoCore *led_p) {

	double voltage;
	voltage = vcc -> read_fpga_vcc();

	double num = voltage / 3.3 * 16;

	for (int i = 0; i < num - 1; i++) {
		led_p->write(1, i);
		sleep_ms(60);
	}
}

void errorLED(GpoCore *led_p) {

	for (int i = 0; i < 16; i++) {
		led_p -> write(0, i);
	}
}

void tempLED(XadcCore *temp, GpoCore *led_p, bool F) {
	double temperature;
	temperature = temp -> read_fpga_temp();
	temperature /= 10;

	if (F) {
		temperature = temperature * 1.8 + 32;
	}

	double num = temperature / 100.0 * 16;

	for (int i = 0; i < num - 1; i++) {
		led_p->write(1, i);
		sleep_ms(60);
	}
}

void writeVCC(XadcCore *vcc, SsegCore *seg) {
	double voltage;
	voltage = vcc -> read_fpga_vcc();
	voltage *= 1000;

	for (int i = 0; i < 8; i++) {
	  seg->write_1ptn(0xff, i);
    }

	seg -> set_dp(0x00);

	uint8_t d1 = voltage / 1000;
	uint8_t d2 =(voltage - d1 * 1000) / 100;
	uint8_t d3 = (voltage - d1 * 1000 - d2 * 100) / 10;
	uint8_t d4 = (voltage - d1 * 1000 - d2 * 100 - d3 * 10);

	uart.disp(d3);
	uart.disp("\n\r");

	seg -> write_1ptn(seg -> h2s(d1), 3);
	seg -> write_1ptn(seg -> h2s(d2), 2);
	seg -> write_1ptn(seg -> h2s(d3), 1);
	seg -> write_1ptn(seg -> h2s(d4), 0);
	seg -> set_dp(8);
	sleep_ms(50);

}

void writeTemp(XadcCore *temp, SsegCore *seg, bool F) {

	double temperature;
	temperature = temp -> read_fpga_temp();
	temperature /= 10;

	if (F) {
		temperature = temperature * 1.8 + 32;
	}

	for (int i = 0; i < 8; i++) {
	  seg->write_1ptn(0xff, i);
    }

	seg -> set_dp(0x00);

	temperature *= 100;
	uint8_t d1 = temperature / 1000;
	uint8_t d2 =(temperature - d1 * 1000) / 100;
	uint8_t d3 = (temperature - d1 * 1000 - d2 * 100) / 10;
	uint8_t d4 = (temperature - d1 * 1000 - d2 * 100 - d3 * 10);

	seg -> write_1ptn(seg -> h2s(d1), 3);
	seg -> write_1ptn(seg -> h2s(d2), 2);
	seg -> write_1ptn(seg -> h2s(d3), 1);
	seg -> write_1ptn(seg -> h2s(d4), 0);
	seg -> set_dp(4);
	sleep_ms(50);

}

void writeZero(SsegCore *seg) {
	for (int i = 0; i < 8; i++) {
	  seg->write_1ptn(0xff, i);
	}

	seg -> set_dp(0x00);
}

void colorRG(PwmCore *rgb, GpiCore *sw) {

	double bright1 = 1.0;
	double bright2 = 1.0;
	double duty1;
	double duty2;
	double P = 1.2589;
	double percent = 1.0;

	const int length = 4 * 20;

	int blue = 0;
	int green = 1;
	int red = 2;

	bool on;
	rgb -> set_freq(50);

	rgb -> set_duty(0, green);
	rgb -> set_duty(0, green + 3);
	rgb -> set_duty(0, red);
	rgb -> set_duty(0, red + 3);
	rgb -> set_duty(0, blue);
	rgb -> set_duty(0, blue + 3);

	for (int i = 0; i < length; i++) {
		on = sw -> read(0);

		if (!on) {
			rgb -> set_duty(0, red);
			rgb -> set_duty(0, red + 3);
			rgb -> set_duty(0, green);
			rgb -> set_duty(0, green + 3);
			rgb -> set_duty(0, blue);
			rgb -> set_duty(0, blue + 3);
			break;
		}

		if (i <= 20) {
			rgb -> set_duty(0.5 * 1.0, green);
			rgb -> set_duty(0.5 * 1.0, green + 3);

			rgb -> set_duty(0.0, red);
			rgb -> set_duty(0.0, red + 3);

			sleep_ms(5);
		}
		else if (i <= 40) {
			bright1 *= P;
			duty1 = bright1 / 100.0;

			bright2 /= P;
			duty2 = bright2;

			rgb -> set_duty(percent * duty1, red);
			rgb -> set_duty(percent * duty1, red + 3);
			rgb -> set_duty(percent * duty2, green);
			rgb -> set_duty(percent * duty2, green + 3);

			sleep_ms(100);
		}
		else if (i <= 60) {
			rgb -> set_duty(0.0, green);
			rgb -> set_duty(0.0, green + 3);

			rgb -> set_duty(0.5 * 1.0, red);
			rgb -> set_duty(0.5 * 1.0, red + 3);

			sleep_ms(5);
		}
		else if (i <=80) {
			bright1 /= P;
			duty1 = bright1 / 100.0;

			bright2 *= P;
			duty2 = bright2;

			rgb -> set_duty(percent * duty2, green);
			rgb -> set_duty(percent * duty2, green + 3);
			rgb -> set_duty(percent * duty1, red);
			rgb -> set_duty(percent * duty1, red + 3);

			sleep_ms(100);
		}
	}
}

void colorGB(PwmCore *rgb, GpiCore *sw) {
	double bright1 = 1.0;
	double bright2 = 1.0;
	double duty1;
	double duty2;
	double P = 1.2589;
	double percent = 1.0;

	const int length = 4 * 20;

	int blue = 0;
	int green = 1;
	int red = 2;

	bool on;
	rgb -> set_freq(50);

	rgb -> set_duty(0, green);
	rgb -> set_duty(0, green + 3);
	rgb -> set_duty(0, red);
	rgb -> set_duty(0, red + 3);
	rgb -> set_duty(0, blue);
	rgb -> set_duty(0, blue + 3);

	for (int i = 0; i < length; i++) {
		on = sw -> read(1);

		if (!on) {
			rgb -> set_duty(0, green);
			rgb -> set_duty(0, green + 3);
			rgb -> set_duty(0, blue);
			rgb -> set_duty(0, blue + 3);
			rgb -> set_duty(0, red);
			rgb -> set_duty(0, red + 3);
			break;
		}

		if (i <= 20) {
			rgb -> set_duty(percent * 1.0, blue);
			rgb -> set_duty(percent * 1.0, blue + 3);

			rgb -> set_duty(0.0, green);
			rgb -> set_duty(0.0, green + 3);

			sleep_ms(5);
		}
		else if (i <= 40) {
			bright1 *= P;
			duty1 = bright1 / 100.0;

			bright2 /= P;
			duty2 = bright2;

			rgb -> set_duty(percent * duty1, green);
			rgb -> set_duty(percent * duty1, green + 3);
			rgb -> set_duty(percent * duty2, blue);
			rgb -> set_duty(percent * duty2, blue + 3);

			sleep_ms(100);
		}
		else if (i <= 60) {
			rgb -> set_duty(0.0, blue);
			rgb -> set_duty(0.0, blue + 3);

			rgb -> set_duty(percent * 1.0, green);
			rgb -> set_duty(percent * 1.0, green + 3);

			sleep_ms(5);
		}
		else if (i <=80) {
			bright1 /= P;
			duty1 = bright1 / 100.0;

			bright2 *= P;
			duty2 = bright2;

			rgb -> set_duty(percent * duty2, blue);
			rgb -> set_duty(percent * duty2, blue + 3);
			rgb -> set_duty(percent * duty1, green);
			rgb -> set_duty(percent * duty1, green + 3);

			sleep_ms(100);
		}
	}
}

void colorBR(PwmCore *rgb, GpiCore *sw) {
	double bright1 = 1.0;
	double bright2 = 1.0;
	double duty1;
	double duty2;
	double P = 1.2589;
	double percent = 1.0;

	const int length = 4 * 20;

	int blue = 0;
	int green = 1;
	int red = 2;

	bool on;
	rgb -> set_freq(50);

	rgb -> set_duty(0, green);
	rgb -> set_duty(0, green + 3);
	rgb -> set_duty(0, red);
	rgb -> set_duty(0, red + 3);
	rgb -> set_duty(0, blue);
	rgb -> set_duty(0, blue + 3);

	for (int i = 0; i < length; i++) {
		on = sw -> read(2);

		if (!on) {
			rgb -> set_duty(0, green);
			rgb -> set_duty(0, green + 3);
			rgb -> set_duty(0, blue);
			rgb -> set_duty(0, blue + 3);
			rgb -> set_duty(0, red);
			rgb -> set_duty(0, red + 3);
			break;
		}

		if (i <= 20) {
			rgb -> set_duty(percent * 1.0, red);
			rgb -> set_duty(percent * 1.0, red + 3);

			rgb -> set_duty(0.0, blue);
			rgb -> set_duty(0.0, blue + 3);

			sleep_ms(5);
		}
		else if (i <= 40) {
			bright1 *= P;
			duty1 = bright1 / 100.0;

			bright2 /= P;
			duty2 = bright2;

			rgb -> set_duty(percent * duty1, blue);
			rgb -> set_duty(percent * duty1, blue + 3);
			rgb -> set_duty(percent * duty2, red);
			rgb -> set_duty(percent * duty2, red + 3);

			sleep_ms(100);
		}
		else if (i <= 60) {
			rgb -> set_duty(0.0, red);
			rgb -> set_duty(0.0, red + 3);

			rgb -> set_duty(percent * 1.0, blue);
			rgb -> set_duty(percent * 1.0, blue + 3);

			sleep_ms(5);
		}
		else if (i <=80) {
			bright1 /= P;
			duty1 = bright1 / 100.0;

			bright2 *= P;
			duty2 = bright2;

			rgb -> set_duty(percent * duty2, red);
			rgb -> set_duty(percent * duty2, red + 3);
			rgb -> set_duty(percent * duty1, blue);
			rgb -> set_duty(percent * duty1, blue + 3);

			sleep_ms(100);
		}
	}
}


int main() {
   while (1) {

	  bool switch0 = sw.read(0);
	  bool switch1 = sw.read(1);
	  bool switch2 = sw.read(2);

	  bool isFahr = 0;

      if (switch0 && !switch1 && !switch2) {
    	  writeVCC(&vcc, &seg);
    	  voltageLED(&vcc, &led);
    	  colorRG(&rgb, &sw);
      }
      else if (!switch0 && switch1 && !switch2) {
    	  isFahr = 0;
    	  writeTemp(&temp, &seg, isFahr);
    	  tempLED(&temp, &led, isFahr);
    	  colorGB(&rgb, &sw);
      }
      else if (!switch0 && !switch1 && switch2) {
		  isFahr = 1;
    	  writeTemp(&temp, &seg, isFahr);
		  tempLED(&temp, &led, isFahr);
		  colorBR(&rgb, &sw);
		}
      else {
    	  errorLED(&led);
    	  writeZero(&seg);
      }
   }
}

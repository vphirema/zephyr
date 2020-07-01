/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>

#include <kernel.h>
#include <string.h>

#include <drivers/espi.h>

#include "pow_seq.h"

K_THREAD_STACK_DEFINE(power_seq_thread_stack, POW_SEQ_THREAD_STACK_SIZE);
static struct k_thread power_seq_thread_id;

static struct gpio_callback power_seq_cb_data;

#define ESPI_FREQ_MHZ	20
static const struct device *espi_dev;
static struct espi_callback espi_bus_cb;
static struct espi_callback espi_chan_cb;
static struct espi_callback espi_vw_cb;

#ifdef TGLRVP
/* TODO: Add config instead of assignment */
struct power_signals power_sig_list[] = {
	[POWER_VR_EC_DSW_PWROK] = {
		.sig_name = "VR_EC_DSW_PWROK",
		.sig_dev = NULL,
		.pin = 11, /* MCHP_GPIO_013 */
		.sig_bind = "GPIO000_036",
		.flags = GPIO_INT_MODE_EDGE | GPIO_INT_TRIG_BOTH,
		.sig_type = POWER_SIG_INTERRUPT,
	},
	[POWER_PCH_EC_SLP_SUS_N] = {
		.sig_name = "PCH_EC_SLP_SUS_N",
		.sig_dev = NULL,
		.pin = 18, /* MCHP_GPIO_022 */
		.sig_bind = "GPIO000_036",
		.flags = GPIO_INT_MODE_EDGE | GPIO_INT_TRIG_BOTH,
		.sig_type = POWER_SIG_INTERRUPT,
	},
	[POWER_VR_EC_RSMRST_PWRGD] = {
		.sig_name = "VR_EC_RSMRST_PWRGD",
		.sig_dev = NULL,
		.pin = 10, /* MCHP_GPIO_012 */
		.sig_bind = "GPIO000_036",
		.flags = GPIO_INT_MODE_EDGE | GPIO_INT_TRIG_BOTH,
		.sig_type = POWER_SIG_INTERRUPT,
	},
	[POWER_VR_EC_ALL_SYS_PWRGD] = {
		.sig_name = "VR_EC_ALL_SYS_PWRGD",
		.sig_dev = NULL,
		.pin = 15, /* MCHP_GPIO_057 */
		.sig_bind = "GPIO040_076",
		.flags = GPIO_INT_MODE_EDGE | GPIO_INT_TRIG_BOTH,
		.sig_type = POWER_SIG_INTERRUPT,
	},
	[POWER_PCH_EC_SLP_S0] = {
		.sig_name = "PCH_EC_SLP_S0",
		.sig_dev = NULL,
		.pin = 3, /* MCHP_GPIO_243 */
		.sig_bind = "GPIO240_276",
		.flags = GPIO_INT_MODE_EDGE | GPIO_INT_TRIG_BOTH,
		.sig_type = POWER_SIG_INTERRUPT,
	},
	[POWER_EC_PCH_RSMRST_N] = {
		.sig_name = "EC_PCH_RSMRST_N",
		.sig_dev = NULL,
		.pin = 12, /* MCHP_GPIO_054 */
		.sig_bind = "GPIO040_076",
		.flags = GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOW,
		.sig_type = POWER_SIG_GPIO,
	},
	[POWER_EC_VR_EN_5P0V_A] = {
		.sig_name = "EC_VR_EN_5P0V_A",
		.sig_dev = NULL,
		.pin = 19, /* MCHP_GPIO_023 */
		.sig_bind = "GPIO000_036",
		.flags = GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOW,
		.sig_type = POWER_SIG_GPIO,
	},
	[POWER_EC_VR_EN_3P3V_A] = {
		.sig_name = "EC_VR_EN_3P3V_A",
		.sig_dev = NULL,
		.pin = 20, /* MCHP_GPIO_024 */
		.sig_bind = "GPIO000_036",
		.flags = GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOW,
		.sig_type = POWER_SIG_GPIO,
	},
	[POWER_EC_PCH_DSW_PWROK] = {
		.sig_name = "EC_PCH_DSW_PWROK",
		.sig_dev = NULL,
		.pin = 18, /* MCHP_GPIO_062 */
		.sig_bind = "GPIO040_076",
		.flags = GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOW,
		.sig_type = POWER_SIG_GPIO,
	},
	[POWER_EC_PCH_PWRBTN_N] = {
		.sig_name = "EC_PCH_PWRBTN_N",
		.sig_dev = NULL,
		.pin = 21, /* MCHP_GPIO_165 */
		.sig_bind = "GPIO140_176",
		.flags = GPIO_OUTPUT | GPIO_OUTPUT_INIT_HIGH,
		.sig_type = POWER_SIG_GPIO,
	},
#ifdef PHYSICAL_SLP_SX_PINS
	[POWER_PCH_EC_SLP_S4] = {
		.sig_name = "PCH_EC_SLP_S4",
		.sig_dev = NULL,
		.pin = 3, /* MCHP_GPIO_241 */
		.sig_bind = "GPIO240_276",
		.flags = GPIO_INT_MODE_EDGE | GPIO_INT_TRIG_BOTH,
		.sig_type = POWER_SIG_INTERRUPT,
	},
	[POWER_PCH_EC_SLP_S3] = {
		.sig_name = "PCH_EC_SLP_S3",
		.sig_dev = NULL,
		.pin = 3, /* MCHP_GPIO_242 */
		.sig_bind = "GPIO240_276",
		.flags = GPIO_INT_MODE_EDGE | GPIO_INT_TRIG_BOTH,
		.sig_type = POWER_SIG_INTERRUPT,
	},
#endif
};
#else
/* TODO: Add config instead of assignment */
struct power_signals power_sig_list[] = {
	[POWER_PCH_EC_SLP_SUS_N] = {
		.sig_name = "PCH_EC_SLP_SUS_N",
		.sig_dev = NULL,
		.pin = NPCX_GPIO_D7,
		.sig_bind = "GPIO_D",
		.flags = GPIO_INT_MODE_EDGE | GPIO_INT_TRIG_BOTH,
		.sig_type = POWER_SIG_INTERRUPT,
	},
	[POWER_VR_EC_RSMRST_PWRGD] = {
		.sig_name = "VR_EC_RSMRST_PWRGD",
		.sig_dev = NULL,
		.pin = NPCX_GPIO_E2,
		.sig_bind = "GPIO_E",
		.flags = GPIO_INT_MODE_EDGE | GPIO_INT_TRIG_BOTH,
		.sig_type = POWER_SIG_INTERRUPT,
	},
	[POWER_VR_EC_ALL_SYS_PWRGD] = {
		.sig_name = "VR_EC_ALL_SYS_PWRGD",
		.sig_dev = NULL,
		.pin = NPCX_GPIO_F4,
		.sig_bind = "GPIO_F",
		.flags = GPIO_INT_MODE_EDGE | GPIO_INT_TRIG_BOTH,
		.sig_type = POWER_SIG_INTERRUPT,
	},
	[POWER_PCH_EC_SLP_S0] = {
		.sig_name = "PCH_EC_SLP_S0",
		.sig_dev = NULL,
		.pin = NPCX_GPIO_D5,
		.sig_bind = "GPIO_D",
		.flags = GPIO_INT_MODE_EDGE | GPIO_INT_TRIG_BOTH,
		.sig_type = POWER_SIG_INTERRUPT,
	},
	[POWER_EC_PCH_RSMRST_N] = {
		.sig_name = "EC_PCH_RSMRST_N",
		.sig_dev = NULL,
		.pin = NPCX_GPIO_A6,
		.sig_bind = "GPIO_A",
		.flags = GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOW, //TODO: ODR
		.sig_type = POWER_SIG_GPIO,
	},
	[POWER_EC_VR_EN_5P0V_A] = {
		.sig_name = "EC_VR_EN_5P0V_A",
		.sig_dev = NULL,
		.pin = NPCX_GPIO_A4,
		.sig_bind = "GPIO_A",
		.flags = GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOW,
		.sig_type = POWER_SIG_GPIO,
	},
	[POWER_EC_VR_EN_3P3V_A] = {
		.sig_name = "EC_VR_EN_3P3V_A",
		.sig_dev = NULL,
		.pin = NPCX_GPIO_A3,
		.sig_bind = "GPIO_A",
		.flags = GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOW,
		.sig_type = POWER_SIG_GPIO,
	},
	[POWER_EC_PCH_DSW_PWROK] = {
		.sig_name = "EC_PCH_DSW_PWROK",
		.sig_dev = NULL,
		.pin = NPCX_GPIO_C7,
		.sig_bind = "GPIO_C",
		.flags = GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOW,
		.sig_type = POWER_SIG_GPIO,
	},
	[POWER_EC_PCH_PWRBTN_N] = {
		.sig_name = "EC_PCH_PWRBTN_N",
		.sig_dev = NULL,
		.pin = NPCX_GPIO_C1,
		.sig_bind = "GPIO_C",
		.flags = GPIO_OUTPUT | GPIO_OUTPUT_INIT_HIGH, //TODO: ODR
		.sig_type = POWER_SIG_GPIO,
	},
	[POWER_EC_PCH_SYS_PWROK] = {
		.sig_name = "EC_PCH_SYS_PWROK",
		.sig_dev = NULL,
		.pin = NPCX_GPIO_37,
		.sig_bind = "GPIO_3",
		.flags = GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOW,
		.sig_type = POWER_SIG_GPIO,
	},
	[POWER_EC_VR_EN_VCCIN] = {
		.sig_name = "EC_EC_VR_EN_VCCIN",
		.sig_dev = NULL,
		.pin = NPCX_GPIO_43,
		.sig_bind = "GPIO_4",
		.flags = GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOW,
		.sig_type = POWER_SIG_GPIO,
	},
#ifdef PHYSICAL_SLP_SX_PINS
	[POWER_PCH_EC_SLP_S3] = {
		.sig_name = "PCH_EC_SLP_S3",
		.sig_dev = NULL,
		.pin = NPCX_GPIO_A5,
		.sig_bind = "GPIO_A",
		.flags = GPIO_INT_MODE_EDGE | GPIO_INT_TRIG_BOTH,
		.sig_type = POWER_SIG_INTERRUPT,
	},
#endif
};
#endif

static void power_seq_callback(const struct device *dev, struct gpio_callback *cb,
		gpio_port_pins_t pins)
{
	//printk("GPIO is triggered, pins=%d, val=%d\n",
	//	pins, gpio_pin_get(dev, __builtin_ffsl(pins) - 1));
	//k_thread_resume(&power_seq_thread_id);
}

static void power_signals_configure(void)
{
	struct power_signals *sig;
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(power_sig_list); i++) {
		sig = &power_sig_list[i];
		printk("Configuring %s\n", sig->sig_name);

		/* Get the GPIO binding */
		sig->sig_dev = device_get_binding(sig->sig_bind);
		if (!sig->sig_dev) {
			printk("Failed to get GPIO %s binding\n",
				sig->sig_name);
			return;
		}

		/* Configure the GPIO */
		ret = gpio_pin_configure(sig->sig_dev, sig->pin, sig->flags);
		if (ret != 0) {
			printk("Failed to configure GPIO %s, err %d\n",
				sig->sig_name, ret);
			return;
		}

		/* Configure Interrupts */
		if (sig->sig_type == POWER_SIG_INTERRUPT) {
			gpio_init_callback(&power_seq_cb_data,
				power_seq_callback, BIT(sig->pin));
			gpio_add_callback(sig->sig_dev, &power_seq_cb_data);
		}
	}
}

static int power_gpio_sig_get(enum power_gpio_signals signal)
{
	struct power_signals *sig = &power_sig_list[signal];

	return gpio_pin_get(sig->sig_dev, sig->pin);
}

static int power_gpio_sig_set(enum power_gpio_signals signal, int val)
{
	struct power_signals *sig = &power_sig_list[signal];

	return gpio_pin_set(sig->sig_dev, sig->pin, val);
}

static int power_vw_sig_get(enum espi_vwire_signal signal)
{
	uint8_t level;

	espi_receive_vwire(espi_dev, signal, &level);
	return level;
}

static void espi_bus_reset(void)
{
	/* If SOC is up toggle the PM_PWRBTN pin */
	if (power_gpio_sig_get(POWER_PCH_EC_SLP_SUS_N)) {
		printk("Toggle PM PWRBTN\n");
		power_gpio_sig_set(POWER_EC_PCH_PWRBTN_N, 0);
		k_msleep(POWE_EC_PCH_PM_PWRBTN_DELAY_MS);
		power_gpio_sig_set(POWER_EC_PCH_PWRBTN_N, 1);
	}
}

static void espi_vw_handler(struct espi_event *event)
{
	printk("VW is triggered, event=%d, val=%d\n", event->evt_details,
			power_vw_sig_get(event->evt_details));

	switch (event->evt_details) {
	case ESPI_VWIRE_SIGNAL_SLP_S3:
	case ESPI_VWIRE_SIGNAL_SLP_S4:
	case ESPI_VWIRE_SIGNAL_PLTRST:
		//k_thread_resume(&power_seq_thread_id);
		break;
	default:
		break;
	}
}

static void espi_bus_handler(const struct device *dev,
				struct espi_callback *cb,
				struct espi_event event)
{
	switch (event.evt_type) {
	case ESPI_BUS_RESET:
		printk("ESPI bus reset\n");
		espi_bus_reset();
		break;
	case ESPI_BUS_EVENT_VWIRE_RECEIVED:
		printk("ESPI VW received\n");
		espi_vw_handler(&event);
		break;
	case ESPI_BUS_EVENT_CHANNEL_READY:
		printk("ESPI channel ready\n");
		break;
	default:
		break;
	}
}

void power_espi_configure(void)
{
	struct espi_cfg cfg = {
		.io_caps = ESPI_IO_MODE_SINGLE_LINE,
		.channel_caps = ESPI_CHANNEL_VWIRE |
			ESPI_CHANNEL_PERIPHERAL |
			ESPI_CHANNEL_OOB,
		.max_freq = ESPI_FREQ_MHZ,
	};

	printk("configuring eSPI\n");
	espi_dev = device_get_binding("ESPI_0");
	if (!espi_dev) {
		printk("Failed to get eSPI binding\n");
		return;
	}

	if (espi_config(espi_dev, &cfg)) {
		printk("Failed to configure eSPI\n");
		return;
	}

	espi_init_callback(&espi_bus_cb, espi_bus_handler, ESPI_BUS_RESET);
	espi_add_callback(espi_dev, &espi_bus_cb);

	espi_init_callback(&espi_chan_cb, espi_bus_handler,
			ESPI_BUS_EVENT_CHANNEL_READY);
	espi_add_callback(espi_dev, &espi_chan_cb);

	espi_init_callback(&espi_vw_cb, espi_bus_handler,
			ESPI_BUS_EVENT_VWIRE_RECEIVED);
	espi_add_callback(espi_dev, &espi_vw_cb);
}

static void power_pass_thru_handler(enum power_gpio_signals in_signal,
			enum power_gpio_signals out_signal, int delay_ms)
{
	int in_sig_val = power_gpio_sig_get(in_signal);

	if (in_sig_val != power_gpio_sig_get(out_signal)) {
		if (in_sig_val)
			k_msleep(delay_ms);
		power_gpio_sig_set(out_signal, in_sig_val);
	}
}

static enum power_state power_state_handler(enum power_state state)
{
	/* Handle DSW passthrough */
#ifdef TGLRVP
	power_pass_thru_handler(POWER_VR_EC_DSW_PWROK, POWER_EC_PCH_DSW_PWROK,
			POWE_EC_PCH_DSW_PWROK_DELAY_MS);
#else
	/* Note: Volteer Proto-2 doesnt have VR_EC_DSW_PWROK */
	power_pass_thru_handler(POWER_EC_VR_EN_3P3V_A, POWER_EC_PCH_DSW_PWROK,
			POWE_EC_PCH_DSW_PWROK_DELAY_MS);
#endif

	/* Handle RSMRST passthrough */
	power_pass_thru_handler(POWER_VR_EC_RSMRST_PWRGD, POWER_EC_PCH_RSMRST_N,
			POWE_EC_PCH_RSMRST_DELAY_MS);

#ifndef TGLRVP
	/* Handle SYS_PWROK passthrough */
	power_pass_thru_handler(POWER_VR_EC_ALL_SYS_PWRGD, POWER_EC_PCH_SYS_PWROK,
			POWE_EC_PCH_SYS_PWROK_DELAY_MS);
	/* Handle SYS_PWROK passthrough */
	power_pass_thru_handler(POWER_VR_EC_ALL_SYS_PWRGD, POWER_EC_VR_EN_VCCIN,
			POWE_EC_VR_EN_VCCIN_DELAY_MS);
#endif

	switch (state) {
	case POWER_G3:
		printk("Enable A-rails\n");
		power_gpio_sig_set(POWER_EC_VR_EN_5P0V_A, 1);
		power_gpio_sig_set(POWER_EC_VR_EN_3P3V_A, 1);
		state = POWER_G3_S5;
		break;
	case POWER_G3_S5:
		state = power_gpio_sig_get(POWER_PCH_EC_SLP_SUS_N) ?
				POWER_S5 : POWER_G3;
		break;
	default:
		break;
	}

	return state;
}

static void power_print_gpios(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(power_sig_list); i++)
		printk("GPIO %s=%d\n", power_sig_list[i].sig_name,
			power_gpio_sig_get(i));

	printk("GPIO ESPI_VWIRE_SIGNAL_SLP_S3=%d\n",
			power_vw_sig_get(ESPI_VWIRE_SIGNAL_SLP_S3));
	printk("GPIO ESPI_VWIRE_SIGNAL_SLP_S4=%d\n",
			power_vw_sig_get(ESPI_VWIRE_SIGNAL_SLP_S4));
}

void power_seq_thread_entry(void *p1, void *p2, void *p3)
{
	enum power_state pow_state = POWER_G3;

	power_signals_configure();
	power_espi_configure();
	printk("in %d\n", __LINE__);

	while(true) {
		printk("\n@%d, in power state %d\n", __LINE__, pow_state);
		power_print_gpios();
		pow_state = power_state_handler(pow_state);
		//k_thread_suspend(&power_seq_thread_id);
		k_msleep(1000);
	}
}

void test_tgl_power_sequence(void)
{
	k_thread_create(&power_seq_thread_id, power_seq_thread_stack,
		POW_SEQ_THREAD_STACK_SIZE, power_seq_thread_entry,
		NULL, NULL, NULL, POW_SEQ_THREAD_PRIORITY,
		K_INHERIT_PERMS, K_NO_WAIT);
	k_thread_start(&power_seq_thread_id);
}

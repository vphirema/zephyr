/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __POW_SEQ_H__
#define __POW_SEQ_H__

#include <drivers/gpio.h>

/* Thread properties */
#define POW_SEQ_THREAD_STACK_SIZE		2048ul
#define POW_SEQ_THREAD_PRIORITY		K_PRIO_COOP(5)

#define POWE_EC_PCH_DSW_PWROK_DELAY_MS	100
#define POWE_EC_PCH_RSMRST_DELAY_MS	10
#define POWE_EC_PCH_SYS_PWROK_DELAY_MS	50
#define POWE_EC_VR_EN_VCCIN_DELAY_MS	5
#define POWE_EC_PCH_PM_PWRBTN_DELAY_MS	200

/* #define PHYSICAL_SLP_SX_PINS */
#define TGLRVP

/* Power states */
enum power_state {
	POWER_G3,
	POWER_S5,
	POWER_S3,
	POWER_S0,
	POWER_G3_S5,
	POWER_S5_S3,
	POWER_S3_S0,
};

#ifdef TGLRVP
/* Power sequence signals list */
enum power_gpio_signals {
	POWER_VR_EC_DSW_PWROK,
	POWER_PCH_EC_SLP_SUS_N,
	POWER_VR_EC_RSMRST_PWRGD,
	POWER_VR_EC_ALL_SYS_PWRGD,
	POWER_PCH_EC_SLP_S0,
	POWER_EC_PCH_RSMRST_N,
	POWER_EC_VR_EN_5P0V_A,
	POWER_EC_VR_EN_3P3V_A,
	POWER_EC_PCH_DSW_PWROK,
	POWER_EC_PCH_PWRBTN_N,
#ifdef PHYSICAL_SLP_SX_PINS
	POWER_PCH_EC_SLP_S4,
	POWER_PCH_EC_SLP_S3,
#endif
	POWER_GPIO_SIGNAL_COUNT
};
#else
/* Power sequence signals list */
enum power_gpio_signals {
	POWER_PCH_EC_SLP_SUS_N,
	POWER_VR_EC_RSMRST_PWRGD,
	POWER_VR_EC_ALL_SYS_PWRGD,
	POWER_PCH_EC_SLP_S0,
	POWER_EC_PCH_RSMRST_N,
	POWER_EC_VR_EN_5P0V_A,
	POWER_EC_VR_EN_3P3V_A,
	POWER_EC_PCH_DSW_PWROK,
	POWER_EC_PCH_PWRBTN_N,
	POWER_EC_PCH_SYS_PWROK,
	POWER_EC_VR_EN_VCCIN,
#ifdef PHYSICAL_SLP_SX_PINS
	POWER_PCH_EC_SLP_S3,
#endif
	POWER_GPIO_SIGNAL_COUNT
};
#endif
enum power_espi_signals {
#ifndef PHYSICAL_SLP_SX_PINS
	POWER_PCH_EC_SLP_S4,
	POWER_PCH_EC_SLP_S3,
#endif
	POWER_PCH_EC_PLT_RST,
	POWER_ESPI_SIGNAL_COUNT
};

enum powe_sig_type {
	POWER_SIG_GPIO,
	POWER_SIG_INTERRUPT,
};

struct power_signals {
	enum powe_sig_type sig_type;
	const struct device *sig_dev;
	const char *sig_name;
	const char *sig_bind;
	gpio_flags_t flags;
	gpio_pin_t pin;
};

/** @brief Test the TGLRVP non-Deep Sx power sequence.
 *
 * Test enables the power rails of TGLRVP and prints current the power states.
 */
void test_tgl_power_sequence(void);

#endif /* __POW_SEQ_H__ */

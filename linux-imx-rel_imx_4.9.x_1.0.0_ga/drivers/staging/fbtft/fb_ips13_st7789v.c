/*
 * FB driver for the Zhongjingyuan IPS1.3 240x240 ST7789V LCD Controller
 *
 * Copyright (C) 2020 Hare
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <video/mipi_display.h>

#include "fbtft.h"

#define DRVNAME "fb_st7789v_240x240"


/**
 * enum st7789v_command - ST7789V display controller commands
 */
enum st7789v_command {
  SET_SLEEP_IN = 0x10,
  SET_SLEEP_OUT = 0x11,
  SET_DISP_WIN_X = 0x2a,
  SET_DISP_WIN_Y = 0x2b,
  WRITE_MEMORY_START = 0x2c,
};

#define MADCTL_BGR BIT(3) /* bitmask for RGB/BGR order */
#define MADCTL_MV BIT(5) /* bitmask for page/column order */
#define MADCTL_MX BIT(6) /* bitmask for column address order */
#define MADCTL_MY BIT(7) /* bitmask for page address order */


static int ips13_init_sequence[] = { 
  -1, 0x36, 0x00, 
  -1, 0x3A, 0x05, 
  -1, 0xB2, 0x0C, 0x0C, 0x00, 0x33, 0x33, 
  -1, 0xB7, 0x35,
  -1, 0xBB, 0x19, 
  -1, 0xC0, 0x2C, 
  -1, 0xC2, 0x01, 
  -1, 0xC3, 0x12,
  -1, 0xC4, 0x20,
  -1, 0xC6, 0x0F,
  -1, 0xD0, 0xA4, 0xA1, 
  -1, 0xE0, 0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23, 
  -1, 0xE1, 0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23, 
  /* delay 120ms */
  -2, 120, 
  -1, 0x21, 
  -1, 0x11, 
  -1, 0x29, 
  /* end marker */
  -3, 
};


/**
 * set_var() - apply LCD properties like rotation and BGR mode
 *
 * @par: FBTFT parameter object
 *
 * Return: 0 on success, < 0 if error occurred.
 */
static int set_var(struct fbtft_par *par)
{
	u8 madctl_par = 0;
	if (par->bgr)
		madctl_par |= MADCTL_BGR;
	switch (par->info->var.rotate) {
    case 0:
      break;
    case 90:
      madctl_par |= (MADCTL_MV | MADCTL_MY);
      break;
    case 180:
      madctl_par |= (MADCTL_MX | MADCTL_MY);
      break;
    case 270:
      madctl_par |= (MADCTL_MV | MADCTL_MX);
      break;
    default:
      return -EINVAL;
	}
	write_reg(par, MIPI_DCS_SET_ADDRESS_MODE, madctl_par);

	return 0;
}

/**
 * sset_addr_win() - Set the GRAM update window
 */
static void set_addr_win(struct fbtft_par *par, int xs, int ys, int xe, int ye)
{

  /* save the last x,y */
  /* if pos not change, not need to update the regs */
  static uint16_t s_x1 = 0xFFFF;
  static uint16_t s_x2 = 0xFFFF;
  static uint16_t s_y1 = 0xFFFF;
  static uint16_t s_y2 = 0xFFFF;

  uint16_t x1, x2, y1, y2;

  /* because ST7789 defalut is 240x320, but IPS1.3 is 240x240 */
  /* if rotate the display to 90 or 180, need to cut the blank part */
  if (par->info->var.rotate == 90)
  {
      xs += (320 - par->pdata->display.height);
  }
  x1 = xs;
  x2 = xs + xe;
  if (s_x1 != x1 || s_x2 != x2)
  {
    write_reg(par, SET_DISP_WIN_X,
              x1 >> 8, x1 & 0xFF, x2 >> 8, x2 & 0xFF);
    s_x1 = x1;
    s_x2 = x2;
  }

  if (par->info->var.rotate == 180)
  {
      ys += (320 - par->pdata->display.width);
  }
  y1 = ys;
  y2 = ys + ye;
  if (s_y1 != y1 || s_y2 != y2)
  {
    write_reg(par, SET_DISP_WIN_Y,
		          y1 >> 8, y1 & 0xFF, y2 >> 8, y2 & 0xFF);
    s_y1 = y1;
    s_y2 = y2;
  }

	write_reg(par, WRITE_MEMORY_START);
}

/**
 * blank() - blank the display
 *
 * @par: FBTFT parameter object
 * @on: whether to enable or disable blanking the display
 *
 * Return: 0 on success, < 0 if error occurred.
 */
static int blank(struct fbtft_par *par, bool on)
{
	if (on)
  {
    /* display off */
    write_reg(par, SET_SLEEP_IN);
  }
	else
  {
    /* display on */
    write_reg(par, SET_SLEEP_OUT);
  }
		
	return 0;
}

// fbtft_write_vmem16_bus16(struct fbtft_par *par, size_t offset, size_t len);
static int ips13_write_vmem16(struct fbtft_par *par, size_t offset, size_t len)
{
  par->spi->bits_per_word = 16;
  fbtft_write_vmem16_bus16(par, offset, len);
  par->spi->bits_per_word = 8;
	return 0;
}

static struct fbtft_display display = {
	.regwidth = 8,
  .buswidth = 8,
  .bpp = 16,
  .init_sequence = ips13_init_sequence,
	.fbtftops = {
    .write_vmem = ips13_write_vmem16,
		.set_var = set_var,
    .set_addr_win = set_addr_win,
		.blank = blank,
	},
};

FBTFT_REGISTER_DRIVER(DRVNAME, "sitronix,st7789v_240x240", &display);

MODULE_ALIAS("spi:" DRVNAME);
MODULE_ALIAS("platform:" DRVNAME);
MODULE_ALIAS("spi:st7789v");
MODULE_ALIAS("platform:st7789v");

MODULE_DESCRIPTION("FB driver for the Zhongjingyuan IPS1.3 240x240 ST7789V LCD Controller");
MODULE_AUTHOR("Hare");
MODULE_LICENSE("GPL");

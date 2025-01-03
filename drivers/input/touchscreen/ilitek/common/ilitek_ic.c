/*
 * ILITEK Touch IC driver
 *
 * Copyright (C) 2011 ILI Technology Corporation.
 *
 * Author: Dicky Chiang <dicky_chiang@ilitek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "ilitek_common_spi.h"

#define PROTOCL_VER_NUM		8
static struct ilitek_protocol_info protocol_info[PROTOCL_VER_NUM] = {
	/* length -> fw, protocol, tp, key, panel, core, func, window, cdc, mp_info */
	[0] = {PROTOCOL_VER_500, 4, 4, 14, 30, 5, 5, 2, 8, 3, 8},
	[1] = {PROTOCOL_VER_510, 4, 3, 14, 30, 5, 5, 3, 8, 3, 8},
	[2] = {PROTOCOL_VER_520, 4, 4, 14, 30, 5, 5, 3, 8, 3, 8},
	[3] = {PROTOCOL_VER_530, 9, 4, 14, 30, 5, 5, 3, 8, 3, 8},
	[4] = {PROTOCOL_VER_540, 9, 4, 14, 30, 5, 5, 3, 8, 15, 8},
	[5] = {PROTOCOL_VER_550, 9, 4, 14, 30, 5, 5, 3, 8, 15, 14},
	[6] = {PROTOCOL_VER_560, 9, 4, 14, 30, 5, 5, 3, 8, 15, 14},
	[7] = {PROTOCOL_VER_570, 9, 4, 14, 30, 5, 5, 3, 8, 15, 14},
};

#define FUNC_CTRL_NUM	20
static struct ilitek_ic_func_ctrl func_ctrl[FUNC_CTRL_NUM] = {
	/* cmd[3] = cmd, func, ctrl */
	// rec_state 0:disable, 1: enable, 2: ignore record
	[0] = {"sense", {0x1, 0x1, 0x0}, 3, 0x0, 2, 0xFF},
	[1] = {"sleep", {0x1, 0x2, 0x0}, 3, 0x0, 2, 0xFF},
	[2] = {"glove", {0x1, 0x6, 0x0}, 3, 0x0, 2, 0xFF},
	[3] = {"stylus", {0x1, 0x7, 0x0}, 3, 0x0, 2, 0xFF},
	[4] = {"lpwg", {0x1, 0xA, 0x0}, 3, 0x0, 2, 0xFF},
	[5] = {"proximity", {0x1, 0x10, 0x0}, 3, 0x0, 2, 0xFF},
	[6] = {"plug", {0x1, 0x11, 0x0}, 3, 0x1, 2, 0xFF},
	[7] = {"edge_palm", {0x1, 0x12, 0x0}, 3, 0x1, 2, 0xFF},
	[8] = {"lock_point", {0x1, 0x13, 0x0}, 3, 0x0, 2, 0xFF},
	[9] = {"active", {0x1, 0x14, 0x0}, 3, 0x0, 2, 0xFF},
	[10] = {"idle", {0x1, 0x19, 0x0}, 3, 0x1, 2, 0xFF},
	[11] = {"gesture_demo_en", {0x1, 0x16, 0x0}, 3, 0x0, 2, 0xFF},
	[12] = {"tp_recore", {0x1, 0x18, 0x0}, 3, 0x0, 2, 0xFF},
	[13] = {"knock_en", {0x1, 0xA, 0x8, 0x03, 0x0, 0x0}, 6, 0xFF, 2, 0xFF},
	[14] = {"int_trigger", {0x1, 0x1B, 0x0}, 3, 0x0, 2, 0xFF},
	[15] = {"ear_phone", {0x1, 0x17, 0x0}, 3, 0x0, 2, 0xFF},
	[16] = {"sip_mode", {0x1, 0x20, 0x0}, 3, 0x1, 2, 0xFF},
	[17] = {"dead_zone_ctrl", {0x1, 0x12, 0x3}, 3, 0x0, 2, 0xFF},
	[18] = {"cover_mode", {0x1, 0x0C, 0x00}, 3, 0x0, 2, 0xFF},
	[19] = {"high_sensitivity_mode", {0x1, 0x25, 0x00}, 3, 0x0, 2, 0xFF},
};

#define CHIP_SUP_NUM	5
static u32 ic_sup_list[CHIP_SUP_NUM] = {
	[0] = ILI9881_CHIP,
	[1] = ILI7807_CHIP,
	[2] = ILI9881N_AA,
	[3] = ILI9881O_AA,
	[4] = ILI9882_CHIP
};

int ili_tddi_ic_sram_test(void)
{
	int retry = 20;
	u32 bist_done = 0, bist_fail = 0, bist_fail_bus = 0, inbist_failbus = 0;

	ili_ice_mode_write(0x040004, 0x01, 1);
	ili_ice_mode_write(0x040010, 0x9878, 2);
	ili_ice_mode_write(0x047003, 0x00, 1);
	ili_ice_mode_write(0x04001C, 0x01, 1);

	ili_ice_mode_write(0x071001, 0x02, 1);
	ili_ice_mode_write(0x071005, 0x3C, 1);

	ili_ice_mode_write(0x040088, 0x6B8A, 2);

	ili_ice_mode_write(0x046003, 0x00, 1);

	ili_ice_mode_write(0x046000, 0x010001, 3);

	ili_ice_mode_write(0x046002, 0x00, 1);

	input_info(true, ilits->dev, "%s id:0x%x, type:0x%x\n", __func__, ilits->chip->id, ilits->chip->type);
	switch (ilits->chip->id) {
	case ILI9881_CHIP:
		if (ilits->chip->type == 0x1D) {
			/* 9881T */
			ili_ice_mode_write(0x046004, 0x1FFFFF00, 4);
		} else {
			ili_ice_mode_write(0x046004, 0x27FFFF00, 4);
		}
		break;
	case ILI9882_CHIP:
	/* 9882N */
		ili_ice_mode_write(0x046004, 0x1FFFFF00, 4);
		break;
	case ILI7807_CHIP:
		if (ilits->chip->type == 0x1F) {
			/* 7807V *//* 7806S */
			ili_ice_mode_write(0x046004, 0x0FFFFF00, 4);
		} else {
			ili_ice_mode_write(0x046004, 0x1FFFFF00, 4);
		}
		break;
	default:
		ili_ice_mode_write(0x046004, 0x1FFFFF00, 4);
		break;
	}

	ili_ice_mode_write(0x046014, 0xA4409800, 4);
	ili_ice_mode_write(0x046018, 0xB480F590, 4);
	ili_ice_mode_write(0x04601C, 0x9000F560, 4);
	ili_ice_mode_write(0x046020, 0x9280C460, 4);
	ili_ice_mode_write(0x046024, 0x0000, 2);

	ili_ice_mode_write(0x046000, 0x000000, 4);
	ili_ice_mode_read(0x04600C, &bist_done, sizeof(bist_done));
	ili_ice_mode_read(0x04600D, &bist_fail, sizeof(bist_fail));
	ili_ice_mode_read(0x046010, &bist_fail_bus, sizeof(bist_fail_bus));

	if (((bist_done & BIT(0)) != 0x00) || ((bist_fail & BIT(0)) != 0x01) || (bist_fail_bus != 0xFFFFFFFF)) {
		ILI_DBG("SRAM TEST FAIL\n");
		return -1;
	}

	ili_ice_mode_write(0x046000, 0x00000001, 4);
	ili_ice_mode_write(0x046000, 0x00000101, 4);

	while (retry > 0) {
		usleep_range(10 * 1000, 10 * 1000);
		ili_ice_mode_read(0x04600C, &bist_done, sizeof(bist_done));

		if ((bist_done & BIT(0)) == 0x01)
			break;
		retry--;
	}
	if (retry <= 0) {
		ILI_DBG("SRAM TEST FAIL\n");
		return -1;
	}

	ili_ice_mode_read(0x04600D, &bist_fail, sizeof(bist_fail));
	ili_ice_mode_read(0x046010, &inbist_failbus, sizeof(inbist_failbus));

	if (((bist_fail & BIT(0)) != 0x00) || (inbist_failbus != 0x00000000)) {
		ILI_DBG("SRAM TEST FAIL\n");
		return -1;
	}

	ILI_DBG("SRAM TEST PASS\n");
	return 0;

}

static int ilitek_tddi_ic_check_support(u32 pid, u16 id)
{
	int i = 0;

	for (i = 0; i < CHIP_SUP_NUM; i++) {
		if ((pid == ic_sup_list[i]) || (id == ic_sup_list[i]))
			break;
	}

	if (i >= CHIP_SUP_NUM)
		input_info(true, ilits->dev, "%s ERROR, ILITEK CHIP(0x%x) Not found !!\n", __func__, pid);

	input_info(true, ilits->dev, "%s ILITEK CHIP ILI9881x found.\n", __func__);

	ilits->chip->pid = pid;

	ilits->chip->reset_key = 0x00019878;
	ilits->chip->wtd_key = 0x9881;

	if (((pid & 0xFFFFFF00) == ILI9881N_AA) || ((pid & 0xFFFFFF00) == ILI9881O_AA))
		ilits->chip->dma_reset = ENABLE;
	else
		ilits->chip->dma_reset = DISABLE;

	ilits->chip->no_bk_shift = RAWDATA_NO_BK_SHIFT;
	ilits->chip->max_count = 0x1FFFF;
	return 0;
}

int ili_ice_mode_bit_mask_write(u32 addr, u32 mask, u32 value)
{
	int ret = 0;
	u32 data = 0;

	if (ili_ice_mode_read(addr, &data, sizeof(u32)) < 0) {
		input_err(true, ilits->dev, "%s Read data error\n", __func__);
		return -1;
	}

	data &= (~mask);
	data |= (value & mask);

	ILI_DBG("%s mask value data = %x\n", __func__, data);

	ret = ili_ice_mode_write(addr, data, sizeof(u32));
	if (ret < 0)
		input_err(true, ilits->dev, "%s Failed to re-write data in ICE mode, ret = %d\n", __func__, ret);

	return ret;
}

int ili_ice_mode_write(u32 addr, u32 data, int len)
{
	int ret = 0, i;
	u8 txbuf[64] = {0};

	if (!atomic_read(&ilits->ice_stat)) {
		input_err(true, ilits->dev, "%s ice mode not enabled\n", __func__);
		return -1;
	}

	txbuf[0] = 0x25;
	txbuf[1] = (char)((addr & 0x000000FF) >> 0);
	txbuf[2] = (char)((addr & 0x0000FF00) >> 8);
	txbuf[3] = (char)((addr & 0x00FF0000) >> 16);

	for (i = 0; i < len; i++)
		txbuf[i + 4] = (char)(data >> (8 * i));

	ret = ilits->wrapper(txbuf, len + 4, NULL, 0, OFF, OFF);
	if (ret < 0)
		input_err(true, ilits->dev, "%s Failed to write data in ice mode, ret = %d\n", __func__, ret);

	return ret;
}

int ili_ice_mode_read(u32 addr, u32 *data, int len)
{
	int ret = 0;
	u8 *rxbuf = NULL;
	u8 txbuf[4] = {0};

	if (!atomic_read(&ilits->ice_stat)) {
		input_err(true, ilits->dev, "%s ice mode not enabled\n", __func__);
		return -1;
	}

	txbuf[0] = 0x25;
	txbuf[1] = (char)((addr & 0x000000FF) >> 0);
	txbuf[2] = (char)((addr & 0x0000FF00) >> 8);
	txbuf[3] = (char)((addr & 0x00FF0000) >> 16);

	ret = ilits->wrapper(txbuf, sizeof(txbuf), NULL, 0, OFF, OFF);
	if (ret < 0)
		goto out;

	rxbuf = kcalloc(len, sizeof(u8), GFP_KERNEL);
	if (ERR_ALLOC_MEM(rxbuf)) {
		input_err(true, ilits->dev, "%s Failed to allocate rxbuf, %ld\n", __func__, PTR_ERR(rxbuf));
		ret = -ENOMEM;
		goto out;
	}

	ret = ilits->wrapper(NULL, 0, rxbuf, len, OFF, OFF);
	if (ret < 0)
		goto out;

	if (len == sizeof(u8))
		*data = rxbuf[0];
	else
		*data = (rxbuf[0] | rxbuf[1] << 8 | rxbuf[2] << 16 | rxbuf[3] << 24);

out:
	if (ret < 0)
		input_err(true, ilits->dev, "%s Failed to read data in ice mode, ret = %d\n", __func__, ret);

	ipio_kfree((void **)&rxbuf);
	return ret;
}

int ili_ice_mode_ctrl(bool enable, bool mcu)
{
	int ret = 0;
	u8 cmd_open[4] = {0x25, 0x62, 0x10, 0x18};
	u8 cmd_close[4] = {0x1B, 0x62, 0x10, 0x18};

	input_info(true, ilits->dev, "%s %s ICE mode, mcu on = %d\n", __func__, (enable ? "Enable" : "Disable"), mcu);

	if (enable) {
		if (atomic_read(&ilits->ice_stat)) {
			input_info(true, ilits->dev, "%s ice mode already enabled\n", __func__);
			return 0;
		}

		if (mcu)
			cmd_open[0] = 0x1F;

		atomic_set(&ilits->ice_stat, ENABLE);

		if (ilits->wrapper(cmd_open, sizeof(cmd_open), NULL, 0, OFF, OFF) < 0) {
			input_err(true, ilits->dev, "%s write ice mode cmd error\n", __func__);
			atomic_set(&ilits->ice_stat, DISABLE);
		}
		ilits->pll_clk_wakeup = false;

#if (TDDI_INTERFACE == BUS_I2C)
		if (ili_ice_mode_write(FLASH_BASED_ADDR, 0x1, 1) < 0)
			input_err(true, ilits->dev, "%s Write cs high failed\n", __func__); /* CS high */
#endif
	} else {
		if (!atomic_read(&ilits->ice_stat)) {
			input_info(true, ilits->dev, "%s ice mode already disabled\n", __func__);
			return 0;
		}

		ret = ilits->wrapper(cmd_close, sizeof(cmd_close), NULL, 0, OFF, OFF);
		if (ret < 0) {
			input_err(true, ilits->dev, "%s Exit to ICE Mode failed !!\n", __func__);
			atomic_set(&ilits->ice_stat, ENABLE);
		} else {
			atomic_set(&ilits->ice_stat, DISABLE);
			ilits->pll_clk_wakeup = true;
		}
	}

	return ret;
}

int ili_ic_func_ctrl(const char *name, int ctrl)
{
	int i = 0, ret;

	for (i = 0; i < FUNC_CTRL_NUM; i++) {
		if (ipio_strcmp(name, func_ctrl[i].name) == 0) {
			if (strlen(name) != strlen(func_ctrl[i].name))
				continue;
			break;
		}
	}

	if (i >= FUNC_CTRL_NUM) {
		input_err(true, ilits->dev, "%s Not found function ctrl, %s\n", __func__, name);
		ret = -1;
		goto out;
	}

	if (ilits->protocol->ver == PROTOCOL_VER_500) {
		input_err(true, ilits->dev, "%s Non support function ctrl with protocol v5.0\n", __func__);
		ret = -1;
		goto out;
	}

	if (ilits->protocol->ver >= PROTOCOL_VER_560) {
		if (ipio_strcmp(func_ctrl[i].name, "gesture") == 0 ||
			ipio_strcmp(func_ctrl[i].name, "phone_cover_window") == 0) {
			input_info(true, ilits->dev, "%s Non support %s function ctrl\n", __func__, func_ctrl[i].name);
			ret = -1;
			goto out;
		}
	}

	func_ctrl[i].cmd[2] = ctrl;

	input_info(true, ilits->dev, "%s func = %s, len = %d, cmd = 0x%x, 0%x, 0x%x\n",
		__func__, func_ctrl[i].name, func_ctrl[i].len, func_ctrl[i].cmd[0],
		func_ctrl[i].cmd[1], func_ctrl[i].cmd[2]);

	ret = ilits->wrapper(func_ctrl[i].cmd, func_ctrl[i].len, NULL, 0, OFF, OFF);
	if (ret < 0)
		input_err(true, ilits->dev, "%s Write TP function failed\n", __func__);

	if (func_ctrl[i].rec_state < 2) {
		if (ctrl == func_ctrl[i].def_cmd)
			func_ctrl[i].rec_state = DISABLE;
		else
			func_ctrl[i].rec_state = ENABLE;

		func_ctrl[i].rec_cmd = ctrl;
	}

	ILI_DBG("%s record %s func cmd %d, rec_state %d\n",
		__func__, func_ctrl[i].name, func_ctrl[i].rec_cmd, func_ctrl[i].rec_state);
out:
	return ret;
}

int ili_ic_code_reset(void)
{
	int ret;
	bool ice = atomic_read(&ilits->ice_stat);

	if (!ice)
		if (ili_ice_mode_ctrl(ENABLE, OFF) < 0)
			input_err(true, ilits->dev, "%s Enable ice mode failed before code reset\n", __func__);

	ret = ili_ice_mode_write(0x40040, 0xAE, 1);
	if (ret < 0)
		input_err(true, ilits->dev, "%s ic code reset failed\n", __func__);

	if (!ice)
		if (ili_ice_mode_ctrl(DISABLE, OFF) < 0)
			input_err(true, ilits->dev, "%s Enable ice mode failed after code reset\n", __func__);
	return ret;
}

int ili_ic_whole_reset(void)
{
	int ret = 0;
	bool ice = atomic_read(&ilits->ice_stat);

	if (!ice)
		if (ili_ice_mode_ctrl(ENABLE, OFF) < 0)
			input_err(true, ilits->dev, "%s Enable ice mode failed before chip reset\n", __func__);

	input_info(true, ilits->dev, "%s ic whole reset key = 0x%x, edge_delay = %d\n",
		__func__, ilits->chip->reset_key, ilits->rst_edge_delay);

	ret = ili_ice_mode_write(ilits->chip->reset_addr, ilits->chip->reset_key, sizeof(u32));
	if (ret < 0) {
		input_err(true, ilits->dev, "%s ic whole reset failed\n", __func__);
		goto out;
	}

	msleep(ilits->rst_edge_delay);

out:
	if (!ice)
		if (ili_ice_mode_ctrl(DISABLE, OFF) < 0)
			input_err(true, ilits->dev, "%s Enable ice mode failed after chip reset\n", __func__);

	return ret;
}

static void ilitek_tddi_ic_wr_pack(int packet)
{
	int retry = 100;
	u32 reg_data = 0;

	while (retry--) {
		if (ili_ice_mode_read(0x73010, &reg_data, sizeof(u8)) < 0)
			input_err(true, ilits->dev, "%s Read 0x73010 error\n", __func__);

		if ((reg_data & 0x02) == 0) {
			input_info(true, ilits->dev, "%s check ok 0x73010 read 0x%X retry = %d\n",
				__func__, reg_data, retry);
			break;
		}
		usleep_range(10 * 1000, 10 * 1000);
	}

	if (retry <= 0)
		input_info(true, ilits->dev, "%s check 0x73010 error read 0x%X\n", __func__, reg_data);

	if (ili_ice_mode_write(0x73000, packet, 4) < 0)
		input_err(true, ilits->dev, "%s Write %x at 0x73000\n", __func__, packet);
}

static u32 ilitek_tddi_ic_rd_pack(int packet)
{
	int retry = 100;
	u32 reg_data = 0;

	ilitek_tddi_ic_wr_pack(packet);

	while (retry--) {
		if (ili_ice_mode_read(0x4800A, &reg_data, sizeof(u8)) < 0)
			input_err(true, ilits->dev, "%s Read 0x4800A error\n", __func__);

		if ((reg_data & 0x02) == 0x02) {
			input_info(true, ilits->dev, "%s check  ok 0x4800A read 0x%X retry = %d\n",
				__func__, reg_data, retry);
			break;
		}
		usleep_range(10 * 1000, 10 * 1000);
	}
	if (retry <= 0)
		input_info(true, ilits->dev, "%s check 0x4800A error read 0x%X\n", __func__, reg_data);

	if (ili_ice_mode_write(0x4800A, 0x02, 1) < 0)
		input_err(true, ilits->dev, "%s Write 0x2 at 0x4800A\n", __func__);

	if (ili_ice_mode_read(0x73016, &reg_data, sizeof(u8)) < 0)
		input_err(true, ilits->dev, "%s Read 0x73016 error\n", __func__);

	return reg_data;
}

void ili_ic_set_ddi_reg_onepage(u8 page, u8 reg, u8 data)
{
	u32 setpage = 0x1FFFFF00 | page;
	u32 setreg = 0x1F000100 | (reg << 16) | data;
	bool ice = atomic_read(&ilits->ice_stat);

	input_info(true, ilits->dev, "%s setpage =  0x%X setreg = 0x%X\n", __func__, setpage, setreg);

	if (!ice)
		if (ili_ice_mode_ctrl(ENABLE, OFF) < 0)
			input_err(true, ilits->dev, "%s Enable ice mode failed before writing ddi reg\n", __func__);

	/*TDI_WR_KEY*/
	ilitek_tddi_ic_wr_pack(0x1FFF9527);
	/*Switch to Page*/
	ilitek_tddi_ic_wr_pack(setpage);
	/* Page*/
	ilitek_tddi_ic_wr_pack(setreg);
	/*TDI_WR_KEY OFF*/
	ilitek_tddi_ic_wr_pack(0x1FFF9500);

	if (!ice)
		if (ili_ice_mode_ctrl(DISABLE, OFF) < 0)
			input_err(true, ilits->dev, "%s Disable ice mode failed after writing ddi reg\n", __func__);
}

void ili_ic_get_ddi_reg_onepage(u8 page, u8 reg, u8 *data)
{
	u32 setpage = 0x1FFFFF00 | page;
	u32 setreg = 0x2F000100 | (reg << 16);
	bool ice = atomic_read(&ilits->ice_stat);

	input_info(true, ilits->dev, "%s setpage = 0x%X setreg = 0x%X\n", __func__, setpage, setreg);

	if (!ice)
		if (ili_ice_mode_ctrl(ENABLE, OFF) < 0)
			input_err(true, ilits->dev, "%s Enable ice mode failed before reading ddi reg\n", __func__);

	/*TDI_WR_KEY*/
	ilitek_tddi_ic_wr_pack(0x1FFF9527);
	/*Set Read Page reg*/
	ilitek_tddi_ic_wr_pack(setpage);

	/*TDI_RD_KEY*/
	ilitek_tddi_ic_wr_pack(0x1FFF9487);
	/*( *( __IO uint8 *)	(0x4800A) ) =0x2*/
	if (ili_ice_mode_write(0x4800A, 0x02, 1) < 0)
		input_err(true, ilits->dev, "%s Write 0x2 at 0x4800A\n", __func__);

	*data = ilitek_tddi_ic_rd_pack(setreg);
	input_info(true, ilits->dev, "%s check page = 0x%X, reg = 0x%X, read 0x%X\n", __func__, page, reg, *data);

	/*TDI_RD_KEY OFF*/
	ilitek_tddi_ic_wr_pack(0x1FFF9400);
	/*TDI_WR_KEY OFF*/
	ilitek_tddi_ic_wr_pack(0x1FFF9500);

	if (!ice)
		if (ili_ice_mode_ctrl(DISABLE, OFF) < 0)
			input_err(true, ilits->dev, "%s Disable ice mode failed after reading ddi reg\n", __func__);
}


void ili_ic_get_pc_counter(int stat)
{
	bool ice = atomic_read(&ilits->ice_stat);
	u32 pc = 0, pc_addr = ilits->chip->pc_counter_addr;
	u32 latch = 0, latch_addr = ilits->chip->pc_latch_addr;
	int ret = 0;

	ILI_DBG("%s stat = %d\n", __func__, stat);

	if (!ice) {
		if (stat == DO_SPI_RECOVER || stat == DO_I2C_RECOVER)
			ret = ili_ice_mode_ctrl(ENABLE, OFF);
		else
			ret = ili_ice_mode_ctrl(ENABLE, ON);

		if (ret < 0)
			input_err(true, ilits->dev, "%s Enable ice mode failed while reading pc counter\n", __func__);
	}

	if (ili_ice_mode_read(ilits->chip->pc_counter_addr, &pc, sizeof(u32)) < 0)
		input_err(true, ilits->dev, "%s Read pc conter error\n", __func__);

	if (ili_ice_mode_read(ilits->chip->pc_latch_addr, &latch, sizeof(u32)) < 0)
		input_err(true, ilits->dev, "%s Read pc latch error\n", __func__);

	ilits->fw_pc = pc;
	ilits->fw_latch = latch;
	input_err(true, ilits->dev, "%s Read counter (addr: 0x%x) = 0x%x, latch (addr: 0x%x) = 0x%x\n",
		__func__, pc_addr, ilits->fw_pc, latch_addr, ilits->fw_latch);

	/* Avoid screen abnormal. */
	if (stat == DO_SPI_RECOVER) {
		atomic_set(&ilits->ice_stat, DISABLE);
		return;
	}

	if (!ice) {
		if (ili_ice_mode_ctrl(DISABLE, ON) < 0)
			input_err(true, ilits->dev, "%s Disable ice mode failed while reading pc counter\n", __func__);
	}
}

int ili_ic_int_trigger_ctrl(bool pulse)
{
	/* It's supported by fw, and the level will be kept at high until data was already prepared. */
	if (ili_ic_func_ctrl("int_trigger", pulse) < 0) {
		input_err(true, ilits->dev, "%s Write CMD error, set back to <%s> trigger\n",
			__func__, ilits->int_pulse ? "Level" : "Pulse");
		return -1;
	}

	ilits->int_pulse = pulse;
	input_info(true, ilits->dev, "%s INT Trigger = %s\n", __func__, ilits->int_pulse ? "Level" : "Pulse");
	return 0;
}

int ili_ic_check_int_level(bool level)
{
	int timer = 3000;
	int gpio = IRQ_GPIO_NUM; //ilits->tp_int

	/*
	 * If callers have a trouble to use the gpio that is passed by vendors,
	 * please utlises a physical gpio number instead or call a help from them.
	 */

	while (--timer > 0) {
		if (level) {
			if (gpio_get_value(gpio)) {
				ILI_DBG("%s INT high detected.\n", __func__);
				return 0;
			}
		} else {
			if (!gpio_get_value(gpio)) {
				ILI_DBG("%s INT low detected.\n", __func__);
				return 0;
			}
		}
		usleep_range(1 * 1000, 1 * 1000);
	}
	input_err(true, ilits->dev, "%s Error! INT level no detected.\n", __func__);
	return -1;
}

int ili_ic_check_int_pulse(bool pulse)
{
	if (!wait_event_interruptible_timeout(ilits->inq, !atomic_read(&ilits->cmd_int_check),
			msecs_to_jiffies(ilits->wait_int_timeout))) {
		input_err(true, ilits->dev, "%s Error! INT pulse no detected. Timeout = %d ms\n",
			__func__, ilits->wait_int_timeout);
		atomic_set(&ilits->cmd_int_check, DISABLE);
		return -1;
	}
	ILI_DBG("%s INT pulse detected.\n", __func__);
	return 0;
}

int ili_ic_check_busy(int count, int delay)
{
	u8 cmd[2] = {0};
	u8 busy = 0, rby = 0;

	cmd[0] = P5_X_READ_DATA_CTRL;
	cmd[1] = P5_X_CDC_BUSY_STATE;

	if (ilits->actual_tp_mode == P5_X_FW_AP_MODE)
		rby = 0x41;
	else if (ilits->actual_tp_mode == P5_X_FW_TEST_MODE)
		rby = 0x51;
	else {
		input_err(true, ilits->dev, "%s Unknown TP mode (0x%x)\n", __func__, ilits->actual_tp_mode);
		return -EINVAL;
	}

	input_info(true, ilits->dev, "%s read byte = %x, delay = %d\n", __func__, rby, delay);

	do {
		if (ilits->wrapper(cmd, sizeof(cmd), NULL, 0, OFF, OFF) < 0)
			input_err(true, ilits->dev, "%s Write check busy cmd failed\n", __func__);

		if (ilits->wrapper(&cmd[1], sizeof(u8), &busy, sizeof(u8), ON, OFF) < 0)
			input_err(true, ilits->dev, "%s Read check busy failed\n", __func__);

		ILI_DBG("%s busy = 0x%x\n", __func__, busy);

		if (busy == rby) {
			input_info(true, ilits->dev, "%s Check busy free\n", __func__);
			return 0;
		}

		msleep(delay);
	} while (--count > 0);

	input_err(true, ilits->dev, "%s Check busy (0x%x) timeout !\n", __func__, busy);
	ili_ic_get_pc_counter(0);
	return -1;
}

int ili_ic_get_project_id(u8 *pdata, int size)
{
	int i;
	u32 tmp;
	bool ice = atomic_read(&ilits->ice_stat);

	if (!pdata) {
		input_err(true, ilits->dev, "%s pdata is null\n", __func__);
		return -ENOMEM;
	}

	input_info(true, ilits->dev, "%s Read size = %d\n", __func__, size);

	if (!ice)
		if (ili_ice_mode_ctrl(ENABLE, OFF) < 0)
			input_err(true, ilits->dev, "%s Enable ice mode failed while reading project id\n", __func__);

	if (ili_ice_mode_write(0x041000, 0x0, 1) < 0)
		input_err(true, ilits->dev, "%s Pull cs low failed\n", __func__);
	if (ili_ice_mode_write(0x041004, 0x66aa55, 3) < 0)
		input_err(true, ilits->dev, "%s Write key failed\n", __func__);

	if (ili_ice_mode_write(0x041008, 0x03, 1) < 0)
		input_err(true, ilits->dev, "%s Write 0x03 at 0x041008\n", __func__);

	if (ili_ice_mode_write(0x041008, (RSV_BK_ST_ADDR & 0xFF0000) >> 16, 1) < 0)
		input_err(true, ilits->dev, "%s Write address failed\n", __func__);
	if (ili_ice_mode_write(0x041008, (RSV_BK_ST_ADDR & 0x00FF00) >> 8, 1) < 0)
		input_err(true, ilits->dev, "%s Write address failed\n", __func__);
	if (ili_ice_mode_write(0x041008, (RSV_BK_ST_ADDR & 0x0000FF), 1) < 0)
		input_err(true, ilits->dev, "%s Write address failed\n", __func__);

	for (i = 0; i < size; i++) {
		if (ili_ice_mode_write(0x041008, 0xFF, 1) < 0)
			input_err(true, ilits->dev, "%s Write dummy failed\n", __func__);
		if (ili_ice_mode_read(0x41010, &tmp, sizeof(u8)) < 0)
			input_err(true, ilits->dev, "%s Read project id error\n", __func__);
		pdata[i] = tmp;
		input_info(true, ilits->dev, "%s project_id[%d] = 0x%x\n", __func__, i, pdata[i]);
	}

	ili_flash_clear_dma();

	if (ili_ice_mode_write(0x041000, 0x1, 1) < 0)
		input_err(true, ilits->dev, "%s Pull cs high\n", __func__);

	if (!ice)
		if (ili_ice_mode_ctrl(DISABLE, OFF) < 0)
			input_err(true, ilits->dev, "%s Disable ice mode failed while reading project id\n", __func__);

	return 0;
}

int ili_ic_get_core_ver(void)
{
	int i = 0, ret = 0;
	u8 cmd[2] = {0}, buf[10] = {0};

	ilits->protocol->core_ver_len = P5_X_CORE_VER_FOUR_LENGTH;

	if (ilits->info_from_hex) {
		buf[1] = ilits->fw_info[68];
		buf[2] = ilits->fw_info[69];
		buf[3] = ilits->fw_info[70];
		buf[4] = ilits->fw_info[71];
		goto out;
	}

	do {
		if (i == 0) {
			cmd[0] = P5_X_READ_DATA_CTRL;
			cmd[1] = P5_X_GET_CORE_VERSION_NEW;
		} else {
			cmd[0] = P5_X_READ_DATA_CTRL;
			cmd[1] = P5_X_GET_CORE_VERSION;
			ilits->protocol->core_ver_len = P5_X_CORE_VER_THREE_LENGTH;
		}

		if (ilits->wrapper(cmd, sizeof(cmd), NULL, 0, OFF, OFF) < 0)
			input_err(true, ilits->dev, "%s Write core ver cmd failed\n", __func__);

		if (ilits->wrapper(&cmd[1], sizeof(u8), buf, ilits->protocol->core_ver_len, ON, OFF) < 0)
			input_err(true, ilits->dev, "%s Write core ver (0x%x) failed\n", __func__, cmd[1]);

		ILI_DBG("%s header = 0x%x\n", __func__, buf[0]);

		if (buf[0] == P5_X_GET_CORE_VERSION ||
			buf[0] == P5_X_GET_CORE_VERSION_NEW)
			break;

	} while (++i < 2);

	if (buf[0] == P5_X_GET_CORE_VERSION)
		buf[4] = 0;

	if (i >= 2) {
		input_err(true, ilits->dev, "%s Invalid header (0x%x)\n", __func__, buf[0]);
		ret = -EINVAL;
	}

out:
	input_info(true, ilits->dev, "%s Core version = %d.%d.%d.%d\n",
		__func__, buf[1], buf[2], buf[3], buf[4]);
	ilits->chip->core_ver = buf[1] << 24 | buf[2] << 16 | buf[3] << 8 | buf[4];
	return ret;
}

void ili_fw_uart_ctrl(u8 ctrl)
{
	u8 cmd[4] = {0};

	if (ctrl > 1) {
		input_info(true, ilits->dev, "%s Unknown cmd, ignore\n", __func__);
		return;
	}

	input_info(true, ilits->dev, "%s %s UART mode\n", __func__, ctrl ? "Enable" : "Disable");

	cmd[0] = P5_X_I2C_UART;
	cmd[1] = 0x3;
	cmd[2] = 0;
	cmd[3] = ctrl;

	if (ilits->wrapper(cmd, sizeof(cmd), NULL, 0, OFF, OFF) < 0) {
		input_info(true, ilits->dev, "%s Write fw uart cmd failed\n", __func__);
		return;
	}
}

int ili_ic_get_fw_ver(void)
{
	int ret = 0;
	u8 cmd[2] = {0};
	u8 buf[10] = {0};

	if (ilits->info_from_hex) {
		buf[1] = ilits->fw_info[48];
		buf[2] = ilits->fw_info[49];
		buf[3] = ilits->fw_info[50];
		buf[4] = ilits->fw_info[51];
		buf[5] = ilits->fw_mp_ver[0];
		buf[6] = ilits->fw_mp_ver[1];
		buf[7] = ilits->fw_mp_ver[2];
		buf[8] = ilits->fw_mp_ver[3];
		goto out;
	}

	cmd[0] = P5_X_READ_DATA_CTRL;
	cmd[1] = P5_X_GET_FW_VERSION;

	if (ilits->wrapper(cmd, sizeof(cmd), NULL, 0, OFF, OFF) < 0) {
		input_err(true, ilits->dev, "%s Write pre cmd failed\n", __func__);
		ret = -EINVAL;
		goto out;

	}

	if (ilits->wrapper(&cmd[1], sizeof(u8), buf, ilits->protocol->fw_ver_len, ON, OFF)) {
		input_err(true, ilits->dev, "%s Write fw version cmd failed\n", __func__);
		ret = -EINVAL;
		goto out;
	}

	if (buf[0] != P5_X_GET_FW_VERSION) {
		input_err(true, ilits->dev, "%s Invalid firmware ver\n", __func__);
		ret = -1;
	}

out:
	input_info(true, ilits->dev, "%s Firmware version = %d.%d.%d.%d\n", __func__, buf[1], buf[2], buf[3], buf[4]);
	input_info(true, ilits->dev, "%s Firmware MP version = %d.%d.%d.%d\n",
		__func__, buf[5], buf[6], buf[7], buf[8]);
	ilits->chip->fw_ver = buf[1] << 24 | buf[2] << 16 | buf[3] << 8 | buf[4];
	ilits->chip->fw_mp_ver = buf[5] << 24 | buf[6] << 16 | buf[7] << 8 | buf[8];
	return ret;
}

int ili_ic_get_panel_info(void)
{
	int ret = 0;
	u8 cmd = P5_X_GET_PANEL_INFORMATION;
	u8 buf[10] = {0};
	u8 len = ilits->protocol->panel_info_len;

	if (ilits->info_from_hex && (ilits->chip->core_ver >= CORE_VER_1410)) {
		buf[1] = ilits->fw_info[16];
		buf[2] = ilits->fw_info[17];
		buf[3] = ilits->fw_info[18];
		buf[4] = ilits->fw_info[19];
		ilits->panel_wid = buf[2] << 8 | buf[1];
		ilits->panel_hei = buf[4] << 8 | buf[3];
		ilits->trans_xy = (ilits->chip->core_ver >= CORE_VER_1430
			&& (ilits->rib.nReportByPixel > 0)) ? ON : OFF;
		goto out;
	}

	len = (ilits->chip->core_ver >= CORE_VER_1430) ? 6 : len;

	ret = ilits->wrapper(&cmd, sizeof(cmd), buf, len, ON, OFF);
	if (ret < 0)
		input_err(true, ilits->dev, "%s Read panel info error\n", __func__);

	if (buf[0] != cmd) {
		input_info(true, ilits->dev, "%s Invalid panel info, use default resolution\n", __func__);
		ilits->panel_wid = TOUCH_SCREEN_X_MAX;
		ilits->panel_hei = TOUCH_SCREEN_Y_MAX;
		ilits->trans_xy = OFF;
	} else {
		ilits->panel_wid = buf[1] << 8 | buf[2];
		ilits->panel_hei = buf[3] << 8 | buf[4];
		ilits->trans_xy = (ilits->chip->core_ver >= CORE_VER_1430) ? buf[5] : OFF;
	}

	cmd = P5_X_GET_LP_DUMP_STATUE;
	ilits->wrapper(&cmd, sizeof(cmd), buf, 8, ON, OFF);
	if (ret < 0)
		input_err(true, ilits->dev, "Read lp dump status error\n");

	if (buf[0] != cmd)
		ilits->lp_dump_enable = false;
	else
		ilits->lp_dump_enable = (((buf[7] & BIT(0)) == 0) ? true : false);

out:
	input_info(true, ilits->dev, "%s Panel info: width = %d, height = %d\n",
		__func__, ilits->panel_wid, ilits->panel_hei);
	input_info(true, ilits->dev, "%s Transfer touch coordinate = %s\n",
		__func__, ilits->trans_xy ? "ON" : "OFF");
	input_info(true, ilits->dev, "%s : LP dump %s\n", __func__, ilits->lp_dump_enable ? "ON" : "OFF");

	return ret;
}

int ili_ic_get_tp_info(void)
{
	int ret = 0;
	u8 cmd[2] = {0};
	u8 buf[20] = {0};

	if (ilits->info_from_hex  && (ilits->chip->core_ver >= CORE_VER_1410)) {
		buf[1] = ilits->fw_info[5];
		buf[2] = ilits->fw_info[7];
		buf[3] = ilits->fw_info[8];
		buf[4] = ilits->fw_info[9];
		buf[5] = ilits->fw_info[10];
		buf[6] = ilits->fw_info[11];
		buf[7] = ilits->fw_info[12];
		buf[8] = ilits->fw_info[14];
		buf[11] = buf[7];
		buf[12] = buf[8];
		goto out;
	}

	cmd[0] = P5_X_READ_DATA_CTRL;
	cmd[1] = P5_X_GET_TP_INFORMATION;

	if (ilits->wrapper(cmd, sizeof(cmd), NULL, 0, OFF, OFF) < 0) {
		input_err(true, ilits->dev, "%s Write tp info pre cmd failed\n", __func__);
		ret = -EINVAL;
		goto out;

	}

	ret = ilits->wrapper(&cmd[1], sizeof(u8), buf, ilits->protocol->tp_info_len, ON, OFF);
	if (ret < 0) {
		input_err(true, ilits->dev, "%s Read tp info error\n", __func__);
		goto out;
	}

	if (buf[0] != P5_X_GET_TP_INFORMATION) {
		input_err(true, ilits->dev, "%s Invalid tp info\n", __func__);
		ret = -1;
		goto out;
	}

out:
	ilits->min_x = buf[1];
	ilits->min_y = buf[2];
	ilits->max_x = buf[4] << 8 | buf[3];
	ilits->max_y = buf[6] << 8 | buf[5];
	ilits->xch_num = buf[7];
	ilits->ych_num = buf[8];
	ilits->stx = buf[11];
	ilits->srx = buf[12];

	input_info(true, ilits->dev, "%s TP Info: min_x = %d, min_y = %d, max_x = %d, max_y = %d\n",
		__func__, ilits->min_x, ilits->min_y, ilits->max_x, ilits->max_y);
	input_info(true, ilits->dev, "%s TP Info: xch = %d, ych = %d, stx = %d, srx = %d\n",
		__func__, ilits->xch_num, ilits->ych_num, ilits->stx, ilits->srx);

	if (!ilits->print_buf && ilits->ych_num != 0 && ilits->xch_num != 0) {
		input_info(true, ilits->dev, "%s,ych_num:%d,xch_num:%d\n", __func__, ilits->ych_num, ilits->xch_num);
		ilits->print_buf = kzalloc(ilits->ych_num * ilits->xch_num * CMD_RESULT_WORD_LEN, GFP_KERNEL);
		if (!ilits->print_buf) {
			input_err(true, ilits->dev,
					"%s: Failed to alloc memory print_buf\n", __func__);
			ret = -EINVAL;
		}
	} else if (ilits->ych_num == 0 && ilits->xch_num == 0) {
		input_err(true, ilits->dev, "%s: abnormal ch nums\n", __func__);
	}
	return ret;
}

static void ilitek_tddi_ic_check_protocol_ver(u32 pver)
{
	int i = 0;

	if (ilits->protocol->ver == pver) {
		ILI_DBG("%s same procotol version, do nothing\n", __func__);
		return;
	}

	for (i = 0; i < PROTOCL_VER_NUM - 1; i++) {
		if (protocol_info[i].ver == pver) {
			ilits->protocol = &protocol_info[i];
			input_info(true, ilits->dev, "%s update protocol version = %x\n",
				__func__, ilits->protocol->ver);
			return;
		}
	}

	input_err(true, ilits->dev, "%s Not found a correct protocol version in list, use newest version\n", __func__);
	ilits->protocol = &protocol_info[PROTOCL_VER_NUM - 1];
}

int ili_ic_get_protocl_ver(void)
{
	int ret = 0;
	u8 cmd[2] = {0};
	u8 buf[10] = {0};
	u32 ver;

	if (ilits->info_from_hex) {
		buf[1] = ilits->fw_info[72];
		buf[2] = ilits->fw_info[73];
		buf[3] = ilits->fw_info[74];
		goto out;
	}

	cmd[0] = P5_X_READ_DATA_CTRL;
	cmd[1] = P5_X_GET_PROTOCOL_VERSION;

	if (ilits->wrapper(cmd, sizeof(cmd), NULL, 0, OFF, OFF) < 0) {
		input_err(true, ilits->dev, "%s Write protocol ver pre cmd failed\n", __func__);
		ret = -EINVAL;
		goto out;

	}

	if (ilits->wrapper(&cmd[1], sizeof(u8), buf, ilits->protocol->pro_ver_len, ON, OFF)) {
		input_err(true, ilits->dev, "%s Read protocol version error\n", __func__);
		ret = -EINVAL;
		goto out;
	}

	if (buf[0] != P5_X_GET_PROTOCOL_VERSION) {
		input_err(true, ilits->dev, "%s Invalid protocol ver\n", __func__);
		ret = -1;
		goto out;
	}

out:
	ver = buf[1] << 16 | buf[2] << 8 | buf[3];

	ilitek_tddi_ic_check_protocol_ver(ver);

	input_info(true, ilits->dev, "%s Protocol version = %d.%d.%d\n", __func__, ilits->protocol->ver >> 16,
		(ilits->protocol->ver >> 8) & 0xFF, ilits->protocol->ver & 0xFF);
	return ret;
}

int ili_ic_get_info(void)
{
	int ret = 0;

	if (!atomic_read(&ilits->ice_stat)) {
		input_err(true, ilits->dev, "%s ice mode doesn't enable\n", __func__);
		return -1;
	}


	if (ili_ice_mode_read(ilits->chip->pid_addr, &ilits->chip->pid, sizeof(u32)) < 0)
		input_err(true, ilits->dev, "%s Read chip pid error\n", __func__);

	if (ili_ice_mode_read(ilits->chip->otp_addr, &ilits->chip->otp_id, sizeof(u32)) < 0)
		input_err(true, ilits->dev, "%s Read otp id error\n", __func__);
	if (ili_ice_mode_read(ilits->chip->ana_addr, &ilits->chip->ana_id, sizeof(u32)) < 0)
		input_err(true, ilits->dev, "%s Read ana id error\n", __func__);

	ilits->chip->id = ilits->chip->pid >> 16;
	ilits->chip->type = (ilits->chip->pid & 0x0000FF00) >> 8;
	ilits->chip->ver = ilits->chip->pid & 0xFF;
	ilits->chip->otp_id &= 0xFF;
	ilits->chip->ana_id &= 0xFF;

	input_info(true, ilits->dev, "%s CHIP: PID = %x\n", __func__, (ilits->chip->pid >> 8));

	ret = ilitek_tddi_ic_check_support(ilits->chip->pid, ilits->chip->id);
	return ret;
}

int ili_ic_dummy_check(void)
{
	int ret = 0;
	u32 wdata = 0xA55A5AA5;
	u32 rdata = 0;
	int val, val2;

	val = gpio_get_value(ilits->tp_rst);
	val2 = gpio_get_value(ilits->tp_int);
	input_info(true, ilits->dev, "%s gpio_get_val ilits->tp_rst = %d,ilits->tp_int =%d\n", __func__, val, val2);

	if (!atomic_read(&ilits->ice_stat)) {
		input_err(true, ilits->dev, "%s ice mode doesn't enable\n", __func__);
		return -1;
	}

	if (ili_ice_mode_write(WDT9_DUMMY2, wdata, sizeof(u32)) < 0)
		input_err(true, ilits->dev, "%s Write dummy error\n", __func__);


	if (ili_ice_mode_read(WDT9_DUMMY2, &rdata, sizeof(u32)) < 0)
		input_err(true, ilits->dev, "%s Read dummy error\n", __func__);

	if (rdata != wdata) {
		input_err(true, ilits->dev, "%s Dummy check incorrect, rdata = %x wdata = %x\n",
			__func__, rdata, wdata);
		return -1;
	}
	input_info(true, ilits->dev, "%s Ilitek IC check successe\n", __func__);

	return ret;
}

void ili_ic_lpwg_dump_buf_init(void)
{
	ilits->lpwg_dump_buf = devm_kzalloc(&ilits->spi->dev, LPWG_DUMP_TOTAL_SIZE, GFP_KERNEL);
	if (ilits->lpwg_dump_buf == NULL) {
		input_err(true, ilits->dev, "kzalloc for lpwg_dump_buf failed!\n");
		return;
	}
	ilits->lpwg_dump_buf_idx = 0;
	input_info(true, ilits->dev, "%s : done\n", __func__);
}

int ili_ic_lpwg_dump_buf_write(u8 *buf)
{
	int i = 0;

	if (ilits->lpwg_dump_buf == NULL) {
		input_err(true, ilits->dev, "%s : kzalloc for lpwg_dump_buf failed!\n", __func__);
		return -1;
	}
//	input_info(true, &ilits->dev, "%s : idx(%d) data (0x%X,0x%X,0x%X,0x%X,0x%X)\n",
//			__func__, ilits->lpwg_dump_buf_idx, buf[0], buf[1], buf[2], buf[3], buf[4]);

	for (i = 0 ; i < LPWG_DUMP_PACKET_SIZE ; i++) {
		ilits->lpwg_dump_buf[ilits->lpwg_dump_buf_idx++] = buf[i];
	}
	if (ilits->lpwg_dump_buf_idx >= LPWG_DUMP_TOTAL_SIZE) {
		input_info(true, ilits->dev, "%s : write end of data buf(%d)!\n",
					__func__, ilits->lpwg_dump_buf_idx);
		ilits->lpwg_dump_buf_idx = 0;
	}
	return 0;
}

void ili_ic_lpwg_get(void)
{
	u8 cmd[3] = {0}, rxbuf[112] = {0};
	int i, num_event = 0, offset = 6;

	if (ilits->lp_dump_enable == false) {
		input_err(true, ilits->dev, "Not support LP dump\n");
		return;
	}

	/* get LPWG commads*/
	cmd[0] = 0x01;
	cmd[1] = 0x0A;
	cmd[2] = 0x13;

	ili_irq_enable();
	if (ilits->wrapper(cmd, 3, rxbuf, 112, ON, OFF) < 0) {
		input_err(true, ilits->dev, "get LPWG info failed\n");
		return;
	}
	ili_irq_disable();

	/* number of event */
	num_event = rxbuf[5];
	input_info(true, ilits->dev, "LPWG event number = %d\n", num_event);

	for (i = 0; i < num_event; i++) {
		ili_ic_lpwg_dump_buf_write(&rxbuf[offset + i * 5]);
//		input_info(true, ilits->dev, "%x %x %x %x %x\n",
//						rxbuf[offset + i * 5], rxbuf[offset + i * 5 + 1], rxbuf[offset + i * 5 + 2],
//						rxbuf[offset + i * 5 + 3], rxbuf[offset + i * 5 + 4]);
	}
}

int ili_ic_lpwg_dump_buf_read(u8 *buf)
{
	u8 read_buf[30] = { 0 };
	int read_packet_cnt;
	int start_idx;
	int i;

	if (ilits->lpwg_dump_buf == NULL) {
		input_err(true, ilits->dev, "%s : kzalloc for lpwg_dump_buf failed!\n", __func__);
		return 0;
	}

	if (ilits->lpwg_dump_buf[ilits->lpwg_dump_buf_idx] == 0
		&& ilits->lpwg_dump_buf[ilits->lpwg_dump_buf_idx + 1] == 0
		&& ilits->lpwg_dump_buf[ilits->lpwg_dump_buf_idx + 2] == 0) {
		start_idx = 0;
		read_packet_cnt = ilits->lpwg_dump_buf_idx / LPWG_DUMP_PACKET_SIZE;
	} else {
		start_idx = ilits->lpwg_dump_buf_idx;
		read_packet_cnt = LPWG_DUMP_TOTAL_SIZE / LPWG_DUMP_PACKET_SIZE;
	}

	input_info(true, ilits->dev, "%s : lpwg_dump_buf_idx(%d), start_idx (%d), read_packet_cnt(%d)\n",
				__func__, ilits->lpwg_dump_buf_idx, start_idx, read_packet_cnt);

	for (i = 0 ; i < read_packet_cnt ; i++) {
		memset(read_buf, 0x00, 30);
		snprintf(read_buf, 30, "%03d : %02X%02X%02X%02X%02X\n",
					i, ilits->lpwg_dump_buf[start_idx + 0], ilits->lpwg_dump_buf[start_idx + 1],
					ilits->lpwg_dump_buf[start_idx + 2], ilits->lpwg_dump_buf[start_idx + 3],
					ilits->lpwg_dump_buf[start_idx + 4]);

//		input_info(true, &ilits->dev, "%s : %s\n", __func__, read_buf);
		strlcat(buf, read_buf, PAGE_SIZE);

		if (start_idx + LPWG_DUMP_PACKET_SIZE >= LPWG_DUMP_TOTAL_SIZE) {
			start_idx = 0;
		} else {
			start_idx += 5;
		}
	}

	return 0;
}

static struct ilitek_ic_info chip;

void ili_ic_init(void)
{
	chip.pid_addr =			TDDI_PID_ADDR;
	chip.pc_counter_addr =		TDDI_PC_COUNTER_ADDR;
	chip.pc_latch_addr =		TDDI_PC_LATCH_ADDR;
	chip.otp_addr =			TDDI_OTP_ID_ADDR;
	chip.ana_addr =			TDDI_ANA_ID_ADDR;
	chip.reset_addr =		TDDI_CHIP_RESET_ADDR;

	ilits->protocol = &protocol_info[PROTOCL_VER_NUM - 1];
	ilits->chip = &chip;
}

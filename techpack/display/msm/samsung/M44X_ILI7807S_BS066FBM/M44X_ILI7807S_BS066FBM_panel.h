/*
 * =================================================================
 *
 *
 *	Description:  samsung display panel file
 *	Company:  Samsung Electronics
 *
 * ================================================================
 */
/*
<one line to give the program's name and a brief idea of what it does.>
Copyright (C) 2023, Samsung Electronics. All rights reserved.

*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
*/
#ifndef _M44X_ILI7807S_BS066FBM_H_
#define _M44X_ILI7807S_BS066FBM_H_

#include "ss_dsi_panel_common.h"
#if defined(CONFIG_PANEL_BUILTIN_BACKLIGHT)
#include "../BLIC/ss_blic_s2dps01a01_i2c.h"
#else 
/* If module backlight */
int ss_blic_s2dps01a01_configure(u8 data[][2], int size);
#endif

#define LCD_DEFAUL_BL_LEVEL 125

#endif

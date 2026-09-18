/*
 * Copyright (c) 2024 Zhangqi Li (@zhangqili)
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * @file color.c
 * @brief RGB/HSV 转换与颜色混合
 */
#include "lighting/color.h"
#include "string.h"

/** @brief RGB 转 HSV（h:0~359, s/v:0~100） */
void rgb_to_hsv(ColorHSV * restrict hsv, const ColorRGB * restrict rgb)
{
	float max;
	float min;
	max=rgb->r;
	min=rgb->r;
	max=rgb->g>max?rgb->g:max;
	min=rgb->g<min?rgb->g:min;
	max=rgb->b>max?rgb->b:max;
	min=rgb->b<min?rgb->b:min;
	if(max==min)
	{
		hsv->h=0;
	}
	else if(max==rgb->r&&rgb->g>=rgb->b)
	{
		hsv->h=60*(rgb->g-rgb->b)/(max-min);
	}
	else if(max==rgb->r && rgb->g<rgb->b)
	{
		hsv->h=60*(rgb->g-rgb->b)/(max-min)+360;
	}
	else if(max==rgb->g)
	{
		hsv->h=60*(rgb->b-rgb->r)/(max-min)+120;
	}
	else if(max==rgb->b)
	{
		hsv->h=60*(rgb->r-rgb->g)/(max-min)+240;
	}
	if(max==0)
	{
		hsv->s=0;
	}
	else
	{
		hsv->s=100*(1-min/max);
	}
	hsv->v=max*100/255;
}

/** @brief HSV 转 RGB（8bit 通道） */
void hsv_to_rgb(ColorRGB * restrict rgb, const ColorHSV * restrict hsv)
{
    float c = 0;
    float x = 0;
    float y = 0;
    float z = 0;
    float h = (float)(hsv->h);
    float s = ((float)(hsv->s)) / 100.0f;
    float v = ((float)(hsv->v)) / 100.0f;
    if (s < 1e-6f)
    {
        rgb->r=hsv->v*255.0/100.0;
        rgb->g=hsv->v*255.0/100.0;
        rgb->b=hsv->v*255.0/100.0;
    }
    else
    {
        h=h/60;
        c=h-(int)h;

        x = v*(1-s);
        y = v*(1-s*c);
        z = v*(1-s*(1-c));
        switch (hsv->h/60)
        {
            case 0:
                rgb->r = hsv->v*255.0/100.0;
                rgb->g = z*255;
                rgb->b = x*255;
                break;
            case 1:
                rgb->r = y*255;
                rgb->g = hsv->v*255.0/100.0;
                rgb->b = x*255;
                break;
            case 2:
                rgb->r = x*255;
                rgb->g = hsv->v*255.0/100.0;
                rgb->b = z*255;
                break;
            case 3:
                rgb->r = x*255;
                rgb->g = y*255;
                rgb->b = hsv->v*255.0/100.0;
                break;
            case 4:
                rgb->r = z*255;
                rgb->g = x*255;
                rgb->b = hsv->v*255.0/100.0;
                break;
            case 5:
                rgb->r = hsv->v*255.0/100.0;
                rgb->g = x*255;
                rgb->b = y*255;
                break;
            default:
                break;
        }
    }
}

/** @brief Color 拷贝为 RGB 结构 */
void color_get_rgb(const Color* restrict color, ColorRGB* restrict rgb)
{
    memcpy(rgb,color,sizeof(Color));
}

/** @brief 用 RGB 结构写入 Color */
void color_set_rgb(Color* restrict  color, const ColorRGB* restrict rgb)
{
    memcpy(color,rgb,sizeof(Color));
}

/** @brief 浮点 Color 写入 RGB 分量 */
void colorf_set_rgb(ColorFloat * restrict color, const ColorRGB * restrict rgb)
{
	color->r = rgb->r;
	color->g = rgb->g;
	color->b = rgb->b;
}

/** @brief 浮点 Color 由 HSV 设置（经 RGB 中转） */
void colorf_set_hsv(ColorFloat * restrict color, const ColorHSV * restrict rgb)
{
	ColorRGB temp_rgb = {0,0,0};
	hsv_to_rgb(&temp_rgb, rgb);
	colorf_set_rgb(color, &temp_rgb);
}

/** @brief Color 转 HSV */
void color_get_hsv(const Color* restrict color, ColorHSV* restrict hsv)
{
    rgb_to_hsv(hsv, color);
}

/** @brief Color 由 HSV 设置 */
void color_set_hsv(Color* restrict color, const ColorHSV* restrict hsv)
{
    hsv_to_rgb(color, hsv);
}

/** @brief 饱和加法混合 dest += source（各通道封顶 255） */
void color_mix(Color *dest, const Color *source)
{
	uint16_t temp_r = dest->r + source->r;
    uint16_t temp_g = dest->g + source->g;
    uint16_t temp_b = dest->b + source->b;

    dest->r = temp_r > 255 ? 255 : (uint8_t)temp_r;
    dest->g = temp_g > 255 ? 255 : (uint8_t)temp_g;
    dest->b = temp_b > 255 ? 255 : (uint8_t)temp_b;
}

/** @brief 浮点版饱和加法混合 */
void colorf_mix(ColorFloat *dest, const ColorFloat *source)
{
	float temp_r = dest->r + source->r;
    float temp_g = dest->g + source->g;
    float temp_b = dest->b + source->b;

    dest->r = temp_r > 255.f ? 255.f : (uint8_t)temp_r;
    dest->g = temp_g > 255.f ? 255.f : (uint8_t)temp_g;
    dest->b = temp_b > 255.f ? 255.f : (uint8_t)temp_b;
}

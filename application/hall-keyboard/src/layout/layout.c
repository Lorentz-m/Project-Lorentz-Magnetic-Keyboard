#include "layout/layout.h"
#include "device/param_defaults.h"

// 成员index位置对应EN_RGB_LAYOUT_KEY_SLOT
static const st_keyboard_layout_mapping g_keyboard_layout[EN_LAYOUT_KEY_RGB_SLOT_MAX] = {
    /* 第1行 灯带左→右 */
    {EN_KEY_PHY_COORD_LINE1_KEY1,  LINE1_KEY1_DEFAULT,  MUX5_K2_COORD,  {0.0f, 0.0f}},       /* KEY14 */
    {EN_KEY_PHY_COORD_LINE1_KEY2,  LINE1_KEY2_DEFAULT,  MUX5_K1_COORD,  {1.0f, 0.0f}},       /* KEY13 */
    {EN_KEY_PHY_COORD_LINE1_KEY3,  LINE1_KEY3_DEFAULT,  MUX5_K15_COORD, {2.0f, 0.0f}},       /* KEY12 */
    {EN_KEY_PHY_COORD_LINE1_KEY4,  LINE1_KEY4_DEFAULT,  MUX5_K14_COORD, {3.0f, 0.0f}},       /* KEY11 */
    {EN_KEY_PHY_COORD_LINE1_KEY5,  LINE1_KEY5_DEFAULT,  MUX5_K13_COORD, {4.0f, 0.0f}},       /* KEY10 */
    {EN_KEY_PHY_COORD_LINE1_KEY6,  LINE1_KEY6_DEFAULT,  MUX5_K12_COORD, {5.0f, 0.0f}},       /* KEY9 */
    {EN_KEY_PHY_COORD_LINE1_KEY7,  LINE1_KEY7_DEFAULT,  MUX1_K5_COORD,  {6.0f, 0.0f}},       /* KEY8 */
    {EN_KEY_PHY_COORD_LINE1_KEY8,  LINE1_KEY8_DEFAULT,  MUX1_K4_COORD,  {7.0f, 0.0f}},       /* KEY7 */
    {EN_KEY_PHY_COORD_LINE1_KEY9,  LINE1_KEY9_DEFAULT,  MUX1_K2_COORD,  {8.0f, 0.0f}},       /* KEY6 */
    {EN_KEY_PHY_COORD_LINE1_KEY10, LINE1_KEY10_DEFAULT, MUX1_K1_COORD,  {9.0f, 0.0f}},       /* KEY5 */
    {EN_KEY_PHY_COORD_LINE1_KEY11, LINE1_KEY11_DEFAULT, MUX1_K15_COORD, {10.0f, 0.0f}},      /* KEY4 */
    {EN_KEY_PHY_COORD_LINE1_KEY12, LINE1_KEY12_DEFAULT, MUX1_K14_COORD, {11.0f, 0.0f}},      /* KEY3 */
    {EN_KEY_PHY_COORD_LINE1_KEY13, LINE1_KEY13_DEFAULT, MUX1_K13_COORD, {12.0f, 0.0f}},      /* KEY2 */
    {EN_KEY_PHY_COORD_LINE1_KEY14, LINE1_KEY14_DEFAULT, MUX1_K12_COORD, {13.5f, 0.0f}},      /* KEY1 */
    /* 第2行 灯带右→左 */
    {EN_KEY_PHY_COORD_LINE2_KEY14, LINE2_KEY14_DEFAULT, MUX2_K12_COORD, {13.75f, 1.0f}},     /* KEY32 */
    {EN_KEY_PHY_COORD_LINE2_KEY13, LINE2_KEY13_DEFAULT, MUX2_K13_COORD, {12.5f, 1.0f}},      /* KEY31 */
    {EN_KEY_PHY_COORD_LINE2_KEY12, LINE2_KEY12_DEFAULT, MUX2_K14_COORD, {11.5f, 1.0f}},      /* KEY30 */
    {EN_KEY_PHY_COORD_LINE2_KEY11, LINE2_KEY11_DEFAULT, MUX2_K15_COORD, {10.5f, 1.0f}},      /* KEY29 */
    {EN_KEY_PHY_COORD_LINE2_KEY10, LINE2_KEY10_DEFAULT, MUX2_K1_COORD,  {9.5f, 1.0f}},       /* KEY28 */
    {EN_KEY_PHY_COORD_LINE2_KEY9,  LINE2_KEY9_DEFAULT,  MUX2_K2_COORD,  {8.5f, 1.0f}},       /* KEY27 */
    {EN_KEY_PHY_COORD_LINE2_KEY8,  LINE2_KEY8_DEFAULT,  MUX2_K4_COORD,  {7.5f, 1.0f}},       /* KEY26 */
    {EN_KEY_PHY_COORD_LINE2_KEY7,  LINE2_KEY7_DEFAULT,  MUX2_K5_COORD,  {6.5f, 1.0f}},       /* KEY25 */
    {EN_KEY_PHY_COORD_LINE2_KEY6,  LINE2_KEY6_DEFAULT,  MUX6_K4_COORD,  {5.5f, 1.0f}},       /* KEY19 */
    {EN_KEY_PHY_COORD_LINE2_KEY5,  LINE2_KEY5_DEFAULT,  MUX6_K5_COORD,  {4.5f, 1.0f}},       /* KEY20 */
    {EN_KEY_PHY_COORD_LINE2_KEY4,  LINE2_KEY4_DEFAULT,  MUX6_K2_COORD,  {3.5f, 1.0f}},       /* KEY18 */
    {EN_KEY_PHY_COORD_LINE2_KEY3,  LINE2_KEY3_DEFAULT,  MUX6_K1_COORD,  {2.5f, 1.0f}},       /* KEY17 */
    {EN_KEY_PHY_COORD_LINE2_KEY2,  LINE2_KEY2_DEFAULT,  MUX5_K4_COORD,  {1.5f, 1.0f}},       /* KEY15 */
    {EN_KEY_PHY_COORD_LINE2_KEY1,  LINE2_KEY1_DEFAULT,  MUX5_K5_COORD,  {0.25f, 1.0f}},      /* KEY16 */
    /* 第3行 灯带左→右 */
    {EN_KEY_PHY_COORD_LINE3_KEY1,  LINE3_KEY1_DEFAULT,  MUX7_K15_COORD, {0.3f, 2.0f}},       /* KEY44 */
    {EN_KEY_PHY_COORD_LINE3_KEY2,  LINE3_KEY2_DEFAULT,  MUX6_K15_COORD, {1.7f, 2.0f}},       /* KEY21 */
    {EN_KEY_PHY_COORD_LINE3_KEY3,  LINE3_KEY3_DEFAULT,  MUX6_K14_COORD, {2.7f, 2.0f}},       /* KEY22 */
    {EN_KEY_PHY_COORD_LINE3_KEY4,  LINE3_KEY4_DEFAULT,  MUX6_K13_COORD, {3.7f, 2.0f}},       /* KEY23 */
    {EN_KEY_PHY_COORD_LINE3_KEY5,  LINE3_KEY5_DEFAULT,  MUX6_K12_COORD, {4.7f, 2.0f}},       /* KEY24 */
    {EN_KEY_PHY_COORD_LINE3_KEY6,  LINE3_KEY6_DEFAULT,  MUX3_K5_COORD,  {5.7f, 2.0f}},       /* KEY40 */
    {EN_KEY_PHY_COORD_LINE3_KEY7,  LINE3_KEY7_DEFAULT,  MUX3_K4_COORD,  {6.7f, 2.0f}},       /* KEY39 */
    {EN_KEY_PHY_COORD_LINE3_KEY8,  LINE3_KEY8_DEFAULT,  MUX3_K2_COORD,  {7.7f, 2.0f}},       /* KEY38 */
    {EN_KEY_PHY_COORD_LINE3_KEY9,  LINE3_KEY9_DEFAULT,  MUX3_K1_COORD,  {8.7f, 2.0f}},       /* KEY37 */
    {EN_KEY_PHY_COORD_LINE3_KEY10, LINE3_KEY10_DEFAULT, MUX3_K15_COORD, {9.7f, 2.0f}},       /* KEY36 */
    {EN_KEY_PHY_COORD_LINE3_KEY11, LINE3_KEY11_DEFAULT, MUX3_K14_COORD, {10.7f, 2.0f}},      /* KEY35 */
    {EN_KEY_PHY_COORD_LINE3_KEY12, LINE3_KEY12_DEFAULT, MUX3_K13_COORD, {11.7f, 2.0f}},      /* KEY34 */
    {EN_KEY_PHY_COORD_LINE3_KEY13, LINE3_KEY13_DEFAULT, MUX3_K12_COORD, {13.3f, 2.0f}},      /* KEY33 */
    /* 第4行 灯带右→左 */
    {EN_KEY_PHY_COORD_LINE4_KEY14, LINE4_KEY14_DEFAULT, MUX4_K12_COORD, {14.0f, 3.0f}},      /* KEY61 */
    {EN_KEY_PHY_COORD_LINE4_KEY13, LINE4_KEY13_DEFAULT, MUX4_K14_COORD, {13.0f, 3.0f}},      /* KEY59 */
    {EN_KEY_PHY_COORD_LINE4_KEY12, LINE4_KEY12_DEFAULT, MUX4_K15_COORD, {12.0f, 3.0f}},      /* KEY58 */
    {EN_KEY_PHY_COORD_LINE4_KEY11, LINE4_KEY11_DEFAULT, MUX4_K1_COORD,  {11.0f, 3.0f}},      /* KEY57 */
    {EN_KEY_PHY_COORD_LINE4_KEY10, LINE4_KEY10_DEFAULT, MUX4_K2_COORD,  {10.0f, 3.0f}},      /* KEY56 */
    {EN_KEY_PHY_COORD_LINE4_KEY9,  LINE4_KEY9_DEFAULT,  MUX4_K4_COORD,  {9.0f, 3.0f}},       /* KEY55 */
    {EN_KEY_PHY_COORD_LINE4_KEY8,  LINE4_KEY8_DEFAULT,  MUX4_K5_COORD,  {8.0f, 3.0f}},       /* KEY54 */
    {EN_KEY_PHY_COORD_LINE4_KEY7,  LINE4_KEY7_DEFAULT,  MUX7_K1_COORD,  {7.0f, 3.0f}},       /* KEY45 */
    {EN_KEY_PHY_COORD_LINE4_KEY6,  LINE4_KEY6_DEFAULT,  MUX7_K2_COORD,  {6.0f, 3.0f}},       /* KEY46 */
    {EN_KEY_PHY_COORD_LINE4_KEY5,  LINE4_KEY5_DEFAULT,  MUX7_K4_COORD,  {5.0f, 3.0f}},       /* KEY47 */
    {EN_KEY_PHY_COORD_LINE4_KEY4,  LINE4_KEY4_DEFAULT,  MUX7_K5_COORD,  {4.0f, 3.0f}},       /* KEY48 */
    {EN_KEY_PHY_COORD_LINE4_KEY3,  LINE4_KEY3_DEFAULT,  MUX7_K12_COORD, {3.0f, 3.0f}},       /* KEY41 */
    {EN_KEY_PHY_COORD_LINE4_KEY2,  LINE4_KEY2_DEFAULT,  MUX7_K13_COORD, {2.0f, 3.0f}},       /* KEY42 */
    {EN_KEY_PHY_COORD_LINE4_KEY1,  LINE4_KEY1_DEFAULT,  MUX7_K14_COORD, {0.5f, 3.0f}},       /* KEY43 */
    /* 第5行 灯带左→右（空格占5灯，均为 KEY64） */
    {EN_KEY_PHY_COORD_LINE5_KEY1,  LINE5_KEY1_DEFAULT,  MUX8_K15_COORD, {0.1f, 4.0f}},       /* KEY51 */
    {EN_KEY_PHY_COORD_LINE5_KEY2,  LINE5_KEY2_DEFAULT,  MUX8_K14_COORD, {1.3f, 4.0f}},       /* KEY52 */
    {EN_KEY_PHY_COORD_LINE5_KEY3,  LINE5_KEY3_DEFAULT,  MUX8_K13_COORD, {2.5f, 4.0f}},       /* KEY53 */
    {EN_KEY_PHY_COORD_LINE5_KEY4,  LINE5_KEY4_DEFAULT,  MUX8_K12_COORD, {4.3f, 4.0f}},       /* KEY64 空格 */
    {EN_KEY_PHY_COORD_LINE5_KEY4,  LINE5_KEY4_DEFAULT,  MUX8_K12_COORD, {5.3f, 4.0f}},       /* KEY64 空格灯珠2 */
    {EN_KEY_PHY_COORD_LINE5_KEY4,  LINE5_KEY4_DEFAULT,  MUX8_K12_COORD, {6.3f, 4.0f}},       /* KEY64 空格灯珠3 */
    {EN_KEY_PHY_COORD_LINE5_KEY4,  LINE5_KEY4_DEFAULT,  MUX8_K12_COORD, {7.3f, 4.0f}},       /* KEY64 空格灯珠4 */
    {EN_KEY_PHY_COORD_LINE5_KEY4,  LINE5_KEY4_DEFAULT,  MUX8_K12_COORD, {8.3f, 4.0f}},       /* KEY64 空格灯珠5 */
    {EN_KEY_PHY_COORD_LINE5_KEY5,  LINE5_KEY5_DEFAULT,  MUX8_K5_COORD,  {10.0f, 4.0f}},      /* KEY62 */
    {EN_KEY_PHY_COORD_LINE5_KEY6,  LINE5_KEY6_DEFAULT,  MUX8_K4_COORD,  {11.0f, 4.0f}},      /* KEY49 */
    {EN_KEY_PHY_COORD_LINE5_KEY7,  LINE5_KEY7_DEFAULT,  MUX8_K2_COORD,  {12.0f, 4.0f}},      /* KEY63 */
    {EN_KEY_PHY_COORD_LINE5_KEY8,  LINE5_KEY8_DEFAULT,  MUX8_K1_COORD,  {13.0f, 4.0f}},      /* KEY50 */
    {EN_KEY_PHY_COORD_LINE5_KEY9,  LINE5_KEY9_DEFAULT,  MUX4_K13_COORD, {14.0f, 4.0f}},      /* KEY60 */
};

const st_led_rgb_keyboard_bounds_unit g_rgb_keyboard_bounds_unit = {
    .left = -0.5f,
    .top = -0.5f,
    .right = 14.5f,
    .bottom = 4.5f,
};

const st_keyboard_layout_mapping *KeyboardLayout_GetLayoutPtr(void)
{
    return g_keyboard_layout;
}

uint8_t KeyboardLayout_GetKeyDefaultHid(uint8_t slot)
{
    return g_keyboard_layout[slot].key_default_hid;
}

uint8_t KeyboardLayout_GetKeyDefaultHidByCoord(st_key_coord coord)
{
    for (uint8_t i = 0; i < EN_LAYOUT_KEY_RGB_SLOT_MAX; i++) {
        if (g_keyboard_layout[i].key_idx.adc_value == coord.adc_value &&
            g_keyboard_layout[i].key_idx.adc_channel == coord.adc_channel &&
            g_keyboard_layout[i].key_idx.group == coord.group) {
            return g_keyboard_layout[i].key_default_hid;
        }
    }
    return 0;
}

st_key_coord KeyboardLayout_GetKeyIdx(uint8_t slot)
{
    return g_keyboard_layout[slot].key_idx;
}

st_led_rgb_location_unit KeyboardLayout_GetLocation(uint8_t slot)
{
    return g_keyboard_layout[slot].location;
}

const st_keyboard_layout_mapping* KeyboardLayout_GetEntry(uint8_t slot)
{
    return &g_keyboard_layout[slot];
}

const st_keyboard_layout_mapping* KeyboardLayout_GetHallKeyEntry(uint8_t slot)
{
    uint8_t rgb_slot;

    if (slot >= (uint8_t)EN_LAYOUT_KEY_HALL_SLOT_MAX) {
        return NULL;
    }
    if (slot <= (uint8_t)EN_LAYOUT_KEY_SLOT_LINE5_KEY4) {
        rgb_slot = slot;
    } else {
        rgb_slot = (uint8_t)(slot + LAYOUT_SPACE_LED_DUP_COUNT);
    }
    return KeyboardLayout_GetEntry(rgb_slot);
}

st_key_coord KeyboardLayout_GetCoordByPhy(uint16_t key_coord)
{
    uint8_t i;
    for (i = 0; i < (uint8_t)EN_LAYOUT_KEY_RGB_SLOT_MAX; i++) {
        if ((uint16_t)g_keyboard_layout[i].key_phy_coord == key_coord) {
            return g_keyboard_layout[i].key_idx;
        }
    }
    return (st_key_coord){0xff, 0xff, 0xff};
}

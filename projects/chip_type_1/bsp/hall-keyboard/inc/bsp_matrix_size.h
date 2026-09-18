#ifndef BSP_MATRIX_SIZE_H
#define BSP_MATRIX_SIZE_H

#ifdef __cplusplus
extern "C" {
#endif

/* adcvalue：ADC1..4 槽位序号 */
typedef enum
{
    BSP_ADC_VALUE1 = 0,
    BSP_ADC_VALUE2,
    BSP_ADC_VALUE3,
    BSP_ADC_VALUE4,
    BSP_ADC_VALUE_MAXNUM,
} EN_BSP_ADC_VALUE;

/* adcchannel：每颗霍尔芯片上的通道数 */
typedef enum
{
    BSP_ADC_CHANNEL1 = 0,
    BSP_ADC_CHANNEL2,
    BSP_ADC_CHANNEL_MAXNUM,
} EN_BSP_ADC_CHANNEL;

/* groupvalue：键盘矩阵组索引 */
typedef enum
{
    BSP_GROUP1 = 0,
    BSP_GROUP2,
    BSP_GROUP3,
    BSP_GROUP4,
    BSP_GROUP5,
    BSP_GROUP6,
    BSP_GROUP7,
    BSP_GROUP8,
    BSP_GROUP_MAXNUM,
} EN_BSP_GROUP;

#define BSP_ADC_AVEREGE_NUM  1
#define BSP_ADC_BUFF_SIZE ((unsigned)(BSP_ADC_CHANNEL_MAXNUM) * (unsigned)(BSP_ADC_AVEREGE_NUM))

#ifdef __cplusplus
}
#endif

#endif

#ifndef DEVICE_PARAMS_H
#define DEVICE_PARAMS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device/keyboard_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 设备参数模块初始化
 * @note 系统启动时调用，初始化参数存储、默认配置等全局状态
 */
void DeviceParams_Init(void);

/**
 * @brief 获取按键性能配置参数
 * @param[in]  adc_value    ADC采样值/按键索引
 * @param[in]  adc_channel  ADC通道号
 * @param[in]  group        按键分组号
 * @return 指向性能参数结构体的常量指针（防抖、响应速度等配置）
 */
const st_key_performance_params *DeviceParams_GetPerformanceParams(uint8_t adc_value, uint8_t adc_channel, uint8_t group);

/**
 * @brief 获取键盘参数结构体常量指针
 * @return 指向全局键盘参数结构体的常量指针，生命周期同整个程序
 * @note 供热路径模块在初始化时缓存，避免每次通过通用接口获取参数
 */
const st_keyboard_params *DeviceParams_GetConstParamsPtr(void);

/**
 * @brief 获取按键键值配置参数
 * @param[in]  adc_value    ADC采样值/按键索引
 * @param[in]  adc_channel  ADC通道号
 * @param[in]  group        按键分组号
 * @return 指向键值参数结构体的常量指针（基础层+FN层全量配置）
 */
const st_key_value_params *DeviceParams_GetValueParams(uint8_t adc_value, uint8_t adc_channel, uint8_t group);

/**
 * @brief 写入按键配置参数（带长度安全校验）
 * @param[in]  adc_value      ADC采样值/按键索引
 * @param[in]  adc_channel    ADC通道号
 * @param[in]  group          按键分组号
 * @param[in]  index          参数配置索引类型
 * @param[in]  value           待写入的参数数据指针
 * @param[in]  len            数据长度（单位：字节），必须与目标参数长度严格一致
 * @param[in]  offline_save   true=立即写入Flash，false=仅更新RAM（后续手动调用Flush）
 * @return true  - 参数设置成功
 * @return false - 长度不匹配/参数无效/写入失败
 */
bool DeviceParams_SetKeyParams(uint8_t adc_value, uint8_t adc_channel, uint8_t group,
    EN_KEY_PARAMS_INDEX index, const void *value, size_t len, bool offline_save);

/**
 * @brief 设置当前FN功能模式
 * @param[in]  fn_mode  目标FN模式
 * @note 若当前已存在激活的FN模式，本次设置会直接忽略
 */
void DeviceParams_SetFnMode(EN_FN_MODE fn_mode);

/**
 * @brief 获取当前生效的FN功能模式
 * @return 当前FN模式枚举值
 */
EN_FN_MODE DeviceParams_GetFnMode(void);

/**
 * @brief 校验键值配置是否有效
 * @param[in]  cfg  待校验的键值配置结构体指针
 * @return true  - 配置有效（非空、已配置）
 * @return false - 配置无效（空指针/未配置）
 */
bool DeviceParams_IsKeyValueConfigValid(const st_key_value_config *cfg);

/**
 * @brief 根据当前FN模式，自动获取按键的有效键值
 * @param[in]  key_cfg  按键全量配置参数（包含基础层+所有FN层）
 * @param[out] out_cfg  输出参数：最终生效的键值配置
 * @return true  - 成功匹配到FN层有效配置
 * @return false - 无匹配FN层，自动使用基础层（base）配置
 * @note 核心功能：实现FN按键切换不同键值的逻辑
 */
bool DeviceParams_GetEffectiveKeyValue(const st_key_value_params *key_cfg, st_key_value_config *out_cfg);

/**
 * @brief 设置键盘全局参数（除按键参数外的通用接口）
 * @param[in]  index          参数索引（EN_KEYBOARD_SETTING_INDEX）
 * @param[in]  value          参数值指针（void*类型，支持多种数据类型）
 * @param[in]  len            数据长度（单位：字节），必须与目标参数长度严格一致
 * @param[in]  offline_save   true=立即写入Flash，false=仅更新RAM（后续手动调用Flush）
 * @return true  - 设置成功
 * @return false - 索引无效/长度不匹配/设置失败
 */
bool DeviceParams_SetKeyboardParam(EN_KEYBOARD_SETTING_INDEX index, const void *value, size_t len, bool offline_save);

/**
 * @brief 获取键盘全局参数值（通过指针输出，支持多种数据类型）
 * @param[in]  index  参数索引（EN_KEYBOARD_SETTING_INDEX）
 * @param[out] value  输出缓冲区指针（void*类型）
 * @param[in]  len    缓冲区长度（单位：字节），必须与目标参数长度严格一致
 * @return true  - 获取成功
 * @return false - 索引无效/长度不匹配/缓冲区为空
 */
bool DeviceParams_GetKeyboardParam(EN_KEYBOARD_SETTING_INDEX index, void *value, size_t len);

/**
 * @brief 恢复出厂设置
 */
void DeviceParams_FactoryReset(void);

/**
 * @brief 将当前RAM中的参数一次性写入Flash（批量更新场景）
 * @return true  - 写入成功
 * @return false - 写入失败
 * @note 与 DeviceParams_SetKeyboardParam/SetKeyParams 的 offline_save=false 配合使用，
 *       先多次Set仅更新RAM，最后统一Flush写入Flash
 */
 bool DeviceParams_OfflineSaveParams(void);

/** 按键运行时三维表指针类型（与 key_platform 索引维度一致） */
typedef st_key_runtime_state (*pst_key_runtime_table)[KEY_GROUP_INDEX_2][KEY_GROUP_INDEX_3];

/**
 * @brief 获取按键运行时状态表首地址
 * @return 三维表首地址；供 key_platform / 高级键只读 is_pressed 等
 */
pst_key_runtime_table DeviceParams_GetKeyRuntimeTable(void);

/** @brief 清零全部按键运行时状态 */
void DeviceParams_ResetKeyRuntimeTable(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_PARAMS_H */

# 磁轴键盘固件开发包

本包为USB磁轴键盘产品方案，可在 Windows 上修改键盘固件并编译。可以改 USB 产品名、键位与灯位、灯效、出厂默认参数和串口调试命令；其余功能已包含在静态库中。用 Python 调用本机 ARM 编译器生成固件，再用配套 USB 升级工具写入键盘。

---

## 1. 功能一览

**磁轴采集与触发**

- 全键霍尔扫描，全键扫描率最高 32 kHz、单键扫描率256kHz；RT 精度 0.1 mm。
- 两种触发：普通模式按设定行程按下/抬起；RT 模式按相对锚点快速触发，可配上下死区。
- 单键行程监测，供高级按键使用；开机有静默标定窗口，标定期间不误报按键。

**键值与功能层**

- 普通键盘键、多媒体键、组合键、宏绑定、自定义功能键。
- 基础层 + FN1～FN3，上位机可分层改键。
- 自定义键：进 FN 层、出厂复位、灯效快捷（模式/颜色/亮度/速度/开关）、穿透到另一层键值等。

**高级按键**

双键冲突仲裁、动态行程、行程分段、点按/长按双功能、火力键、回弹触发、迅洁（深度优先）。

**USB连接与上位机**

- 高速USB模式，支持8k轮询率。
- USB 键盘报告、多媒体报告。
- 配套网页驱动。
- 上位机协议：改键、高级功能、按键性能（RT、行程、死区、校准）、灯效、校准开关、霍尔模数查询、恢复默认参数。
- 固件可通过 USB 选择对应 `.bin` 升级。

**宏**

六种执行模式（点击忽略/重开/立即停/完成本次停，按住立即停/完成本次停）。

**灯效**

固定色、循环变色、底座彩虹、底座波浪、触发灯、串灯、渐灭串灯、菱形波纹、渐灭菱形波纹、泡泡；亮度/颜色可调，可总开关；支持按键联动。

**掉电保存**

用户数据分三区：键盘参数（键值、性能、灯效、上报率等）、霍尔标定、宏数据，带校验。

---

## 2. 环境准备

在本机安装：

| 软件 | 说明 |
|------|------|
| Windows | 建议 Windows 10 及以上 |
| Python 3 | 用于运行编译脚本，无需额外 pip 包 |
| Keil MDK | 使用其中的 ARM 编译器 ARMCC v5。`ARMCC\bin` 目录下应有 `Armcc.exe`、`Armlink.exe`、`Fromelf.exe` |

编译器路径写在 `build/build_config.json` 的 `armcc_bin`。默认示例：

```text
D:\Keil5\ARM\ARMCC\bin
```

若本机 Keil 不在该目录，把 `armcc_bin` 改成实际路径后再编译。

---

## 3. 怎么编译

在本包**根目录**打开命令行（PowerShell 或 cmd）：

```text
python build/build.py
```

也可以先进入 `build` 文件夹，再执行：

```text
python build.py
```

### 脚本参数

| 命令 | 作用 |
|------|------|
| `python build/build.py` | 编译。源文件没有改动时，会跳过已经编过的文件 |
| `python build/build.py -c` | 清理后再完整编译一遍 |
| `python build/build.py -v` | 在窗口打印完整的编译、链接命令 |

`-c` 与 `-v` 可以一起用，例如 `python build/build.py -c -v`。

### 编哪些源文件

由 `build/build_config.json` 决定：

- `source_dirs` 下列出的目录（含子目录）里的 `.c` 都会编译。当前是 `application/hall-keyboard/src`。
- 在这些目录里**新增**的 `.c` 会自动编进去，不必改脚本。
- 某个文件不想编时，把路径或文件名写进 `exclude`。

### 编译结果

成功后在本包根目录生成：

```text
Output/firmware/firmware.bin
Output/firmware/firmware.hex
Output/build.log
```

窗口里会看到类似 `compiling xxx.c...`、`linking...`、`Program Size:` 的摘要。完整命令和工具输出在 `Output/build.log`。

---

## 4. 目录说明

```text
Project/
├── README.md                          本说明
├── build/
│   ├── build.py                       编译脚本
│   ├── build_config.json              编译器路径、要编的源码目录
│   └── link.sct                       链接脚本
├── application/hall-keyboard/
│   ├── src/                           可修改的源码
│   │   ├── usb/                       产品名、USB 中断入口
│   │   ├── debug/uart_recv.c          串口命令
│   │   ├── device/param_defaults.c    出厂默认填充
│   │   ├── layout/layout.c            键位、灯位、出厂 HID
│   │   └── lighting/                  灯效
│   └── inc/                           头文件
├── application/common/                头文件（事件、软定时器）
├── projects/chip_type_1/
│   ├── lib/                           静态库（编译时自动链接）
│   └── bsp/hall-keyboard/inc/         板级尺寸等头文件
└── Output/                            编译结果（运行脚本后出现）
```

---

## 5. 改哪里

源文件在 `application/hall-keyboard/src/`，对应头文件在 `inc/`。

### 5.1 USB 产品名

文件：`src/usb/usb_product_name.c`

在 `Customer_UsbProductName_Init` 里改名称，例如：

```c
KeyboardUsb_SetProductName("你的产品名");
```

重新编译并升级后，电脑上显示的设备名会更新（部分系统会缓存旧名称，必要时重新插拔或改一下名称再试）。

`src/usb/usb_irq.c` 是 USB 中断入口，请保留并参与编译。

### 5.2 出厂默认参数

编译期默认值在 `inc/device/param_defaults.h`：

| 宏 | 作用 |
|----|------|
| `KEYBOARD_DATA_CHECK_FLAG` | 参数版本。改了下面各项默认值后，把这个数改成一个新的 32 位值，已出厂的键盘才会丢掉旧数据、用上新默认 |
| `HALL_KEY_RT_MODE_DEFAULT` | 默认是否开启 RT |
| `HALL_KEY_NORMAL_TRAVEL_UM_DEFAULT` | 绝对行程模式触发行程（微米） |
| `HALL_KEY_PRESS_THRESHOLD_MM` / `HALL_KEY_RELEASE_THRESHOLD_MM` | RT 按下/抬起精度（毫米） |
| `HALL_KEY_*_DEAD_ZONE_MM_DEFAULT` | 顶/底死区 |
| `USB_REPORT_RATE_DEFAULT` | USB 上报率（如 `USB_REPORT_RATE_8K`） |
| `LIGHTING_*` / `BRIGHTNESS_DEFAULT` / `SLEEP_TIME_MIN_DEFAULT` | 灯效与睡眠 |
| `LINE*_KEY*_DEFAULT` | 各物理键出厂 HID |

HID 取值见 `inc/device/keyboard_hid_usage.h`（如 `Keyboard_a`、`Keyboard_FN1`）。

填充逻辑在 `src/device/param_defaults.c` 的 `DeviceParams_FillFactoryDefaults`。

只改宏、不升版本时，键盘 Flash 里若已有旧参数，上电仍会用旧数据。需要任选其一：

- 把 `KEYBOARD_DATA_CHECK_FLAG` 改成一个新的 32 位数；或
- 在键盘上做一次恢复出厂。

不要改文件里的 `KB_CFG_STATIC_ASSERT`，也不要改 `device_params.h` 里结构体字段的顺序和大小。

### 5.3 键位、灯位、出厂 HID

文件：`src/layout/layout.c`

表里每一项对应一颗键：物理坐标、出厂 HID、灯珠槽位、灯效用的 `(x, y)`。改配列或灯位时改这张表。出厂 HID 宏与 `param_defaults.h` 中的 `LINE*_KEY*_DEFAULT` 对应。

### 5.4 灯效

源码在 `src/lighting/`，头文件在 `inc/lighting/`。

| 文件 | 改什么 |
|------|--------|
| `lighting.c` | 初始化顺序、上电亮灯延迟 `LIGHTING_BOOT_ON_DELAY_MS` |
| `lighting_mode.h` / `lighting_mode.c` | 用户可见模式枚举与切换；默认模式对应 `param_defaults.h` 的 `LIGHTING_MODE_DEFAULT` |
| `rgb_config.h` | `RGB_MODE_USE_*` / `RGB_BASE_MODE_USE_*`：改为 0 则不编入该模式 |
| `lighting_color.c` | `s_color_table[]` 预设色 |
| `lighting_brightness.c` / `lighting_speed.c` | 亮度、速度步进 |
| `lighting_event.c` | 按键/自定义功能 → 切模式、切色、调亮度 |
| `lighting_key.c` | 行程转灯效强度 |
| `rgb_engine.c` | 各模式渲染 |
| `rgb_layout.c` | 从 `layout.c` 建立灯珠坐标 |
| `color.c` | 颜色运算 |

增加一种用户可见模式时：

1. 在 `rgb_config.h` 打开对应 `RGB_MODE_USE_*`（或在 `rgb_engine.c` 增加渲染）。
2. 在 `EN_LIGHTING_MODE` 增加枚举，并在 `lighting_mode.c` 的切换里接到对应渲染模式。
3. 默认模式改 `LIGHTING_MODE_DEFAULT`，并升高 `KEYBOARD_DATA_CHECK_FLAG`。
4. 模式数量变化时，确认参数里灯效速度数组仍然够用；不要改 `device_params.h` 里结构体字段顺序。

灯珠坐标来自 `layout.c` 的 `(x, y)`。

### 5.5 串口自定义命令

注册表在 `src/debug/uart_recv.c` 的 `s_uart_recv_table`。加一条命令：

```c
static void UartRecv_OnMyCmd(const char *line, uint16_t len)
{
    (void)len;
    /* line 是整行，含前缀；自行解析参数 */
}

static const st_uart_cmd_entry s_uart_recv_table[] = {
    {"DEMO_HELLO", UartRecv_OnDemoHello},
    {"MY_CMD",     UartRecv_OnMyCmd},
};
```

按前缀匹配，最多 `UART_CMD_USER_MAX` 条（默认 16）。

---

## 6. 写入键盘

### 6.1 应用程序烧录

1. 按第 3 节编译，得到 `Output/firmware/firmware.bin`。
2. 将该文件复制为 `usb_iap/payload/firmware.bin`（与 `usb_iap/config.ini` 中的 `bin_path` 一致）。
3. 按下面说明改 `usb_iap/config.ini` 里常改的两项。
4. 键盘用 USB 接到电脑，确认能被识别。
5. 运行 `usb_iap/iap_tool.exe` 写入。

`usb_iap/config.ini` 一般只改这两处，其余保持默认即可：

- USB 名称：`[device]` 段的 `name`。工具用它匹配键盘的 USB 产品名(ProdueName)（不区分大小写，产品名里包含这段即可）。默认 `HALL_KEYBOARD`，对应固件默认名 `Hall_Keyboard`。若按第 5.1 节改过产品名，这里改成新名称里的对应片段，否则工具找不到设备。
- 固件路径：`[payload]` 段的 `bin_path`。相对 `usb_iap/` 目录。默认 `payload/firmware.bin`。若 `.bin` 不放在该位置，把路径改成实际文件。

固件链接起始地址为 `0x0800A000`，大小 `0x36000`。升级工具一般已按此配置，通常不必改。

### 6.2 网页驱动烧录

1. 按第 3 节编译，得到 `Output/firmware/firmware.bin`。
2. 选择网页驱动`更新页`，选择固件 -> 浏览文件 -> 选中`Output/firmware/firmware.bin` -> 点击开始更新

---

## 7. 注意

- 头文件可以按需增加内容，请不要删除现有声明。
- 不要改 `device_params.h`、`keyboard_config.h` 里结构体字段的顺序和大小。
- 若需配套硬件，可联系支持人员。

---

## 8. 联系方式

- QQ：2258243067
- QQ群：971647871

---

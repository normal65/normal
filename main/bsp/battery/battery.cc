/**
 * [初学者导读] 文件：main/bsp/battery/battery.cc
 * 作用：读取电池 ADC 电压，并按分压和放电曲线换算电量百分比。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：电压需要按分压和标定换算，不能直接当百分比。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "battery.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_adc/adc_cali.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_adc/adc_cali_scheme.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_adc/adc_oneshot.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_log.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "config/config.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "Battery";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::bsp::battery {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {
// 固定参数 kAdcChannel = ADC_CHANNEL_5：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr adc_channel_t kAdcChannel = ADC_CHANNEL_5; // GPIO6
// 固定参数 kAdcUnit = ADC_UNIT_1：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr adc_unit_t kAdcUnit = ADC_UNIT_1;
// 固定参数 kAdcAtten = ADC_ATTEN_DB_12：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr adc_atten_t kAdcAtten = ADC_ATTEN_DB_12;
// 固定参数 kAdcWidth = ADC_BITWIDTH_12：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr adc_bitwidth_t kAdcWidth = ADC_BITWIDTH_12;

// 锂电池放电曲线（非线性），电压 (mV) → 电量 (%)
struct VoltPct
{
    // 执行语句：uint32_t mv;；它会更新一次变量、对象或程序状态。
    uint32_t mv;
    // 执行语句：uint8_t pct;；它会更新一次变量、对象或程序状态。
    uint8_t pct;
};

// 固定参数 kNominalFullMv = 4200：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t kNominalFullMv = 4200;
// 固定参数 kNominalEmptyMv = 3000：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t kNominalEmptyMv = 3000;

// 编译期数组：保存电压到百分比的换算曲线。
constexpr VoltPct kDischargeCurve[] = {
    // 执行这条代码：{4200, 100}, {4150, 95}, {4110, 90}, {4080, 85}, {4020, 80}, {3980, 70}, {3950, 60}, {3910, 50},。
    {4200, 100}, {4150, 95}, {4110, 90}, {4080, 85}, {4020, 80}, {3980, 70}, {3950, 60}, {3910, 50},
    // 继续上一行的表达式或参数列表：{3870, 40},  {3830, 30}, {3800, 20}, {3750, 15}, {3700, 10}, {3600, 5},  {3300, 1},  {3000, 0},。
    {3870, 40},  {3830, 30}, {3800, 20}, {3750, 15}, {3700, 10}, {3600, 5},  {3300, 1},  {3000, 0},
};
// 固定参数 kCurveLen = sizeof(kDischargeCurve) / sizeof(kDischargeCurve[0])：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr size_t kCurveLen = sizeof(kDischargeCurve) / sizeof(kDischargeCurve[0]);

// 定义或声明函数 nominal_voltage_to_percent：参数在括号里，返回类型写在函数名前面。
uint8_t nominal_voltage_to_percent(uint32_t mv)
{
    // 判断条件：mv >= kDischargeCurve[0].mv 是否成立。
    if (mv >= kDischargeCurve[0].mv)
        // 返回 100 给调用者，表示当前函数的执行结果。
        return 100;
    // 判断条件：mv <= kDischargeCurve[kCurveLen - 1].mv 是否成立。
    if (mv <= kDischargeCurve[kCurveLen - 1].mv)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < kCurveLen - 1; i++)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto& hi = kDischargeCurve[i];
        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto& lo = kDischargeCurve[i + 1];
        // 判断条件：mv >= lo.mv 是否成立。
        if (mv >= lo.mv)
        {
            // 定义变量 uint32_t pct 并初始化为 lo.pct + (((mv - lo.mv) * (hi.pct - lo.pct)) / (hi.mv - lo.mv))。
            uint32_t pct = lo.pct + (((mv - lo.mv) * (hi.pct - lo.pct)) / (hi.mv - lo.mv));
            // 返回 static_cast<uint8_t>(pct) 给调用者，表示当前函数的执行结果。
            return static_cast<uint8_t>(pct);
        }
    }
    // 返回 0 给调用者，表示当前函数的执行结果。
    return 0;
}

// 定义或声明函数 calibrate_voltage_to_nominal：参数在括号里，返回类型写在函数名前面。
uint32_t calibrate_voltage_to_nominal(uint32_t mv)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t full_mv = config::BATTERY_FULL_MV;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t empty_mv = config::BATTERY_EMPTY_MV;
    // 判断条件：mv >= full_mv 是否成立。
    if (mv >= full_mv)
        // 返回 kNominalFullMv 给调用者，表示当前函数的执行结果。
        return kNominalFullMv;
    // 判断条件：mv <= empty_mv 是否成立。
    if (mv <= empty_mv)
        // 返回 kNominalEmptyMv 给调用者，表示当前函数的执行结果。
        return kNominalEmptyMv;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t span = full_mv - empty_mv;
    // 判断条件：span == 0 是否成立。
    if (span == 0)
        // 返回 kNominalEmptyMv 给调用者，表示当前函数的执行结果。
        return kNominalEmptyMv;
    // 返回 empty_mv + ((mv - empty_mv) * (kNominalFullMv - kNominalEmptyMv)) / span 给调用者，表示当前函数的执行结果。
    return empty_mv + ((mv - empty_mv) * (kNominalFullMv - kNominalEmptyMv)) / span;
}

// 定义或声明函数 voltage_to_percent：参数在括号里，返回类型写在函数名前面。
uint8_t voltage_to_percent(uint32_t mv)
{
    // 判断条件：mv >= config::BATTERY_FULL_MV 是否成立。
    if (mv >= config::BATTERY_FULL_MV)
        // 返回 100 给调用者，表示当前函数的执行结果。
        return 100;
    // 判断条件：mv <= config::BATTERY_EMPTY_MV 是否成立。
    if (mv <= config::BATTERY_EMPTY_MV)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 返回 nominal_voltage_to_percent(calibrate_voltage_to_nominal(mv)) 给调用者，表示当前函数的执行结果。
    return nominal_voltage_to_percent(calibrate_voltage_to_nominal(mv));
}
// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 Battery::Battery：参数在括号里，返回类型写在函数名前面。
Battery::Battery()
    // 定义或声明函数 adc_handle_：参数在括号里，返回类型写在函数名前面。
    : adc_handle_(nullptr)
    // 定义或声明函数 cali_handle_：参数在括号里，返回类型写在函数名前面。
    , cali_handle_(nullptr)
    // 定义或声明函数 initialized_：参数在括号里，返回类型写在函数名前面。
    , initialized_(false)
    // 定义或声明函数 cali_ok_：参数在括号里，返回类型写在函数名前面。
    , cali_ok_(false)
{
}

// 定义或声明函数 Battery::~Battery：参数在括号里，返回类型写在函数名前面。
Battery::~Battery()
{
    // 调用 deinit()：创建和销毁队列。
    deinit();
}

// 定义或声明函数 Battery::init：参数在括号里，返回类型写在函数名前面。
bool Battery::init()
{
    // 判断条件：initialized_ 是否成立。
    if (initialized_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // ADC oneshot 初始化
    adc_oneshot_unit_init_cfg_t unit_cfg = {};
    // 赋值：把右侧 kAdcUnit 的结果保存到左侧 unit_cfg.unit_id。
    unit_cfg.unit_id = kAdcUnit;
    // 赋值：把右侧 ADC_ULP_MODE_DISABLE 的结果保存到左侧 unit_cfg.ulp_mode。
    unit_cfg.ulp_mode = ADC_ULP_MODE_DISABLE;

    // 定义变量 adc_oneshot_unit_handle_t handle 并初始化为 nullptr。
    adc_oneshot_unit_handle_t handle = nullptr;
    // 判断条件：adc_oneshot_new_unit(&unit_cfg, &handle) != ESP_OK 是否成立。
    if (adc_oneshot_new_unit(&unit_cfg, &handle) != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "ADC unit 初始化失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "ADC unit 初始化失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 handle 的结果保存到左侧 adc_handle_。
    adc_handle_ = handle;

    // 通道配置
    adc_oneshot_chan_cfg_t chan_cfg = {};
    // 赋值：把右侧 kAdcAtten 的结果保存到左侧 chan_cfg.atten。
    chan_cfg.atten = kAdcAtten;
    // 赋值：把右侧 kAdcWidth 的结果保存到左侧 chan_cfg.bitwidth。
    chan_cfg.bitwidth = kAdcWidth;

    // 判断条件：adc_oneshot_config_channel(handle, kAdcChannel, &chan_cfg) != ESP_OK 是否成立。
    if (adc_oneshot_config_channel(handle, kAdcChannel, &chan_cfg) != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "ADC 通道配置失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "ADC 通道配置失败");
        // 调用 deinit()：创建和销毁队列。
        deinit();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 校准（优先 curve fitting，回退 line fitting）
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    // 定义变量 adc_cali_curve_fitting_config_t cali_cfg 并初始化为 {}。
    adc_cali_curve_fitting_config_t cali_cfg = {};
    // 赋值：把右侧 kAdcUnit 的结果保存到左侧 cali_cfg.unit_id。
    cali_cfg.unit_id = kAdcUnit;
    // 赋值：把右侧 kAdcChannel 的结果保存到左侧 cali_cfg.chan。
    cali_cfg.chan = kAdcChannel;
    // 赋值：把右侧 kAdcAtten 的结果保存到左侧 cali_cfg.atten。
    cali_cfg.atten = kAdcAtten;
    // 赋值：把右侧 kAdcWidth 的结果保存到左侧 cali_cfg.bitwidth。
    cali_cfg.bitwidth = kAdcWidth;

    // 定义变量 adc_cali_handle_t cali 并初始化为 nullptr。
    adc_cali_handle_t cali = nullptr;
    // 判断条件：adc_cali_create_scheme_curve_fitting(&cali_cfg, &cali) == ESP_OK 是否成立。
    if (adc_cali_create_scheme_curve_fitting(&cali_cfg, &cali) == ESP_OK)
    {
        // 赋值：把右侧 cali 的结果保存到左侧 cali_handle_。
        cali_handle_ = cali;
        // 赋值：把右侧 true 的结果保存到左侧 cali_ok_。
        cali_ok_ = true;
    }
// 执行这条代码：#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED。它会更新变量、对象状态或调用链。
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    // 定义变量 adc_cali_line_fitting_config_t cali_cfg 并初始化为 {}。
    adc_cali_line_fitting_config_t cali_cfg = {};
    // 赋值：把右侧 kAdcUnit 的结果保存到左侧 cali_cfg.unit_id。
    cali_cfg.unit_id = kAdcUnit;
    // 赋值：把右侧 kAdcAtten 的结果保存到左侧 cali_cfg.atten。
    cali_cfg.atten = kAdcAtten;
    // 赋值：把右侧 kAdcWidth 的结果保存到左侧 cali_cfg.bitwidth。
    cali_cfg.bitwidth = kAdcWidth;

    // 定义变量 adc_cali_handle_t cali 并初始化为 nullptr。
    adc_cali_handle_t cali = nullptr;
    // 判断条件：adc_cali_create_scheme_line_fitting(&cali_cfg, &cali) == ESP_OK 是否成立。
    if (adc_cali_create_scheme_line_fitting(&cali_cfg, &cali) == ESP_OK)
    {
        // 赋值：把右侧 cali 的结果保存到左侧 cali_handle_。
        cali_handle_ = cali;
        // 赋值：把右侧 true 的结果保存到左侧 cali_ok_。
        cali_ok_ = true;
    }
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif

    // 判断条件：!cali_ok_ 是否成立。
    if (!cali_ok_)
    {
        // 调用 ESP_LOGW(TAG, "ADC 校准不可用，电压读数可能不准确");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "ADC 校准不可用，电压读数可能不准确");
    }

    // 赋值：把右侧 true 的结果保存到左侧 initialized_。
    initialized_ = true;
    // 调用 ESP_LOGD(TAG, "OK  GPIO%d  cali=%s", static_cast<int>(config::BATTERY_VBAT),，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "OK  GPIO%d  cali=%s", static_cast<int>(config::BATTERY_VBAT),
             // 继续上一行的表达式或参数列表：cali_ok_ ? "yes" : "no");。
             cali_ok_ ? "yes" : "no");
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Battery::deinit：参数在括号里，返回类型写在函数名前面。
void Battery::deinit()
{
    // 判断条件：cali_handle_ != nullptr 是否成立。
    if (cali_handle_ != nullptr)
    {
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        // 调用 adc_cali_delete_scheme_curve_fitting(static_cast<adc_cali_handle_t>(cali_handle_));，执行这个函数并处理它的返回值或状态变化。
        adc_cali_delete_scheme_curve_fitting(static_cast<adc_cali_handle_t>(cali_handle_));
// 执行这条代码：#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED。它会更新变量、对象状态或调用链。
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
        // 调用 adc_cali_delete_scheme_line_fitting(static_cast<adc_cali_handle_t>(cali_handle_));，执行这个函数并处理它的返回值或状态变化。
        adc_cali_delete_scheme_line_fitting(static_cast<adc_cali_handle_t>(cali_handle_));
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
        // 赋值：把右侧 nullptr 的结果保存到左侧 cali_handle_。
        cali_handle_ = nullptr;
    }
    // 判断条件：adc_handle_ != nullptr 是否成立。
    if (adc_handle_ != nullptr)
    {
        // 调用 adc_oneshot_del_unit(static_cast<adc_oneshot_unit_handle_t>(adc_handle_));，执行这个函数并处理它的返回值或状态变化。
        adc_oneshot_del_unit(static_cast<adc_oneshot_unit_handle_t>(adc_handle_));
        // 赋值：把右侧 nullptr 的结果保存到左侧 adc_handle_。
        adc_handle_ = nullptr;
    }
    // 赋值：把右侧 false 的结果保存到左侧 initialized_。
    initialized_ = false;
    // 赋值：把右侧 false 的结果保存到左侧 cali_ok_。
    cali_ok_ = false;
}

// 定义或声明函数 Battery::read_voltage_mv：参数在括号里，返回类型写在函数名前面。
uint32_t Battery::read_voltage_mv()
{
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 赋值：把右侧 static_cast<adc_oneshot_unit_handle_t>(adc_handle_) 的结果保存到左侧 auto* handle。
    auto* handle = static_cast<adc_oneshot_unit_handle_t>(adc_handle_);

    // 多次采样取平均
    int32_t sum = 0;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint32_t i = 0; i < kSampleCount; i++)
    {
        // 定义变量 int raw 并初始化为 0。
        int raw = 0;
        // 判断条件：adc_oneshot_read(handle, kAdcChannel, &raw) == ESP_OK 是否成立。
        if (adc_oneshot_read(handle, kAdcChannel, &raw) == ESP_OK)
        {
            // 赋值：把右侧 raw 的结果保存到左侧 sum +。
            sum += raw;
        }
    }
    // 定义变量 int avg_raw 并初始化为 static_cast<int>(sum / static_cast<int32_t>(kSampleCount))。
    int avg_raw = static_cast<int>(sum / static_cast<int32_t>(kSampleCount));

    // 校准转换为电压
    int voltage_mv = 0;
    // 判断条件：cali_ok_ 是否成立。
    if (cali_ok_)
    {
        // 调用 adc_cali_raw_to_voltage(static_cast<adc_cali_handle_t>(cali_handle_), avg_raw, &voltage_mv);，执行这个函数并处理它的返回值或状态变化。
        adc_cali_raw_to_voltage(static_cast<adc_cali_handle_t>(cali_handle_), avg_raw, &voltage_mv);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 无校准时粗略估算：12-bit @ 12dB ≈ 0~3100mV
        voltage_mv = static_cast<int>((static_cast<uint32_t>(avg_raw) * 3100) / 4095);
    }

    // 乘以分压系数还原电池实际电压
    return static_cast<uint32_t>(voltage_mv) * kDividerFactor;
}

// 定义或声明函数 Battery::read_percent：参数在括号里，返回类型写在函数名前面。
uint8_t Battery::read_percent()
{
    // 返回 voltage_to_percent(read_voltage_mv()) 给调用者，表示当前函数的执行结果。
    return voltage_to_percent(read_voltage_mv());
}

// 定义或声明函数 Battery::mv_to_percent：参数在括号里，返回类型写在函数名前面。
uint8_t Battery::mv_to_percent(uint32_t mv)
{
    // 返回 voltage_to_percent(mv) 给调用者，表示当前函数的执行结果。
    return voltage_to_percent(mv);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::bsp::battery

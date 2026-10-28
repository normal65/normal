/**
 * [初学者导读] 文件：main/bsp/driver/button/button.cc
 * 作用：读取拨码/物理按键的 ADC 电压并产生按键事件。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：ADC 采样有抖动和区间误差，需要去抖和边界判断。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "button.hpp"

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
static const char* const TAG = "DipSwitch";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::bsp::driver::button {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 固定参数 kAdcAtten = ADC_ATTEN_DB_12：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr adc_atten_t kAdcAtten = ADC_ATTEN_DB_12;
// 固定参数 kAdcWidth = ADC_BITWIDTH_12：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr adc_bitwidth_t kAdcWidth = ADC_BITWIDTH_12;

// 定义或声明函数 classify_position：参数在括号里，返回类型写在函数名前面。
DipId classify_position(uint32_t mv, const Config& cfg)
{
    // 判断条件：mv >= cfg.threshold_released_mv 是否成立。
    if (mv >= cfg.threshold_released_mv)
    {
        // 返回 DipId::NONE 给调用者，表示当前函数的执行结果。
        return DipId::NONE;
    }
    // 判断条件：mv < cfg.threshold_up_mid_mv 是否成立。
    if (mv < cfg.threshold_up_mid_mv)
    {
        // 返回 DipId::UP 给调用者，表示当前函数的执行结果。
        return DipId::UP;
    }
    // 判断条件：mv < cfg.threshold_mid_down_mv 是否成立。
    if (mv < cfg.threshold_mid_down_mv)
    {
        // 返回 DipId::MID 给调用者，表示当前函数的执行结果。
        return DipId::MID;
    }
    // 返回 DipId::DOWN 给调用者，表示当前函数的执行结果。
    return DipId::DOWN;
}

// 定义或声明函数 state_of：参数在括号里，返回类型写在函数名前面。
DipState state_of(DipId pos)
{
    // 定义变量 DipState st 并初始化为 {}。
    DipState st = {};
    // 多分支判断：根据括号里的值选择不同 case。
    switch (pos)
    {
        // 分支匹配：case DipId::UP 成立时执行下面的代码。
        case DipId::UP:
            // 赋值：把右侧 true 的结果保存到左侧 st.up。
            st.up = true;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case DipId::MID 成立时执行下面的代码。
        case DipId::MID:
            // 赋值：把右侧 true 的结果保存到左侧 st.mid。
            st.mid = true;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case DipId::DOWN 成立时执行下面的代码。
        case DipId::DOWN:
            // 赋值：把右侧 true 的结果保存到左侧 st.down。
            st.down = true;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case DipId::NONE 成立时执行下面的代码。
        case DipId::NONE:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
    // 返回 st 给调用者，表示当前函数的执行结果。
    return st;
}

// 定义或声明函数 mask_of：参数在括号里，返回类型写在函数名前面。
uint8_t mask_of(DipId pos)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (pos)
    {
        // 分支匹配：case DipId::UP 成立时执行下面的代码。
        case DipId::UP:
            // 返回 1U << 0 给调用者，表示当前函数的执行结果。
            return 1U << 0;
        // 分支匹配：case DipId::MID 成立时执行下面的代码。
        case DipId::MID:
            // 返回 1U << 1 给调用者，表示当前函数的执行结果。
            return 1U << 1;
        // 分支匹配：case DipId::DOWN 成立时执行下面的代码。
        case DipId::DOWN:
            // 返回 1U << 2 给调用者，表示当前函数的执行结果。
            return 1U << 2;
        // 分支匹配：case DipId::NONE 成立时执行下面的代码。
        case DipId::NONE:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
    // 返回 0 给调用者，表示当前函数的执行结果。
    return 0;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 DipSwitch::DipSwitch：参数在括号里，返回类型写在函数名前面。
DipSwitch::DipSwitch()
    // 执行这条代码：: cfg_{}。它会更新变量、对象状态或调用链。
    : cfg_{}
    // 定义或声明函数 adc_handle_：参数在括号里，返回类型写在函数名前面。
    , adc_handle_(nullptr)
    // 定义或声明函数 cali_handle_：参数在括号里，返回类型写在函数名前面。
    , cali_handle_(nullptr)
    // 定义或声明函数 adc_unit_：参数在括号里，返回类型写在函数名前面。
    , adc_unit_(-1)
    // 定义或声明函数 adc_channel_：参数在括号里，返回类型写在函数名前面。
    , adc_channel_(-1)
    // 定义或声明函数 initialized_：参数在括号里，返回类型写在函数名前面。
    , initialized_(false)
    // 定义或声明函数 cali_ok_：参数在括号里，返回类型写在函数名前面。
    , cali_ok_(false)
{
}

// 定义或声明函数 DipSwitch::~DipSwitch：参数在括号里，返回类型写在函数名前面。
DipSwitch::~DipSwitch()
{
    // 调用 deinit()：创建和销毁队列。
    deinit();
}

// 定义或声明函数 DipSwitch::init：参数在括号里，返回类型写在函数名前面。
bool DipSwitch::init(const Config* config)
{
    // 判断条件：initialized_ 是否成立。
    if (initialized_)
    {
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 赋值：把右侧 (config != nullptr) ? *config : Config() 的结果保存到左侧 cfg_。
    cfg_ = (config != nullptr) ? *config : Config();

    // 定义变量 adc_unit_t unit 并初始化为 ADC_UNIT_1。
    adc_unit_t unit = ADC_UNIT_1;
    // 定义变量 adc_channel_t channel 并初始化为 ADC_CHANNEL_0。
    adc_channel_t channel = ADC_CHANNEL_0;
    // 判断条件：adc_oneshot_io_to_channel(config::DIP_SWITCH, &unit, &channel) != ESP_OK 是否成立。
    if (adc_oneshot_io_to_channel(config::DIP_SWITCH, &unit, &channel) != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "GPIO%d 无法映射 ADC 通道", static_cast<int>(config::DIP_SWITCH));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "GPIO%d 无法映射 ADC 通道", static_cast<int>(config::DIP_SWITCH));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 static_cast<int>(unit) 的结果保存到左侧 adc_unit_。
    adc_unit_ = static_cast<int>(unit);
    // 赋值：把右侧 static_cast<int>(channel) 的结果保存到左侧 adc_channel_。
    adc_channel_ = static_cast<int>(channel);

    // 定义变量 adc_oneshot_unit_init_cfg_t unit_cfg 并初始化为 {}。
    adc_oneshot_unit_init_cfg_t unit_cfg = {};
    // 赋值：把右侧 unit 的结果保存到左侧 unit_cfg.unit_id。
    unit_cfg.unit_id = unit;
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

    // 定义变量 adc_oneshot_chan_cfg_t chan_cfg 并初始化为 {}。
    adc_oneshot_chan_cfg_t chan_cfg = {};
    // 赋值：把右侧 kAdcAtten 的结果保存到左侧 chan_cfg.atten。
    chan_cfg.atten = kAdcAtten;
    // 赋值：把右侧 kAdcWidth 的结果保存到左侧 chan_cfg.bitwidth。
    chan_cfg.bitwidth = kAdcWidth;

    // 判断条件：adc_oneshot_config_channel(handle, channel, &chan_cfg) != ESP_OK 是否成立。
    if (adc_oneshot_config_channel(handle, channel, &chan_cfg) != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "ADC 通道配置失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "ADC 通道配置失败");
        // 调用 deinit()：创建和销毁队列。
        deinit();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    // 定义变量 adc_cali_curve_fitting_config_t cali_cfg 并初始化为 {}。
    adc_cali_curve_fitting_config_t cali_cfg = {};
    // 赋值：把右侧 unit 的结果保存到左侧 cali_cfg.unit_id。
    cali_cfg.unit_id = unit;
    // 赋值：把右侧 channel 的结果保存到左侧 cali_cfg.chan。
    cali_cfg.chan = channel;
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
    // 赋值：把右侧 unit 的结果保存到左侧 cali_cfg.unit_id。
    cali_cfg.unit_id = unit;
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

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t mv = read_voltage_mv();
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const DipState st = read();
    // 调用 ESP_LOGD(TAG, "OK  GPIO%d  %umV  up=%d  mid=%d  down=%d  mask=0x%02X",，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "OK  GPIO%d  %umV  up=%d  mid=%d  down=%d  mask=0x%02X",
             // 继续上一行的表达式或参数列表：static_cast<int>(config::DIP_SWITCH), static_cast<unsigned>(mv), st.up ? 1 : 0,。
             static_cast<int>(config::DIP_SWITCH), static_cast<unsigned>(mv), st.up ? 1 : 0,
             // 继续上一行的表达式或参数列表：st.mid ? 1 : 0, st.down ? 1 : 0, read_mask());。
             st.mid ? 1 : 0, st.down ? 1 : 0, read_mask());
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 DipSwitch::deinit：参数在括号里，返回类型写在函数名前面。
void DipSwitch::deinit()
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

    // 赋值：把右侧 -1 的结果保存到左侧 adc_unit_。
    adc_unit_ = -1;
    // 赋值：把右侧 -1 的结果保存到左侧 adc_channel_。
    adc_channel_ = -1;
    // 赋值：把右侧 false 的结果保存到左侧 initialized_。
    initialized_ = false;
    // 赋值：把右侧 false 的结果保存到左侧 cali_ok_。
    cali_ok_ = false;
}

// 定义或声明函数 DipSwitch::read_voltage_mv：参数在括号里，返回类型写在函数名前面。
uint32_t DipSwitch::read_voltage_mv() const
{
    // 判断条件：!initialized_ || adc_handle_ == nullptr || adc_channel_ < 0 是否成立。
    if (!initialized_ || adc_handle_ == nullptr || adc_channel_ < 0)
    {
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }

    // 赋值：把右侧 static_cast<adc_oneshot_unit_handle_t>(adc_handle_) 的结果保存到左侧 auto* handle。
    auto* handle = static_cast<adc_oneshot_unit_handle_t>(adc_handle_);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto channel = static_cast<adc_channel_t>(adc_channel_);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t samples = (cfg_.sample_count > 0) ? cfg_.sample_count : 1;

    // 定义变量 int32_t sum 并初始化为 0。
    int32_t sum = 0;
    // 定义变量 uint32_t ok_count 并初始化为 0。
    uint32_t ok_count = 0;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint32_t i = 0; i < samples; i++)
    {
        // 定义变量 int raw 并初始化为 0。
        int raw = 0;
        // 判断条件：adc_oneshot_read(handle, channel, &raw) == ESP_OK 是否成立。
        if (adc_oneshot_read(handle, channel, &raw) == ESP_OK)
        {
            // 赋值：把右侧 raw 的结果保存到左侧 sum +。
            sum += raw;
            // 执行语句：ok_count++;；它会更新一次变量、对象或程序状态。
            ok_count++;
        }
    }
    // 判断条件：ok_count == 0 是否成立。
    if (ok_count == 0)
    {
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int avg_raw = static_cast<int>(sum / static_cast<int32_t>(ok_count));

    // 定义变量 int voltage_mv 并初始化为 0。
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
        // 赋值：把右侧 static_cast<int>((static_cast<uint32_t>(avg_raw) * 3100) / 4095) 的结果保存到左侧 voltage_mv。
        voltage_mv = static_cast<int>((static_cast<uint32_t>(avg_raw) * 3100) / 4095);
    }

    // 返回 static_cast<uint32_t>(voltage_mv) 给调用者，表示当前函数的执行结果。
    return static_cast<uint32_t>(voltage_mv);
}

// 定义或声明函数 DipSwitch::read_position：参数在括号里，返回类型写在函数名前面。
DipId DipSwitch::read_position() const
{
    // 返回 classify_position(read_voltage_mv(), cfg_) 给调用者，表示当前函数的执行结果。
    return classify_position(read_voltage_mv(), cfg_);
}

// 定义或声明函数 DipSwitch::read：参数在括号里，返回类型写在函数名前面。
DipState DipSwitch::read() const
{
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};
    }
    // 返回 state_of(read_position()) 给调用者，表示当前函数的执行结果。
    return state_of(read_position());
}

// 定义或声明函数 DipSwitch::read：参数在括号里，返回类型写在函数名前面。
bool DipSwitch::read(DipId id) const
{
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 返回 read_position() == id 给调用者，表示当前函数的执行结果。
    return read_position() == id;
}

// 定义或声明函数 DipSwitch::read_mask：参数在括号里，返回类型写在函数名前面。
uint8_t DipSwitch::read_mask() const
{
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }
    // 返回 mask_of(read_position()) 给调用者，表示当前函数的执行结果。
    return mask_of(read_position());
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::bsp::driver::button

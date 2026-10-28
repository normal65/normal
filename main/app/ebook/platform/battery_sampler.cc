/**
 * [初学者导读] 文件：main/app/ebook/platform/battery_sampler.cc
 * 作用：周期采样电池电压，并对电量进行平滑和状态广播。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：阻塞工作放独立任务，周期小检查放 Housekeeper。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/battery_sampler.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "bsp/battery/battery.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "BatterySampler";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::platform {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 执行语句：::app::bsp::battery::Battery g_bat;；它会更新一次变量、对象或程序状态。
::app::bsp::battery::Battery g_bat;

// 定义或声明函数 ema：参数在括号里，返回类型写在函数名前面。
uint32_t ema(uint32_t prev, uint32_t raw, uint8_t k)
{
    // 判断条件：k <= 1 是否成立。
    if (k <= 1)
        // 返回 raw 给调用者，表示当前函数的执行结果。
        return raw;
    // 返回 ((prev * (k - 1)) + raw) / k 给调用者，表示当前函数的执行结果。
    return ((prev * (k - 1)) + raw) / k;
}

// 定义或声明函数 mv_to_pct：参数在括号里，返回类型写在函数名前面。
uint8_t mv_to_pct(uint32_t mv)
{
    // 返回 ::app::bsp::battery::Battery::mv_to_percent(mv) 给调用者，表示当前函数的执行结果。
    return ::app::bsp::battery::Battery::mv_to_percent(mv);
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 BatterySampler::get_instance：参数在括号里，返回类型写在函数名前面。
BatterySampler& BatterySampler::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static BatterySampler s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 BatterySampler::init：参数在括号里，返回类型写在函数名前面。
bool BatterySampler::init()
{
    // 判断条件：ready_ 是否成立。
    if (ready_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 判断条件：!g_bat.init() 是否成立。
    if (!g_bat.init())
    {
        // 调用 EBOOK_LOGW(TAG, "init failed");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "init failed");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 立即采一次作为 EMA / 迟滞基线
    const uint32_t mv = g_bat.read_voltage_mv();
    // 赋值：把右侧 mv 的结果保存到左侧 voltage_ema_mv_。
    voltage_ema_mv_ = mv;
    // 赋值：把右侧 true 的结果保存到左侧 ema_init_。
    ema_init_ = true;
    // 赋值：把右侧 mv_to_pct(mv) 的结果保存到左侧 stable_pct_。
    stable_pct_ = mv_to_pct(mv);
    // 赋值：把右侧 true 的结果保存到左侧 pct_init_。
    pct_init_ = true;
    // 调用 data::SystemState::get_instance().set_battery(stable_pct_, mv);，执行这个函数并处理它的返回值或状态变化。
    data::SystemState::get_instance().set_battery(stable_pct_, mv);
    // 赋值：把右侧 true 的结果保存到左侧 ready_。
    ready_ = true;

    // 调用 EBOOK_LOGD(TAG, "init OK: %u%% %umV", static_cast<unsigned>(stable_pct_),，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGD(TAG, "init OK: %u%% %umV", static_cast<unsigned>(stable_pct_),
               // 继续上一行的表达式或参数列表：static_cast<unsigned>(mv));。
               static_cast<unsigned>(mv));
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 BatterySampler::voltage_mv：参数在括号里，返回类型写在函数名前面。
uint32_t BatterySampler::voltage_mv()
{
    // 返回 ready_ ? voltage_ema_mv_ : 0 给调用者，表示当前函数的执行结果。
    return ready_ ? voltage_ema_mv_ : 0;
}

// 定义或声明函数 BatterySampler::percent：参数在括号里，返回类型写在函数名前面。
uint8_t BatterySampler::percent()
{
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
        // 返回 data::SystemState::get_instance().battery_pct() 给调用者，表示当前函数的执行结果。
        return data::SystemState::get_instance().battery_pct();

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t raw_mv = g_bat.read_voltage_mv();

    // 赋值：把右侧 ema_init_ ? ema(voltage_ema_mv_, raw_mv, kEmaWeight) : raw_mv 的结果保存到左侧 voltage_ema_mv_。
    voltage_ema_mv_ = ema_init_ ? ema(voltage_ema_mv_, raw_mv, kEmaWeight) : raw_mv;
    // 赋值：把右侧 true 的结果保存到左侧 ema_init_。
    ema_init_ = true;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t cand = mv_to_pct(voltage_ema_mv_);

    // 首次 / 显著下降 / 充电跳变
    const bool first = !pct_init_;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool drop_enough = (cand + kDropDeltaPct <= stable_pct_);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool charge_jump = (cand >= stable_pct_ + kChargeJumpPct);

    // 判断条件：first || drop_enough || charge_jump 是否成立。
    if (first || drop_enough || charge_jump)
        // 赋值：把右侧 cand 的结果保存到左侧 stable_pct_。
        stable_pct_ = cand;
    // 赋值：把右侧 true 的结果保存到左侧 pct_init_。
    pct_init_ = true;

    // 调用 data::SystemState::get_instance().set_battery(stable_pct_, voltage_ema_mv_);，执行这个函数并处理它的返回值或状态变化。
    data::SystemState::get_instance().set_battery(stable_pct_, voltage_ema_mv_);

    // 调用 EBOOK_LOGD(TAG, "raw=%umV ema=%umV cand=%u stable=%u", static_cast<unsigned>(raw_mv),，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGD(TAG, "raw=%umV ema=%umV cand=%u stable=%u", static_cast<unsigned>(raw_mv),
               // 继续上一行的表达式或参数列表：static_cast<unsigned>(voltage_ema_mv_), static_cast<unsigned>(cand),。
               static_cast<unsigned>(voltage_ema_mv_), static_cast<unsigned>(cand),
               // 继续上一行的表达式或参数列表：static_cast<unsigned>(stable_pct_));。
               static_cast<unsigned>(stable_pct_));

    // 返回 stable_pct_ 给调用者，表示当前函数的执行结果。
    return stable_pct_;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::platform

/**
 * [初学者导读] 文件：main/app/ebook/apps/weather/weather_cities.cc
 * 作用：保存和查询天气城市码。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/weather/weather_cities.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/persist.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::weather {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 固定参数 kKCity = "wx.city"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKCity = "wx.city";

/** name, adcode（高德行政区划代码） */
// 编译期数组 kCities：运行时只读的固定查表数据。
constexpr ChinaCity kCities[] = {
    // 执行这条代码：{"\u5317\u4EAC", "110000"}, {"\u4E0A\u6D77", "310000"}, {"\u5E7F\u5DDE", "440100"},。它会更新变量、对象状态或调用链。
    {"\u5317\u4EAC", "110000"}, {"\u4E0A\u6D77", "310000"}, {"\u5E7F\u5DDE", "440100"},
    // 继续上一行的表达式或参数列表：{"\u6DF1\u5733", "440300"}, {"\u6210\u90FD", "510100"}, {"\u676D\u5DDE", "330100"},。
    {"\u6DF1\u5733", "440300"}, {"\u6210\u90FD", "510100"}, {"\u676D\u5DDE", "330100"},
    // 继续上一行的表达式或参数列表：{"\u6B66\u6C49", "420100"}, {"\u897F\u5B89", "610100"}, {"\u5357\u4EAC", "320100"},。
    {"\u6B66\u6C49", "420100"}, {"\u897F\u5B89", "610100"}, {"\u5357\u4EAC", "320100"},
    // 继续上一行的表达式或参数列表：{"\u5929\u6D25", "120000"}, {"\u91CD\u5E86", "500000"}, {"\u82CF\u5DDE", "320500"},。
    {"\u5929\u6D25", "120000"}, {"\u91CD\u5E86", "500000"}, {"\u82CF\u5DDE", "320500"},
    // 继续上一行的表达式或参数列表：{"\u90D1\u5DDE", "410100"}, {"\u957F\u6C99", "430100"}, {"\u6C88\u9633", "210100"},。
    {"\u90D1\u5DDE", "410100"}, {"\u957F\u6C99", "430100"}, {"\u6C88\u9633", "210100"},
    // 继续上一行的表达式或参数列表：{"\u9752\u5C9B", "370200"}, {"\u5927\u8FDE", "210200"}, {"\u53A6\u95E8", "350200"},。
    {"\u9752\u5C9B", "370200"}, {"\u5927\u8FDE", "210200"}, {"\u53A6\u95E8", "350200"},
    // 继续上一行的表达式或参数列表：{"\u798F\u5DDE", "350100"}, {"\u6D4E\u5357", "370100"}, {"\u54C8\u5C14\u6EE8", "230100"},。
    {"\u798F\u5DDE", "350100"}, {"\u6D4E\u5357", "370100"}, {"\u54C8\u5C14\u6EE8", "230100"},
    // 继续上一行的表达式或参数列表：{"\u957F\u6625", "220100"}, {"\u6606\u660E", "530100"}, {"\u8D35\u9633", "520100"},。
    {"\u957F\u6625", "220100"}, {"\u6606\u660E", "530100"}, {"\u8D35\u9633", "520100"},
    // 继续上一行的表达式或参数列表：{"\u5357\u5B81", "450100"}, {"\u5408\u80A5", "340100"}, {"\u77F3\u5BB6\u5E84", "130100"},。
    {"\u5357\u5B81", "450100"}, {"\u5408\u80A5", "340100"}, {"\u77F3\u5BB6\u5E84", "130100"},
    // 继续上一行的表达式或参数列表：{"\u592A\u539F", "140100"}, {"\u5357\u660C", "360100"}, {"\u73E0\u6D77", "440400"},。
    {"\u592A\u539F", "140100"}, {"\u5357\u660C", "360100"}, {"\u73E0\u6D77", "440400"},
    // 继续上一行的表达式或参数列表：{"\u4F5B\u5C71", "440600"},。
    {"\u4F5B\u5C71", "440600"},
};

// 固定参数 kCount = static_cast<uint8_t>(sizeof(kCities) / sizeof(kCities[0]))：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kCount = static_cast<uint8_t>(sizeof(kCities) / sizeof(kCities[0]));

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 WeatherCities::count：参数在括号里，返回类型写在函数名前面。
uint8_t WeatherCities::count()
{
    // 返回 kCount 给调用者，表示当前函数的执行结果。
    return kCount;
}

// 只读约束：这个值或对象在当前位置不能被修改。
const ChinaCity& WeatherCities::at(uint8_t idx)
{
    // 天气城市查询失败时返回的默认城市，保证上层始终能拿到有效引用。
    static const ChinaCity kFallback{"", "110000"};
    // 返回 (idx < kCount) ? kCities[idx] : kFallback 给调用者，表示当前函数的执行结果。
    return (idx < kCount) ? kCities[idx] : kFallback;
}

// 只读约束：这个值或对象在当前位置不能被修改。
const ChinaCity* WeatherCities::find_by_name(const char* name)
{
    // 判断条件：name == nullptr 是否成立。
    if (name == nullptr)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < kCount; ++i)
        // 判断条件：std::strcmp(kCities[i].name, name) == 0 是否成立。
        if (std::strcmp(kCities[i].name, name) == 0)
            // 返回 &kCities[i] 给调用者，表示当前函数的执行结果。
            return &kCities[i];
    // 返回 nullptr 给调用者，表示当前函数的执行结果。
    return nullptr;
}

// 定义或声明函数 WeatherCities::find_index_by_adcode：参数在括号里，返回类型写在函数名前面。
uint8_t WeatherCities::find_index_by_adcode(const char* adcode)
{
    // 判断条件：adcode == nullptr || std::strlen(adcode) < 6 是否成立。
    if (adcode == nullptr || std::strlen(adcode) < 6)
    {
        // 返回 0xFF 给调用者，表示当前函数的执行结果。
        return 0xFF;
    }

    // 开始代码块：下面是这个函数、分支或循环的主体。
    auto try_match = [](const char* code) -> uint8_t {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t i = 0; i < kCount; ++i)
        {
            // 判断条件：std::strcmp(kCities[i].adcode, code) == 0 是否成立。
            if (std::strcmp(kCities[i].adcode, code) == 0)
            {
                // 返回 i 给调用者，表示当前函数的执行结果。
                return i;
            }
        }
        // 返回 0xFF 给调用者，表示当前函数的执行结果。
        return 0xFF;
    };

    // 定义变量 uint8_t idx 并初始化为 try_match(adcode)。
    uint8_t idx = try_match(adcode);
    // 判断条件：idx != 0xFF 是否成立。
    if (idx != 0xFF)
    {
        // 返回 idx 给调用者，表示当前函数的执行结果。
        return idx;
    }

    // 执行语句：char norm[7];；它会更新一次变量、对象或程序状态。
    char norm[7];
    // 调用 std::strncpy(norm, adcode, 6);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(norm, adcode, 6);
    // 赋值：把右侧 '\0' 的结果保存到左侧 norm[6]。
    norm[6] = '\0';

    // 区县级 → 市级：510107 → 510100
    norm[4] = '0';
    // 赋值：把右侧 '0' 的结果保存到左侧 norm[5]。
    norm[5] = '0';
    // 赋值：把右侧 try_match(norm) 的结果保存到左侧 idx。
    idx = try_match(norm);
    // 判断条件：idx != 0xFF 是否成立。
    if (idx != 0xFF)
    {
        // 返回 idx 给调用者，表示当前函数的执行结果。
        return idx;
    }

    // 直辖市区县 → 省级：110101 → 110000
    norm[2] = '0';
    // 赋值：把右侧 '0' 的结果保存到左侧 norm[3]。
    norm[3] = '0';
    // 返回 try_match(norm) 给调用者，表示当前函数的执行结果。
    return try_match(norm);
}

// 定义或声明函数 WeatherCities::load_index：参数在括号里，返回类型写在函数名前面。
uint8_t WeatherCities::load_index()
{
    // 定义变量 uint8_t idx 并初始化为 0。
    uint8_t idx = 0;
    // 判断条件：data::Persist::get_u8(kKCity, idx) && idx < kCount 是否成立。
    if (data::Persist::get_u8(kKCity, idx) && idx < kCount)
        // 返回 idx 给调用者，表示当前函数的执行结果。
        return idx;
    // 返回 0 给调用者，表示当前函数的执行结果。
    return 0;
}

// 定义或声明函数 WeatherCities::save_index：参数在括号里，返回类型写在函数名前面。
void WeatherCities::save_index(uint8_t idx)
{
    // 判断条件：idx >= kCount 是否成立。
    if (idx >= kCount)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 函数调用：(void)data::Persist::set_u8(kKCity, idx);；执行该函数并处理它的返回值或副作用。
    (void)data::Persist::set_u8(kKCity, idx);
    // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    data::Persist::commit();
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::weather

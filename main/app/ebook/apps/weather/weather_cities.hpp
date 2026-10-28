// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/weather/weather_cities.hpp
 * 作用：保存和查询天气城市码。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::weather {

// struct ChinaCity：把相关数据字段组合在一起，表示项目中的一个概念。
struct ChinaCity
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* name;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* adcode; /**< 高德城市编码，6 位数字字符串 */
};

/**
 * @brief 国内常用城市表（NVS: wx.city）
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class WeatherCities
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 count：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static uint8_t count();
    // 静态函数 at：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static const ChinaCity& at(uint8_t idx);
    // 静态函数 find_by_name：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static const ChinaCity* find_by_name(const char* name);

    /** 按高德 adcode 查城市索引；支持区县级 adcode 归并到市级 */
    // 静态函数 find_index_by_adcode：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static uint8_t find_index_by_adcode(const char* adcode);

    // 静态函数 load_index：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static uint8_t load_index();
    // 静态函数 save_index：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void save_index(uint8_t idx);
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::weather

/**
 * [初学者导读] 文件：main/app/ebook/apps/weather/weather_app.cc
 * 作用：获取并显示城市天气，处理城市选择和刷新状态。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/weather/weather_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <string>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cJSON.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "sdkconfig.h"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/weather/weather_cities.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "network/wifi/wifi.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "protocol/http/http.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::weather {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 任务栈大小：8192，栈太小会崩溃，太大会浪费 RAM。
constexpr uint32_t kFetchStack = 8192;
// 超时上限：超过 12000 毫秒仍未完成时，代码会按失败处理。
constexpr int32_t kHttpTimeoutMs = 12000;

// 只读约束：这个值或对象在当前位置不能被修改。
const char* amap_web_key()
{
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if defined(CONFIG_EBOOK_AMAP_WEATHER_KEY)
    // 返回 CONFIG_EBOOK_AMAP_WEATHER_KEY 给调用者，表示当前函数的执行结果。
    return CONFIG_EBOOK_AMAP_WEATHER_KEY;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
    // 返回 "" 给调用者，表示当前函数的执行结果。
    return "";
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
}

// 定义或声明函数 amap_key_ready：参数在括号里，返回类型写在函数名前面。
bool amap_key_ready()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* k = amap_web_key();
    // 返回 k != nullptr && k[0] != '\0' 给调用者，表示当前函数的执行结果。
    return k != nullptr && k[0] != '\0';
}

// 定义或声明函数 city_switch_rect：参数在括号里，返回类型写在函数名前面。
core::Rect city_switch_rect()
{
    // 返回 core::Rect{Theme::kPadLg, static_cast<int16_t>(Theme::kListStartY + 4), 给调用者，表示当前函数的执行结果。
    return core::Rect{Theme::kPadLg, static_cast<int16_t>(Theme::kListStartY + 4),
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenW - Theme::kPadLg * 2), 28};。
                      static_cast<uint16_t>(Theme::kScreenW - Theme::kPadLg * 2), 28};
}

// 定义或声明函数 amap_status_ok：参数在括号里，返回类型写在函数名前面。
bool amap_status_ok(cJSON* root)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const cJSON* st = cJSON_GetObjectItem(root, "status");
    // 返回 cJSON_IsString(st) && st->valuestring != nullptr && 给调用者，表示当前函数的执行结果。
    return cJSON_IsString(st) && st->valuestring != nullptr &&
           // 继续上一行的表达式或参数列表：std::strcmp(st->valuestring, "1") == 0;。
           std::strcmp(st->valuestring, "1") == 0;
}

// 定义或声明函数 parse_temp_str：参数在括号里，返回类型写在函数名前面。
int16_t parse_temp_str(const char* s)
{
    // 判断条件：s == nullptr || s[0] == '\0' 是否成立。
    if (s == nullptr || s[0] == '\0')
    {
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }
    // 返回 static_cast<int16_t>(std::atoi(s)) 给调用者，表示当前函数的执行结果。
    return static_cast<int16_t>(std::atoi(s));
}

// 定义或声明函数 copy_desc：参数在括号里，返回类型写在函数名前面。
void copy_desc(char* dst, size_t cap, const char* src)
{
    // 判断条件：dst == nullptr || cap == 0 是否成立。
    if (dst == nullptr || cap == 0)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 判断条件：src == nullptr 是否成立。
    if (src == nullptr)
    {
        // 赋值：把右侧 '\0' 的结果保存到左侧 dst[0]。
        dst[0] = '\0';
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 调用 std::strncpy(dst, src, cap - 1);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(dst, src, cap - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 dst[cap - 1]。
    dst[cap - 1] = '\0';
}

// 定义或声明函数 http_get_text：参数在括号里，返回类型写在函数名前面。
bool http_get_text(const char* url, std::string& out)
{
    // 执行语句：::app::protocol::http::HttpResponse resp;；它会更新一次变量、对象或程序状态。
    ::app::protocol::http::HttpResponse resp;
    // 判断条件：!::app::protocol::http::HttpMgr::get_instance().get(url, resp, kHttpTimeoutMs) 是否成立。
    if (!::app::protocol::http::HttpMgr::get_instance().get(url, resp, kHttpTimeoutMs))
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 访问对象成员：out.assign(resp.body.begin(), resp.body.end());；读取或修改这个对象里的成员状态。
    out.assign(resp.body.begin(), resp.body.end());
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 parse_amap_live：参数在括号里，返回类型写在函数名前面。
bool parse_amap_live(const char* json, int16_t& cur, char* desc, size_t desc_len)
{
    // 判断条件：json == nullptr 是否成立。
    if (json == nullptr)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 cJSON_Parse(json) 的结果保存到左侧 cJSON* root。
    cJSON* root = cJSON_Parse(json);
    // 判断条件：root == nullptr 是否成立。
    if (root == nullptr)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 bool ok 并初始化为 false。
    bool ok = false;
    // 判断条件：amap_status_ok(root) 是否成立。
    if (amap_status_ok(root))
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const cJSON* lives = cJSON_GetObjectItem(root, "lives");
        // 判断条件：cJSON_IsArray(lives) && cJSON_GetArraySize(lives) > 0 是否成立。
        if (cJSON_IsArray(lives) && cJSON_GetArraySize(lives) > 0)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const cJSON* live = cJSON_GetArrayItem(lives, 0);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const cJSON* temp = cJSON_GetObjectItem(live, "temperature");
            // 只读约束：这个值或对象在当前位置不能被修改。
            const cJSON* wx = cJSON_GetObjectItem(live, "weather");
            // 判断条件：cJSON_IsString(temp) 是否成立。
            if (cJSON_IsString(temp))
            {
                // 赋值：把右侧 parse_temp_str(temp->valuestring) 的结果保存到左侧 cur。
                cur = parse_temp_str(temp->valuestring);
                // 赋值：把右侧 true 的结果保存到左侧 ok。
                ok = true;
            }
            // 判断条件：cJSON_IsString(wx) 是否成立。
            if (cJSON_IsString(wx))
            {
                // 调用 copy_desc(desc, desc_len, wx->valuestring);，执行这个函数并处理它的返回值或状态变化。
                copy_desc(desc, desc_len, wx->valuestring);
            }
        }
    }

    // 调用 cJSON_Delete(root);，执行这个函数并处理它的返回值或状态变化。
    cJSON_Delete(root);
    // 返回 ok 给调用者，表示当前函数的执行结果。
    return ok;
}

// 函数调用：bool parse_amap_forecast(const char* json, int16_t& tmax, int16_t& tmin, char* desc,；执行该函数并处理它的返回值或副作用。
bool parse_amap_forecast(const char* json, int16_t& tmax, int16_t& tmin, char* desc,
                         // 继续上一行的表达式或参数列表：size_t desc_len)。
                         size_t desc_len)
{
    // 判断条件：json == nullptr 是否成立。
    if (json == nullptr)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 cJSON_Parse(json) 的结果保存到左侧 cJSON* root。
    cJSON* root = cJSON_Parse(json);
    // 判断条件：root == nullptr 是否成立。
    if (root == nullptr)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 bool ok 并初始化为 false。
    bool ok = false;
    // 判断条件：amap_status_ok(root) 是否成立。
    if (amap_status_ok(root))
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const cJSON* forecasts = cJSON_GetObjectItem(root, "forecasts");
        // 判断条件：cJSON_IsArray(forecasts) && cJSON_GetArraySize(forecasts) > 0 是否成立。
        if (cJSON_IsArray(forecasts) && cJSON_GetArraySize(forecasts) > 0)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const cJSON* fc = cJSON_GetArrayItem(forecasts, 0);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const cJSON* casts = cJSON_GetObjectItem(fc, "casts");
            // 判断条件：cJSON_IsArray(casts) && cJSON_GetArraySize(casts) > 0 是否成立。
            if (cJSON_IsArray(casts) && cJSON_GetArraySize(casts) > 0)
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const cJSON* today = cJSON_GetArrayItem(casts, 0);
                // 只读约束：这个值或对象在当前位置不能被修改。
                const cJSON* dayt = cJSON_GetObjectItem(today, "daytemp");
                // 只读约束：这个值或对象在当前位置不能被修改。
                const cJSON* night = cJSON_GetObjectItem(today, "nighttemp");
                // 只读约束：这个值或对象在当前位置不能被修改。
                const cJSON* wx = cJSON_GetObjectItem(today, "dayweather");
                // 判断条件：cJSON_IsString(dayt) && cJSON_IsString(night) 是否成立。
                if (cJSON_IsString(dayt) && cJSON_IsString(night))
                {
                    // 赋值：把右侧 parse_temp_str(dayt->valuestring) 的结果保存到左侧 tmax。
                    tmax = parse_temp_str(dayt->valuestring);
                    // 赋值：把右侧 parse_temp_str(night->valuestring) 的结果保存到左侧 tmin。
                    tmin = parse_temp_str(night->valuestring);
                    // 赋值：把右侧 true 的结果保存到左侧 ok。
                    ok = true;
                }
                // 判断条件：desc[0] == '\0' && cJSON_IsString(wx) 是否成立。
                if (desc[0] == '\0' && cJSON_IsString(wx))
                {
                    // 调用 copy_desc(desc, desc_len, wx->valuestring);，执行这个函数并处理它的返回值或状态变化。
                    copy_desc(desc, desc_len, wx->valuestring);
                }
            }
        }
    }

    // 调用 cJSON_Delete(root);，执行这个函数并处理它的返回值或状态变化。
    cJSON_Delete(root);
    // 返回 ok 给调用者，表示当前函数的执行结果。
    return ok;
}

// 返回值：0 成功，2 网络失败，3 解析失败，4 无 Key
uint8_t fetch_amap_weather(const char* adcode, int16_t& cur, int16_t& tmax, int16_t& tmin,
                           // 继续上一行的表达式或参数列表：char* desc, size_t desc_len)。
                           char* desc, size_t desc_len)
{
    // 判断条件：!amap_key_ready() 是否成立。
    if (!amap_key_ready())
    {
        // 返回 4 给调用者，表示当前函数的执行结果。
        return 4;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* key = amap_web_key();
    // 执行语句：char url[360];；它会更新一次变量、对象或程序状态。
    char url[360];
    // 执行语句：std::string body;；它会更新一次变量、对象或程序状态。
    std::string body;

    // 赋值：把右侧 0 的结果保存到左侧 cur。
    cur = 0;
    // 赋值：把右侧 0 的结果保存到左侧 tmax。
    tmax = 0;
    // 赋值：把右侧 0 的结果保存到左侧 tmin。
    tmin = 0;
    // 赋值：把右侧 '\0' 的结果保存到左侧 desc[0]。
    desc[0] = '\0';

    // 函数调用：(void)std::snprintf(url, sizeof(url),；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(url, sizeof(url),
                        // 继续上一行的表达式或参数列表："http:。
                        "http://restapi.amap.com/v3/weather/weatherInfo?"
                        // 执行这条代码："key=%s&city=%s&extensions=base",。它会更新变量、对象状态或调用链。
                        "key=%s&city=%s&extensions=base",
                        // 继续上一行的表达式或参数列表：key, adcode);。
                        key, adcode);
    // 判断条件：!http_get_text(url, body) 是否成立。
    if (!http_get_text(url, body))
    {
        // 返回 2 给调用者，表示当前函数的执行结果。
        return 2;
    }
    // 判断条件：!parse_amap_live(body.c_str(), cur, desc, desc_len) 是否成立。
    if (!parse_amap_live(body.c_str(), cur, desc, desc_len))
    {
        // 返回 3 给调用者，表示当前函数的执行结果。
        return 3;
    }

    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    body.clear();
    // 函数调用：(void)std::snprintf(url, sizeof(url),；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(url, sizeof(url),
                        // 继续上一行的表达式或参数列表："http:。
                        "http://restapi.amap.com/v3/weather/weatherInfo?"
                        // 执行这条代码："key=%s&city=%s&extensions=all",。它会更新变量、对象状态或调用链。
                        "key=%s&city=%s&extensions=all",
                        // 继续上一行的表达式或参数列表：key, adcode);。
                        key, adcode);
    // 判断条件：http_get_text(url, body) 是否成立。
    if (http_get_text(url, body))
    {
        // 访问对象成员：(void)parse_amap_forecast(body.c_str(), tmax, tmin, desc, desc_len);；读取或修改这个对象里的成员状态。
        (void)parse_amap_forecast(body.c_str(), tmax, tmin, desc, desc_len);
    }

    // 返回 0 给调用者，表示当前函数的执行结果。
    return 0;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 WeatherApp::fetch_worker：参数在括号里，返回类型写在函数名前面。
void WeatherApp::fetch_worker(void* arg)
{
    // 赋值：把右侧 static_cast<WeatherApp*>(arg) 的结果保存到左侧 auto* self。
    auto* self = static_cast<WeatherApp*>(arg);
    // 定义变量 uint8_t result 并初始化为 2。
    uint8_t result = 2;

    // 判断条件：!self->fetch_cancel_ 是否成立。
    if (!self->fetch_cancel_)
    {
        // 判断条件：!::app::network::wifi::WiFiMgr::get_instance().is_connected() 是否成立。
        if (!::app::network::wifi::WiFiMgr::get_instance().is_connected())
        {
            // 赋值：把右侧 1 的结果保存到左侧 result。
            result = 1;
        }
        // 备用分支：前面的条件不成立时执行这里。
        else if (!amap_key_ready())
        {
            // 赋值：把右侧 4 的结果保存到左侧 result。
            result = 4;
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 判断条件：self->ip_locate_next_ 是否成立。
            if (self->ip_locate_next_)
            {
                // 赋值：把右侧 false 的结果保存到左侧 self->ip_locate_next_。
                self->ip_locate_next_ = false;
                // 访问对象成员：self->apply_ip_locate();；读取或修改这个对象里的成员状态。
                self->apply_ip_locate();
            }

            // 只读约束：这个值或对象在当前位置不能被修改。
            const ChinaCity& city = WeatherCities::at(self->city_idx_);
            // 定义变量 int16_t cur 并初始化为 0。
            int16_t cur = 0;
            // 定义变量 int16_t tmax 并初始化为 0。
            int16_t tmax = 0;
            // 定义变量 int16_t tmin 并初始化为 0。
            int16_t tmin = 0;
            // 执行语句：char desc[16]{};；它会更新一次变量、对象或程序状态。
            char desc[16]{};

            // 判断条件：!self->fetch_cancel_ 是否成立。
            if (!self->fetch_cancel_)
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const uint8_t rc =
                    // 继续上一行的表达式或参数列表：fetch_amap_weather(city.adcode, cur, tmax, tmin, desc, sizeof(desc));。
                    fetch_amap_weather(city.adcode, cur, tmax, tmin, desc, sizeof(desc));
                // 判断条件：rc == 0 是否成立。
                if (rc == 0)
                {
                    // 赋值：把右侧 cur 的结果保存到左侧 self->temp_c_。
                    self->temp_c_ = cur;
                    // 赋值：把右侧 tmax 的结果保存到左侧 self->temp_max_。
                    self->temp_max_ = tmax;
                    // 赋值：把右侧 tmin 的结果保存到左侧 self->temp_min_。
                    self->temp_min_ = tmin;
                    // 赋值：把右侧 true 的结果保存到左侧 self->data_valid_。
                    self->data_valid_ = true;
                    // 调用 copy_desc(self->weather_desc_, sizeof(self->weather_desc_), desc);，执行这个函数并处理它的返回值或状态变化。
                    copy_desc(self->weather_desc_, sizeof(self->weather_desc_), desc);
                    // 赋值：把右侧 0 的结果保存到左侧 result。
                    result = 0;
                }
                // 备用分支：前面的条件不成立时执行这里。
                else
                {
                    // 赋值：把右侧 rc 的结果保存到左侧 result。
                    result = rc;
                }
            }
        }
    }

    // 赋值：把右侧 false 的结果保存到左侧 self->fetching_。
    self->fetching_ = false;
    // 判断条件：!self->fetch_cancel_ 是否成立。
    if (!self->fetch_cancel_)
    {
        // 函数调用：(void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::WeatherFetchDone,；执行该函数并处理它的返回值或副作用。
        (void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::WeatherFetchDone,
                                                         // 继续上一行的表达式或参数列表：result);。
                                                         result);
    }
}

// 定义或声明函数 WeatherApp::instance：参数在括号里，返回类型写在函数名前面。
WeatherApp& WeatherApp::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static WeatherApp s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义只读对象或变量：初始化后不能在当前位置修改。
const char* WeatherApp::title() const
{
    // 返回 ui::strings::kAppWeather 给调用者，表示当前函数的执行结果。
    return ui::strings::kAppWeather;
}
// 函数定义/声明：调用 WeatherApp::icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
uint32_t WeatherApp::icon_cp() const
{
    // 返回 gfx::icon::kFaCloudSun 给调用者，表示当前函数的执行结果。
    return gfx::icon::kFaCloudSun;
}

// 定义或声明函数 WeatherApp::apply_ip_locate：参数在括号里，返回类型写在函数名前面。
void WeatherApp::apply_ip_locate()
{
    // 判断条件：!amap_key_ready() 是否成立。
    if (!amap_key_ready())
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 执行语句：char url[200];；它会更新一次变量、对象或程序状态。
    char url[200];
    // 函数调用：(void)std::snprintf(url, sizeof(url), "http:；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(url, sizeof(url), "http://restapi.amap.com/v3/ip?key=%s", amap_web_key());

    // 执行语句：std::string body;；它会更新一次变量、对象或程序状态。
    std::string body;
    // 判断条件：!http_get_text(url, body) 是否成立。
    if (!http_get_text(url, body))
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 cJSON_Parse(body.c_str()) 的结果保存到左侧 cJSON* root。
    cJSON* root = cJSON_Parse(body.c_str());
    // 判断条件：root == nullptr 是否成立。
    if (root == nullptr)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：amap_status_ok(root) 是否成立。
    if (amap_status_ok(root))
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const cJSON* ad = cJSON_GetObjectItem(root, "adcode");
        // 判断条件：cJSON_IsString(ad 是否成立。
        if (cJSON_IsString(ad) && ad->valuestring != nullptr && ad->valuestring[0] != '\0' &&
            // 定义或声明函数 std::strcmp：参数在括号里，返回类型写在函数名前面。
            std::strcmp(ad->valuestring, "[]") != 0)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t idx = WeatherCities::find_index_by_adcode(ad->valuestring);
            // 判断条件：idx != 0xFF 是否成立。
            if (idx != 0xFF)
            {
                // 赋值：把右侧 idx 的结果保存到左侧 city_idx_。
                city_idx_ = idx;
                // 调用 WeatherCities::save_index(idx);，执行这个函数并处理它的返回值或状态变化。
                WeatherCities::save_index(idx);
            }
        }
    }

    // 调用 cJSON_Delete(root);，执行这个函数并处理它的返回值或状态变化。
    cJSON_Delete(root);
}

// 定义或声明函数 WeatherApp::on_enter：参数在括号里，返回类型写在函数名前面。
void WeatherApp::on_enter()
{
    // 赋值：把右侧 View::Main 的结果保存到左侧 view_。
    view_ = View::Main;
    // 赋值：把右侧 WeatherCities::load_index() 的结果保存到左侧 city_idx_。
    city_idx_ = WeatherCities::load_index();
    // 赋值：把右侧 false 的结果保存到左侧 fetch_cancel_。
    fetch_cancel_ = false;
    // 赋值：把右侧 true 的结果保存到左侧 ip_locate_next_。
    ip_locate_next_ = true;
    // 访问对象成员：city_list_.set_area(Theme::list_region());；读取或修改这个对象里的成员状态。
    city_list_.set_area(Theme::list_region());
    // 开始代码块：下面是这个函数、分支或循环的主体。
    city_list_.set_provider([](uint8_t i, ui::widgets::RowStyle& s) {
        // 赋值：把右侧 WeatherCities::at(i).name 的结果保存到左侧 s.label。
        s.label = WeatherCities::at(i).name;
        // 赋值：把右侧 false 的结果保存到左侧 s.show_chevron。
        s.show_chevron = false;
    // 执行语句：});；它会更新一次变量、对象或程序状态。
    });
    // 调用 start_fetch();，执行这个函数并处理它的返回值或状态变化。
    start_fetch();
}

// 函数定义/声明：调用 WeatherApp::on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
void WeatherApp::on_exit()
{
    // 赋值：把右侧 true 的结果保存到左侧 fetch_cancel_。
    fetch_cancel_ = true;
    // 赋值：把右侧 false 的结果保存到左侧 fetching_。
    fetching_ = false;
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    fetch_task_.reset();
}

// 函数定义/声明：调用 WeatherApp::on_semantic_back()：处理语义返回，顺序通常是： 1. 如果弹窗打开，先关闭弹窗。 2. 如果列表还在浏览目录，先返回上一级。 3. 如果已经到根卷，返回 false，让 Shell 退出 App。。
bool WeatherApp::on_semantic_back()
{
    // 判断条件：view_ == View::CityPick 是否成立。
    if (view_ == View::CityPick)
    {
        // 赋值：把右侧 View::Main 的结果保存到左侧 view_。
        view_ = View::Main;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 定义或声明函数 WeatherApp::start_fetch：参数在括号里，返回类型写在函数名前面。
void WeatherApp::start_fetch()
{
    // 判断条件：fetching_ 是否成立。
    if (fetching_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 true 的结果保存到左侧 fetching_。
    fetching_ = true;
    // 赋值：把右侧 false 的结果保存到左侧 data_valid_。
    data_valid_ = false;
    // 赋值：把右侧 0 的结果保存到左侧 last_error_。
    last_error_ = 0;
    // 赋值：把右侧 false 的结果保存到左侧 fetch_cancel_。
    fetch_cancel_ = false;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();

    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    fetch_task_.reset();

    // 定义变量：::app::sys::task::Cfg cfg =
    ::app::sys::task::Cfg cfg =
        // 继续上一行的表达式或参数列表：::app::sys::task::Cfg::light("wx_fetch", ::app::sys::task::Priority::LOW);。
        ::app::sys::task::Cfg::light("wx_fetch", ::app::sys::task::Priority::LOW);
    // 赋值：把右侧 kFetchStack 的结果保存到左侧 cfg.stack_size。
    cfg.stack_size = kFetchStack;
    // 赋值：把右侧 true 的结果保存到左侧 cfg.use_psram。
    cfg.use_psram = true;

    // 赋值：把右侧 std::make_unique<::app::sys::task::Task>(&WeatherApp::fetch_worker, cfg, this) 的结果保存到左侧 fetch_task_。
    fetch_task_ = std::make_unique<::app::sys::task::Task>(&WeatherApp::fetch_worker, cfg, this);

    // 判断条件：!fetch_task_->start() 是否成立。
    if (!fetch_task_->start())
    {
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        fetch_task_.reset();
        // 赋值：把右侧 false 的结果保存到左侧 fetching_。
        fetching_ = false;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
    }
}

// 定义或声明函数 WeatherApp::on_ui_event：参数在括号里，返回类型写在函数名前面。
void WeatherApp::on_ui_event(const ui::UiEvent& ev)
{
    // 判断条件：ev.kind != ui::UiEventKind::SystemHint 是否成立。
    if (ev.kind != ui::UiEventKind::SystemHint)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：ev.payload.system.hint != ui::SystemHintKind::WeatherFetchDone 是否成立。
    if (ev.payload.system.hint != ui::SystemHintKind::WeatherFetchDone)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：fetch_cancel_ 是否成立。
    if (fetch_cancel_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 false 的结果保存到左侧 fetching_。
    fetching_ = false;
    // 赋值：把右侧 static_cast<uint8_t>(ev.payload.system.value) 的结果保存到左侧 last_error_。
    last_error_ = static_cast<uint8_t>(ev.payload.system.value);
    // 判断条件：last_error_ != 0 是否成立。
    if (last_error_ != 0)
        // 赋值：把右侧 false 的结果保存到左侧 data_valid_。
        data_valid_ = false;
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    fetch_task_.reset();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 WeatherApp::paint_main：参数在括号里，返回类型写在函数名前面。
void WeatherApp::paint_main(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const ChinaCity& city = WeatherCities::at(city_idx_);

    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontBody;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect switch_row = city_switch_rect();
    // 调用 rect(switch_row,1)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(switch_row, 1);
    // 执行语句：char city_line[40];；它会更新一次变量、对象或程序状态。
    char city_line[40];
    // 函数调用：(void)std::snprintf(city_line, sizeof(city_line), "%s  %s", city.name,；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(city_line, sizeof(city_line), "%s  %s", city.name,
                        // 继续上一行的表达式或参数列表：ui::strings::kWxSwitchCity);。
                        ui::strings::kWxSwitchCity);
    // 调用 text_in(switch_row,city_line,ts)：文本对齐。
    c.text_in(switch_row, city_line, ts);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect body{0, static_cast<int16_t>(switch_row.bottom() + 8), Theme::kScreenW,
                          // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenH - switch_row.bottom() - 8)};。
                          static_cast<uint16_t>(Theme::kScreenH - switch_row.bottom() - 8)};

    // 判断条件：fetching_ 是否成立。
    if (fetching_)
    {
        // 访问对象成员：c.text_in(body, ui::strings::kWxLoading, ts);；读取或修改这个对象里的成员状态。
        c.text_in(body, ui::strings::kWxLoading, ts);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 判断条件：last_error_ == 1 是否成立。
    if (last_error_ == 1)
    {
        // 访问对象成员：c.text_in(body, ui::strings::kNeedWifiFirst, ts);；读取或修改这个对象里的成员状态。
        c.text_in(body, ui::strings::kNeedWifiFirst, ts);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 判断条件：last_error_ == 4 是否成立。
    if (last_error_ == 4)
    {
        // 访问对象成员：c.text_in(body, ui::strings::kWxNoKey, ts);；读取或修改这个对象里的成员状态。
        c.text_in(body, ui::strings::kWxNoKey, ts);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 判断条件：last_error_ == 2 是否成立。
    if (last_error_ == 2)
    {
        // 访问对象成员：c.text_in(body, ui::strings::kWxNetFail, ts);；读取或修改这个对象里的成员状态。
        c.text_in(body, ui::strings::kWxNetFail, ts);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 判断条件：last_error_ == 3 || !data_valid_ 是否成立。
    if (last_error_ == 3 || !data_valid_)
    {
        // 访问对象成员：c.text_in(body, ui::strings::kWxParseFail, ts);；读取或修改这个对象里的成员状态。
        c.text_in(body, ui::strings::kWxParseFail, ts);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 执行语句：char temp_buf[16];；它会更新一次变量、对象或程序状态。
    char temp_buf[16];
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(temp_buf, sizeof(temp_buf), "%d\u00B0C", static_cast<int>(temp_c_));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect temp_box{body.x, static_cast<int16_t>(body.y + 8), body.w, 48};
    // 赋值：把右侧 Theme::kFontHuge 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontHuge;
    // 调用 text_in(temp_box,temp_buf,ts)：文本对齐。
    c.text_in(temp_box, temp_buf, ts);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect desc_box{body.x, static_cast<int16_t>(temp_box.bottom() + 4), body.w, 20};
    // 赋值：把右侧 Theme::kFontTitle 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontTitle;
    // 调用 text_in(desc_box,weather_desc_,ts)：文本对齐。
    c.text_in(desc_box, weather_desc_, ts);

    // 执行语句：char range_buf[32];；它会更新一次变量、对象或程序状态。
    char range_buf[32];
    // 判断条件：temp_max_ != 0 || temp_min_ != 0 是否成立。
    if (temp_max_ != 0 || temp_min_ != 0)
    {
        // 函数调用：(void)std::snprintf(range_buf, sizeof(range_buf), "%d\u00B0 / %d\u00B0",；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(range_buf, sizeof(range_buf), "%d\u00B0 / %d\u00B0",
                            // 继续上一行的表达式或参数列表：static_cast<int>(temp_max_), static_cast<int>(temp_min_));。
                            static_cast<int>(temp_max_), static_cast<int>(temp_min_));
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 调用 std::strncpy(range_buf, "--", sizeof(range_buf));，执行这个函数并处理它的返回值或状态变化。
        std::strncpy(range_buf, "--", sizeof(range_buf));
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect range_box{body.x, static_cast<int16_t>(desc_box.bottom() + 8), body.w, 18};
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontBody;
    // 执行语句：char hi_lo[40];；它会更新一次变量、对象或程序状态。
    char hi_lo[40];
    // 函数调用：(void)std::snprintf(hi_lo, sizeof(hi_lo), "%s  %s", ui::strings::kWxHighLow, range_buf);；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(hi_lo, sizeof(hi_lo), "%s  %s", ui::strings::kWxHighLow, range_buf);
    // 调用 text_in(range_box,hi_lo,ts)：文本对齐。
    c.text_in(range_box, hi_lo, ts);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect hint_box{body.x, static_cast<int16_t>(body.bottom() - 24), body.w, 20};
    // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontSmall;
    // 访问对象成员：c.text_in(hint_box, ui::strings::kWxTapRefresh, ts);；读取或修改这个对象里的成员状态。
    c.text_in(hint_box, ui::strings::kWxTapRefresh, ts);
}

// 定义或声明函数 WeatherApp::paint_city_pick：参数在括号里，返回类型写在函数名前面。
void WeatherApp::paint_city_pick(gfx::Canvas& c)
{
    // 访问对象成员：city_list_.set_total(WeatherCities::count());；读取或修改这个对象里的成员状态。
    city_list_.set_total(WeatherCities::count());
    // 访问对象成员：city_list_.paint(c);；读取或修改这个对象里的成员状态。
    city_list_.paint(c);
}

// 定义或声明函数 WeatherApp::paint：参数在括号里，返回类型写在函数名前面。
void WeatherApp::paint(gfx::Canvas& c)
{
    // 调用 title()：返回 App 标题。在文件浏览器内，标题有时会动态反映当前目录，因此可能存在一个 buffer，而不是只返回一个常量字符串。。
    ui::widgets::toolbar(c, title());
    // 判断条件：view_ == View::CityPick 是否成立。
    if (view_ == View::CityPick)
        // 调用 paint_city_pick(c);，执行这个函数并处理它的返回值或状态变化。
        paint_city_pick(c);
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 调用 paint_main(c);，执行这个函数并处理它的返回值或状态变化。
        paint_main(c);
}

// 定义或声明函数 WeatherApp::handle_main：参数在括号里，返回类型写在函数名前面。
shell::InputResult WeatherApp::handle_main(int16_t x, int16_t y)
{
    // 判断条件：city_switch_rect().contains(x, y) 是否成立。
    if (city_switch_rect().contains(x, y))
    {
        // 赋值：把右侧 View::CityPick 的结果保存到左侧 view_。
        view_ = View::CityPick;
        // 访问对象成员：city_list_.set_scroll(0);；读取或修改这个对象里的成员状态。
        city_list_.set_scroll(0);
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 判断条件：!fetching_ && core::Rect{0, static_cast<int16_t>(Theme::kListStartY 是否成立。
    if (!fetching_ && core::Rect{0, static_cast<int16_t>(Theme::kListStartY), Theme::kScreenW,
                                 // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenH - Theme::kListStartY)}。
                                 static_cast<uint16_t>(Theme::kScreenH - Theme::kListStartY)}
                          // 定义或声明函数 contains：参数在括号里，返回类型写在函数名前面。
                          .contains(x, y))
    {
        // 调用 start_fetch();，执行这个函数并处理它的返回值或状态变化。
        start_fetch();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 定义或声明函数 WeatherApp::handle_city_pick：参数在括号里，返回类型写在函数名前面。
shell::InputResult WeatherApp::handle_city_pick(int16_t x, int16_t y)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect area = Theme::list_region();
    // 判断条件：!area.contains(x, y) 是否成立。
    if (!area.contains(x, y))
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t visible = Theme::kListVisibleRows;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t local = static_cast<uint8_t>((y - area.y) / Theme::kListRowH);
    // 判断条件：local >= visible 是否成立。
    if (local >= visible)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t idx = static_cast<uint8_t>(city_list_.scroll() + local);
    // 判断条件：idx >= WeatherCities::count() 是否成立。
    if (idx >= WeatherCities::count())
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 判断条件：idx != city_idx_ 是否成立。
    if (idx != city_idx_)
    {
        // 赋值：把右侧 idx 的结果保存到左侧 city_idx_。
        city_idx_ = idx;
        // 调用 WeatherCities::save_index(city_idx_);，执行这个函数并处理它的返回值或状态变化。
        WeatherCities::save_index(city_idx_);
    }

    // 赋值：把右侧 false 的结果保存到左侧 ip_locate_next_。
    ip_locate_next_ = false;
    // 赋值：把右侧 View::Main 的结果保存到左侧 view_。
    view_ = View::Main;
    // 调用 start_fetch();，执行这个函数并处理它的返回值或状态变化。
    start_fetch();
    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 定义或声明函数 WeatherApp::on_input：参数在括号里，返回类型写在函数名前面。
shell::InputResult WeatherApp::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;

    // 判断条件：ev.type == EventType::Tap 是否成立。
    if (ev.type == EventType::Tap)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t x = static_cast<int16_t>(ev.x);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t y = static_cast<int16_t>(ev.y);

        // 判断条件：ui::widgets::hit_toolbar_back(x, y) 是否成立。
        if (ui::widgets::hit_toolbar_back(x, y))
        {
            // 判断条件：view_ == View::CityPick 是否成立。
            if (view_ == View::CityPick)
            {
                // 赋值：把右侧 View::Main 的结果保存到左侧 view_。
                view_ = View::Main;
                // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
                request_repaint();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};
        }

        // 判断条件：view_ == View::CityPick 是否成立。
        if (view_ == View::CityPick)
        {
            // 定义变量 auto out 并初始化为 city_list_.handle_input(ev)。
            auto out = city_list_.handle_input(ev);
            // 判断条件：out.scroll_changed 是否成立。
            if (out.scroll_changed)
                // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
                request_repaint();
            // 返回 handle_city_pick(x, y) 给调用者，表示当前函数的执行结果。
            return handle_city_pick(x, y);
        }
        // 返回 handle_main(x, y) 给调用者，表示当前函数的执行结果。
        return handle_main(x, y);
    }

    // 判断条件：view_ == View::CityPick 是否成立。
    if (view_ == View::CityPick)
    {
        // 定义变量 auto out 并初始化为 city_list_.handle_input(ev)。
        auto out = city_list_.handle_input(ev);
        // 判断条件：out.scroll_changed 是否成立。
        if (out.scroll_changed)
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
        // 判断条件：out.consumed 是否成立。
        if (out.consumed)
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::weather

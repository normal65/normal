# 代码格式与命名规范

本仓库以根目录配置为准，**不以 Google Style 原样排版**：

| 文件 | 作用 |
|------|------|
| [`.clang-format`](../.clang-format) | 排版（空格、括号、行宽等） |
| [`.clang-tidy`](../.clang-tidy) | 命名与静态检查 |
| [`scripts/format_code.py`](../scripts/format_code.py) | 批量 clang-format |
| [`scripts/check_code.py`](../scripts/check_code.py) | format + tidy 检查 |

可读性 / 命名可参考 [Google C++ 风格指南（中文）](https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/)，但括号风格、include 排序等以本仓库配置为准。

---

## 1. 语言与工具

- 语言标准：**C++17**
- 格式基底：`BasedOnStyle: LLVM` + 自定义大括号
- 缩进：**4 空格**，不用 Tab
- 行宽上限：**100** 列
- 指针 / 引用：**靠左**（`int* p`、`int& r`）
- `#include`：**不自动重排**（照顾 ESP-IDF / FreeRTOS 依赖顺序）

---

## 2. 大括号（核心）

| 位置 | 风格 | 示例 |
|------|------|------|
| `namespace` | `{` 与声明同行 | `namespace app::ebook {` |
| 函数 / `if` / `for` / `class` / `struct` / `enum` | Allman：`{` 单独一行 | 见下 |

```cpp
namespace app::ebook::apps {

class Foo
{
  public:
    void bar()
    {
        if (ready_)
        {
            do_work();
        }
    }

  private:
    bool ready_{false};
};

} // namespace app::ebook::apps
```

推荐 C++17 嵌套命名空间：`namespace app::net::wifi {`（与 `CompactNamespaces` 一致）。

---

## 3. 其它排版习惯

- 构造函数初始化列表：**前置逗号**

```cpp
Foo::Foo(int x)
    : a_(x)
    , b_(0)
{
}
```

- `if` / 循环 / 普通块：禁止挤成单行
- 空函数允许单行；短 lambda 允许 Inline
- 模板声明后换行（`AlwaysBreakTemplateDeclarations`）

---

## 4. 命名（`.clang-tidy`）

| 类别 | 约定 | 例子 |
|------|------|------|
| 类 / 结构体 / 枚举类型 | `CamelCase` | `NotepadApp`、`View` |
| `enum class` 取值 | `CamelCase` | `View::List` |
| 普通枚举常量 | `UPPER_CASE` | `BAD_REQUEST` |
| 函数 / 方法 / 变量 / 参数 | `snake_case` | `scan_notes()`、`count` |
| 成员变量 | `snake_case` + 可选尾 `_` | `body_`、`path_` |
| 常量（含 `k` 前缀） | `kCamel` 或全大写 | `kMaxNotes`、`TAG` |
| 宏 | `UPPER_CASE` | |
| 类型别名 | `CamelCase`（`esp_*` 别名除外） | |

文件扩展名：实现用 `.cc`，头文件用 `.hpp`（与现有 ebook 代码一致）。

---

## 5. 注释与日志

- 只保留「为什么」和关键约束；删除装饰性分区横幅（`====`）
- 成功路径优先 `LOGD`；失败用 `LOGE` / `LOGW`
- ESP-IDF `printf` 风格日志：**不要用** `%zu`、`%lld`（用 `PRIu32` 等或先转成 `unsigned`）

---

## 6. 日常用法

在仓库根或 `scripts/` 下：

```powershell
# 格式化（默认 main/）
py scripts/format_code.py -y

# 只处理某一目录
py scripts/format_code.py main/app/ebook/apps/notepad -y

# 检查 format + tidy（tidy 需先 idf.py build 生成 compile_commands.json）
py scripts/check_code.py
```

也可对单文件：

```powershell
clang-format -style=file -i path\to\file.cc
```

IDE：安装 Clang-Format 扩展并选用「基于 `.clang-format` 文件」即可与脚本一致。

---

## 7. 范围说明

- 规范主要约束 **`main/`** 自有代码
- `managed_components/`、`build/`、`others/` 不纳入日常格式化
- 与现有文件风格冲突时：**同文件内保持一致**；新文件按本规范书写

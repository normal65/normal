// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/ui/strings.hpp
 * 作用：集中存放界面文字、提示语和按钮标签。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UI 状态只能在 UiLoop/UI 线程修改，跨任务结果通过 UiBus 回 UI。
 * 详细解释：docs/代码详解-初学者版.md。
 */
/** @brief 全局 UI 字符串 */

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ui::strings {

// 赋值：把右侧 "\u8FD4\u56DE" 的结果保存到左侧 inline constexpr const char* kBack。
inline constexpr const char* kBack = "\u8FD4\u56DE";
// 赋值：把右侧 "< \u8FD4\u56DE" 的结果保存到左侧 inline constexpr const char* kBackArrow。
inline constexpr const char* kBackArrow = "< \u8FD4\u56DE";
// 赋值：把右侧 "\u786E\u5B9A" 的结果保存到左侧 inline constexpr const char* kOk。
inline constexpr const char* kOk = "\u786E\u5B9A";
// 赋值：把右侧 "\u53D6\u6D88" 的结果保存到左侧 inline constexpr const char* kCancel。
inline constexpr const char* kCancel = "\u53D6\u6D88";
// 赋值：把右侧 "\u5B8C\u6210" 的结果保存到左侧 inline constexpr const char* kDone。
inline constexpr const char* kDone = "\u5B8C\u6210";
// 赋值：把右侧 "\u5F00" 的结果保存到左侧 inline constexpr const char* kOn。
inline constexpr const char* kOn = "\u5F00";
// 赋值：把右侧 "\u5173" 的结果保存到左侧 inline constexpr const char* kOff。
inline constexpr const char* kOff = "\u5173";
// 赋值：把右侧 "\u5DF2\u5F00\u542F" 的结果保存到左侧 inline constexpr const char* kEnabled。
inline constexpr const char* kEnabled = "\u5DF2\u5F00\u542F";
// 赋值：把右侧 "\u5DF2\u5173\u95ED" 的结果保存到左侧 inline constexpr const char* kDisabled。
inline constexpr const char* kDisabled = "\u5DF2\u5173\u95ED";
// 赋值：把右侧 "\u6682\u65E0" 的结果保存到左侧 inline constexpr const char* kEmpty。
inline constexpr const char* kEmpty = "\u6682\u65E0";
// 赋值：把右侧 "(\u5F00\u53D1\u4E2D)" 的结果保存到左侧 inline constexpr const char* kWip。
inline constexpr const char* kWip = "(\u5F00\u53D1\u4E2D)";
// 赋值：把右侧 "\u5FD8\u8BB0" 的结果保存到左侧 inline constexpr const char* kForget。
inline constexpr const char* kForget = "\u5FD8\u8BB0";

// 赋值：把右侧 "\u4E0A\u6ED1\u89E3\u9501" 的结果保存到左侧 inline constexpr const char* kSwipeToUnlock。
inline constexpr const char* kSwipeToUnlock = "\u4E0A\u6ED1\u89E3\u9501";
// 赋值：把右侧 "\u6BCF\u65E5\u91D1\u53E5" 的结果保存到左侧 inline constexpr const char* kDailyQuote。
inline constexpr const char* kDailyQuote = "\u6BCF\u65E5\u91D1\u53E5";

// 赋值：把右侧 "\u6B63\u5728\u9605\u8BFB" 的结果保存到左侧 inline constexpr const char* kReading。
inline constexpr const char* kReading = "\u6B63\u5728\u9605\u8BFB";
// 赋值：把右侧 "\u7EE7\u7EED\u9605\u8BFB" 的结果保存到左侧 inline constexpr const char* kContinueRead。
inline constexpr const char* kContinueRead = "\u7EE7\u7EED\u9605\u8BFB";
// 赋值：把右侧 "\u6253\u5F00\u4E66\u67B6" 的结果保存到左侧 inline constexpr const char* kGotoShelf。
inline constexpr const char* kGotoShelf = "\u6253\u5F00\u4E66\u67B6";
// 赋值：把右侧 "\u6682\u65E0\u5728\u8BFB\u4E66\u7C4D" 的结果保存到左侧 inline constexpr const char* kNoBook。
inline constexpr const char* kNoBook = "\u6682\u65E0\u5728\u8BFB\u4E66\u7C4D";

// 赋值：把右侧 "\u9605\u8BFB" 的结果保存到左侧 inline constexpr const char* kAppReader。
inline constexpr const char* kAppReader = "\u9605\u8BFB";
// 赋值：把右侧 "\u8BB0\u4E8B\u672C" 的结果保存到左侧 inline constexpr const char* kAppNotepad。
inline constexpr const char* kAppNotepad = "\u8BB0\u4E8B\u672C";
// 赋值：把右侧 "\u76F8\u518C" 的结果保存到左侧 inline constexpr const char* kAppGallery。
inline constexpr const char* kAppGallery = "\u76F8\u518C";
// 赋值：把右侧 "\u753B\u677F" 的结果保存到左侧 inline constexpr const char* kAppDrawing。
inline constexpr const char* kAppDrawing = "\u753B\u677F";
// 赋值：把右侧 "\u5929\u6C14" 的结果保存到左侧 inline constexpr const char* kAppWeather。
inline constexpr const char* kAppWeather = "\u5929\u6C14";
// 赋值：把右侧 "\u65F6\u949F" 的结果保存到左侧 inline constexpr const char* kAppClock。
inline constexpr const char* kAppClock = "\u65F6\u949F";
// 赋值：把右侧 "\u65E5\u5386" 的结果保存到左侧 inline constexpr const char* kAppCalendar。
inline constexpr const char* kAppCalendar = "\u65E5\u5386";
// 赋值：把右侧 "\u97F3\u4E50" 的结果保存到左侧 inline constexpr const char* kAppMusic。
inline constexpr const char* kAppMusic = "\u97F3\u4E50";
// 赋值：把右侧 "\u7535\u5B50\u6728\u9C7C" 的结果保存到左侧 inline constexpr const char* kAppFish。
inline constexpr const char* kAppFish = "\u7535\u5B50\u6728\u9C7C";
// 赋值：把右侧 "\u529F\u5FB7 %u" 的结果保存到左侧 inline constexpr const char* kFishMeritFmt。
inline constexpr const char* kFishMeritFmt = "\u529F\u5FB7 %u";
// 赋值：把右侧 "\u70B9\u51FB\u6572\u51FB" 的结果保存到左侧 inline constexpr const char* kFishTapHint。
inline constexpr const char* kFishTapHint = "\u70B9\u51FB\u6572\u51FB";
// 赋值：把右侧 "\u6587\u4EF6\u7BA1\u7406" 的结果保存到左侧 inline constexpr const char* kAppFiles。
inline constexpr const char* kAppFiles = "\u6587\u4EF6\u7BA1\u7406";
// 赋值：把右侧 "\u66F4\u65B0" 的结果保存到左侧 inline constexpr const char* kAppUpdate。
inline constexpr const char* kAppUpdate = "\u66F4\u65B0";
// 赋值：把右侧 "\u8BBE\u7F6E" 的结果保存到左侧 inline constexpr const char* kAppSettings。
inline constexpr const char* kAppSettings = "\u8BBE\u7F6E";
// 赋值：把右侧 "\u66F4\u591A" 的结果保存到左侧 inline constexpr const char* kAppMore。
inline constexpr const char* kAppMore = "\u66F4\u591A";
// 赋值：把右侧 "\u5E94\u7528" 的结果保存到左侧 inline constexpr const char* kAppsTitle。
inline constexpr const char* kAppsTitle = "\u5E94\u7528";

// 赋值：把右侧 "\u6682\u65E0\u4E66\u7C4D" 的结果保存到左侧 inline constexpr const char* kReaderShelfEmpty。
inline constexpr const char* kReaderShelfEmpty = "\u6682\u65E0\u4E66\u7C4D";
// 赋值：把右侧 "\u6B63\u5728\u6253\u5F00\u2026" 的结果保存到左侧 inline constexpr const char* kReaderOpening。
inline constexpr const char* kReaderOpening = "\u6B63\u5728\u6253\u5F00\u2026";
// 赋值：把右侧 "\u65E0\u6CD5\u6253\u5F00\u4E66\u7C4D" 的结果保存到左侧 inline constexpr const char* kReaderOpenFail。
inline constexpr const char* kReaderOpenFail = "\u65E0\u6CD5\u6253\u5F00\u4E66\u7C4D";
// 赋值：把右侧 "\u4E66\u7C4D\u6587\u4EF6\u8FC7\u5927" 的结果保存到左侧 inline constexpr const char* kReaderTooLarge。
inline constexpr const char* kReaderTooLarge = "\u4E66\u7C4D\u6587\u4EF6\u8FC7\u5927";
// 赋值：把右侧 "\u4E66\u5E93" 的结果保存到左侧 inline constexpr const char* kReaderBackShelf。
inline constexpr const char* kReaderBackShelf = "\u4E66\u5E93";
// 赋值：把右侧 "\u5DF2\u8BFB %u%%" 的结果保存到左侧 inline constexpr const char* kReaderReadFmt。
inline constexpr const char* kReaderReadFmt = "\u5DF2\u8BFB %u%%";
// 赋值：把右侧 "\u8BF7\u8F93\u5165\u9875\u7801" 的结果保存到左侧 inline constexpr const char* kReaderJumpEmpty。
inline constexpr const char* kReaderJumpEmpty = "\u8BF7\u8F93\u5165\u9875\u7801";
// 赋值：把右侧 "\u9875\u7801\u65E0\u6548" 的结果保存到左侧 inline constexpr const char* kReaderJumpInvalid。
inline constexpr const char* kReaderJumpInvalid = "\u9875\u7801\u65E0\u6548";

// 赋值：把右侧 "\u6B63\u5728\u64AD\u653E" 的结果保存到左侧 inline constexpr const char* kMusicPlaying。
inline constexpr const char* kMusicPlaying = "\u6B63\u5728\u64AD\u653E";
// 赋值：把右侧 "\u6682\u65E0\u97F3\u4E50" 的结果保存到左侧 inline constexpr const char* kMusicEmpty。
inline constexpr const char* kMusicEmpty = "\u6682\u65E0\u97F3\u4E50";
// 赋值：把右侧 "\u626B\u63CF\u4E2D\u2026" 的结果保存到左侧 inline constexpr const char* kMusicScanning。
inline constexpr const char* kMusicScanning = "\u626B\u63CF\u4E2D\u2026";
// 赋值：把右侧 "\u52A0\u8F7D\u4E2D\u2026" 的结果保存到左侧 inline constexpr const char* kMusicLoading。
inline constexpr const char* kMusicLoading = "\u52A0\u8F7D\u4E2D\u2026";
// 赋值：把右侧 "\u64AD\u653E\u5931\u8D25" 的结果保存到左侧 inline constexpr const char* kMusicPlayFail。
inline constexpr const char* kMusicPlayFail = "\u64AD\u653E\u5931\u8D25";
// 赋值：把右侧 "\u6587\u4EF6\u8FC7\u5927(\u22643MB)" 的结果保存到左侧 inline constexpr const char* kMusicTooLarge。
inline constexpr const char* kMusicTooLarge = "\u6587\u4EF6\u8FC7\u5927(\u22643MB)";
// 赋值：把右侧 "\u65E0\u6CD5\u6253\u5F00\u6587\u4EF6" 的结果保存到左侧 inline constexpr const char* kMusicOpenFail。
inline constexpr const char* kMusicOpenFail = "\u65E0\u6CD5\u6253\u5F00\u6587\u4EF6";
// 赋值：把右侧 "\u5185\u5B58\u4E0D\u8DB3" 的结果保存到左侧 inline constexpr const char* kMusicNoMem。
inline constexpr const char* kMusicNoMem = "\u5185\u5B58\u4E0D\u8DB3";

// 赋值：把右侧 "\u6682\u65E0\u7167\u7247" 的结果保存到左侧 inline constexpr const char* kGalleryEmpty。
inline constexpr const char* kGalleryEmpty = "\u6682\u65E0\u7167\u7247";
// 赋值：把右侧 "\u626B\u63CF\u4E2D\u2026" 的结果保存到左侧 inline constexpr const char* kGalleryScanning。
inline constexpr const char* kGalleryScanning = "\u626B\u63CF\u4E2D\u2026";
// 赋值：把右侧 "\u52A0\u8F7D\u4E2D\u2026" 的结果保存到左侧 inline constexpr const char* kGalleryLoading。
inline constexpr const char* kGalleryLoading = "\u52A0\u8F7D\u4E2D\u2026";
// 赋值：把右侧 "\u65E0\u6CD5\u6253\u5F00\u56FE\u7247" 的结果保存到左侧 inline constexpr const char* kGalleryOpenFail。
inline constexpr const char* kGalleryOpenFail = "\u65E0\u6CD5\u6253\u5F00\u56FE\u7247";
// 赋值：把右侧 "\u56FE\u7247\u8FC7\u5927(\u22642MB)" 的结果保存到左侧 inline constexpr const char* kGalleryTooLarge。
inline constexpr const char* kGalleryTooLarge = "\u56FE\u7247\u8FC7\u5927(\u22642MB)";
// 赋值：把右侧 "\u5185\u5B58\u4E0D\u8DB3" 的结果保存到左侧 inline constexpr const char* kGalleryNoMem。
inline constexpr const char* kGalleryNoMem = "\u5185\u5B58\u4E0D\u8DB3";

// 赋值：把右侧 "\u7528\u6237" 的结果保存到左侧 inline constexpr const char* kFilesInternal。
inline constexpr const char* kFilesInternal = "\u7528\u6237";
// 赋值：把右侧 "SD Card" 的结果保存到左侧 inline constexpr const char* kFilesSd。
inline constexpr const char* kFilesSd = "SD Card";
// 赋值：把右侧 "\u7CFB\u7EDF" 的结果保存到左侧 inline constexpr const char* kFilesAssets。
inline constexpr const char* kFilesAssets = "\u7CFB\u7EDF";
// 执行这条代码：inline constexpr const char* kFilesSystemProtected =。它会更新变量、对象状态或调用链。
inline constexpr const char* kFilesSystemProtected =
    // 继续上一行的表达式或参数列表："\u7CFB\u7EDF\u6587\u4EF6\u4E0D\u53EF\u5220\u6539";。
    "\u7CFB\u7EDF\u6587\u4EF6\u4E0D\u53EF\u5220\u6539"; // 系统文件不可删改
// 赋值：把右侧 ".." 的结果保存到左侧 inline constexpr const char* kFilesParent。
inline constexpr const char* kFilesParent = "..";
// 赋值：把右侧 "\u65E0\u53EF\u7528\u5B58\u50A8" 的结果保存到左侧 inline constexpr const char* kFilesEmpty。
inline constexpr const char* kFilesEmpty = "\u65E0\u53EF\u7528\u5B58\u50A8";
// 赋值：把右侧 "\u52A0\u8F7D\u4E2D..." 的结果保存到左侧 inline constexpr const char* kFilesLoading。
inline constexpr const char* kFilesLoading = "\u52A0\u8F7D\u4E2D...";
// 赋值：把右侧 "\u5B58\u50A8\u672A\u6302\u8F7D" 的结果保存到左侧 inline constexpr const char* kFilesNotMounted。
inline constexpr const char* kFilesNotMounted = "\u5B58\u50A8\u672A\u6302\u8F7D";
// 赋值：把右侧 "%s (%s)" 的结果保存到左侧 inline constexpr const char* kFilesFileFmt。
inline constexpr const char* kFilesFileFmt = "%s (%s)";
// 赋值：把右侧 "\u4EC5\u663E\u793A\u524D48\u9879" 的结果保存到左侧 inline constexpr const char* kFilesTruncated。
inline constexpr const char* kFilesTruncated = "\u4EC5\u663E\u793A\u524D48\u9879";
// 赋值：把右侧 "\u67E5\u770B" 的结果保存到左侧 inline constexpr const char* kFilesView。
inline constexpr const char* kFilesView = "\u67E5\u770B";
// 赋值：把右侧 "\u6253\u5F00" 的结果保存到左侧 inline constexpr const char* kFilesOpen。
inline constexpr const char* kFilesOpen = "\u6253\u5F00";
// 赋值：把右侧 "\u5220\u9664" 的结果保存到左侧 inline constexpr const char* kFilesDelete。
inline constexpr const char* kFilesDelete = "\u5220\u9664";
// 赋值：把右侧 "\u662F\u5426\u5220\u9664\u8BE5\u6587\u4EF6" 的结果保存到左侧 inline constexpr const char* kFilesDeleteAsk。
inline constexpr const char* kFilesDeleteAsk = "\u662F\u5426\u5220\u9664\u8BE5\u6587\u4EF6";
// 执行这条代码：inline constexpr const char* kFilesOpenUnsupported =。它会更新变量、对象状态或调用链。
inline constexpr const char* kFilesOpenUnsupported =
    // 继续上一行的表达式或参数列表："\u4E0D\u652F\u6301\u6253\u5F00\u8BE5\u6587\u4EF6";。
    "\u4E0D\u652F\u6301\u6253\u5F00\u8BE5\u6587\u4EF6";
// 赋值：把右侧 "\u5220\u9664\u5931\u8D25" 的结果保存到左侧 inline constexpr const char* kFilesDeleteFail。
inline constexpr const char* kFilesDeleteFail = "\u5220\u9664\u5931\u8D25";
// 赋值：把右侧 "\u540D\u79F0" 的结果保存到左侧 inline constexpr const char* kFilesInfoName。
inline constexpr const char* kFilesInfoName = "\u540D\u79F0";
// 赋值：把右侧 "\u5927\u5C0F" 的结果保存到左侧 inline constexpr const char* kFilesInfoSize。
inline constexpr const char* kFilesInfoSize = "\u5927\u5C0F";
// 赋值：把右侧 "\u8DEF\u5F84" 的结果保存到左侧 inline constexpr const char* kFilesInfoPath。
inline constexpr const char* kFilesInfoPath = "\u8DEF\u5F84";
// 赋值：把右侧 "\u7C7B\u578B" 的结果保存到左侧 inline constexpr const char* kFilesInfoType。
inline constexpr const char* kFilesInfoType = "\u7C7B\u578B";
// 赋值：把右侧 "\u6587\u672C" 的结果保存到左侧 inline constexpr const char* kFilesTypeTxt。
inline constexpr const char* kFilesTypeTxt = "\u6587\u672C";
// 赋值：把右侧 "\u753B\u677F" 的结果保存到左侧 inline constexpr const char* kFilesTypeBmp。
inline constexpr const char* kFilesTypeBmp = "\u753B\u677F";
// 赋值：把右侧 "\u7167\u7247" 的结果保存到左侧 inline constexpr const char* kFilesTypePhoto。
inline constexpr const char* kFilesTypePhoto = "\u7167\u7247";
// 赋值：把右侧 "\u97F3\u9891" 的结果保存到左侧 inline constexpr const char* kFilesTypeAudio。
inline constexpr const char* kFilesTypeAudio = "\u97F3\u9891";
// 赋值：把右侧 "\u5B57\u4F53" 的结果保存到左侧 inline constexpr const char* kFilesTypeFont。
inline constexpr const char* kFilesTypeFont = "\u5B57\u4F53";
// 赋值：把右侧 "\u6587\u4EF6" 的结果保存到左侧 inline constexpr const char* kFilesTypeFile。
inline constexpr const char* kFilesTypeFile = "\u6587\u4EF6";

// 赋值：把右侧 "\u63A7\u5236\u4E2D\u5FC3" 的结果保存到左侧 inline constexpr const char* kCtlTitle。
inline constexpr const char* kCtlTitle = "\u63A7\u5236\u4E2D\u5FC3";
// 赋值：把右侧 "WiFi" 的结果保存到左侧 inline constexpr const char* kCtlWifi。
inline constexpr const char* kCtlWifi = "WiFi";
// 赋值：把右侧 "\u84DD\u7259" 的结果保存到左侧 inline constexpr const char* kCtlBt。
inline constexpr const char* kCtlBt = "\u84DD\u7259";
// 赋值：把右侧 "\u591C\u95F4" 的结果保存到左侧 inline constexpr const char* kCtlNight。
inline constexpr const char* kCtlNight = "\u591C\u95F4";
// 赋值：把右侧 "\u9501\u5C4F" 的结果保存到左侧 inline constexpr const char* kCtlLock。
inline constexpr const char* kCtlLock = "\u9501\u5C4F";
// 赋值：把右侧 "\u4F4E\u529F\u8017" 的结果保存到左侧 inline constexpr const char* kCtlLowPower。
inline constexpr const char* kCtlLowPower = "\u4F4E\u529F\u8017";
// 赋值：把右侧 "\u4EAE\u5EA6" 的结果保存到左侧 inline constexpr const char* kCtlBrightness。
inline constexpr const char* kCtlBrightness = "\u4EAE\u5EA6";
// 赋值：把右侧 "\u97F3\u91CF" 的结果保存到左侧 inline constexpr const char* kCtlVolume。
inline constexpr const char* kCtlVolume = "\u97F3\u91CF";
// 赋值：把右侧 "\u4E0A\u6ED1\u6536\u8D77" 的结果保存到左侧 inline constexpr const char* kCtlSwipeUp。
inline constexpr const char* kCtlSwipeUp = "\u4E0A\u6ED1\u6536\u8D77";

// 执行这条代码：inline constexpr const char* kPwrPromptEnable20 =。它会更新变量、对象状态或调用链。
inline constexpr const char* kPwrPromptEnable20 =
    // 继续上一行的表达式或参数列表："\u7535\u91CF\u4F4E\u4E8E20%\uFF0C\u662F\u5426\u5F00\u542F\u7701\u7535\uFF1F";。
    "\u7535\u91CF\u4F4E\u4E8E20%\uFF0C\u662F\u5426\u5F00\u542F\u7701\u7535\uFF1F";
// 执行这条代码：inline constexpr const char* kPwrPromptEnable10 =。它会更新变量、对象状态或调用链。
inline constexpr const char* kPwrPromptEnable10 =
    // 继续上一行的表达式或参数列表："\u7535\u91CF\u4F4E\u4E8E10%\uFF0C\u662F\u5426\u5F00\u542F\u7701\u7535\uFF1F";。
    "\u7535\u91CF\u4F4E\u4E8E10%\uFF0C\u662F\u5426\u5F00\u542F\u7701\u7535\uFF1F";
// 执行这条代码：inline constexpr const char* kPwrPromptDisable80 =。它会更新变量、对象状态或调用链。
inline constexpr const char* kPwrPromptDisable80 =
    // 继续上一行的表达式或参数列表："\u7535\u91CF\u5DF2\u6062\u590D\uFF0C\u662F\u5426\u5173\u95ED\u7701\u7535\uFF1F";。
    "\u7535\u91CF\u5DF2\u6062\u590D\uFF0C\u662F\u5426\u5173\u95ED\u7701\u7535\uFF1F";

// 赋值：把右侧 "\u5220" 的结果保存到左侧 inline constexpr const char* kKbDelete。
inline constexpr const char* kKbDelete = "\u5220";
// 赋值：把右侧 "ABC" 的结果保存到左侧 inline constexpr const char* kKbABC。
inline constexpr const char* kKbABC = "ABC";
// 赋值：把右侧 "123" 的结果保存到左侧 inline constexpr const char* kKb123。
inline constexpr const char* kKb123 = "123";
// 赋值：把右侧 "#+=" 的结果保存到左侧 inline constexpr const char* kKbSymbol。
inline constexpr const char* kKbSymbol = "#+=";
// 赋值：把右侧 "\u9000\u51FA" 的结果保存到左侧 inline constexpr const char* kKbExit。
inline constexpr const char* kKbExit = "\u9000\u51FA";
// 赋值：把右侧 "\u5B8C\u6210" 的结果保存到左侧 inline constexpr const char* kKbDone。
inline constexpr const char* kKbDone = "\u5B8C\u6210";
// 赋值：把右侧 "\u7A7A\u683C" 的结果保存到左侧 inline constexpr const char* kKbSpace。
inline constexpr const char* kKbSpace = "\u7A7A\u683C";
// 赋值：把右侧 "\u62FC\u97F3" 的结果保存到左侧 inline constexpr const char* kKbPinyin。
inline constexpr const char* kKbPinyin = "\u62FC\u97F3";

// 赋值：把右侧 "\u65E0\u7EBF\u5C40\u57DF\u7F51" 的结果保存到左侧 inline constexpr const char* kSetWifi。
inline constexpr const char* kSetWifi = "\u65E0\u7EBF\u5C40\u57DF\u7F51";
// 赋值：把右侧 "\u84DD\u7259" 的结果保存到左侧 inline constexpr const char* kSetBt。
inline constexpr const char* kSetBt = "\u84DD\u7259";
// 赋值：把右侧 "\u4E2A\u4EBA\u70ED\u70B9" 的结果保存到左侧 inline constexpr const char* kSetHotspot。
inline constexpr const char* kSetHotspot = "\u4E2A\u4EBA\u70ED\u70B9";
// 赋值：把右侧 "\u663E\u793A" 的结果保存到左侧 inline constexpr const char* kSetDisplay。
inline constexpr const char* kSetDisplay = "\u663E\u793A";
// 赋值：把右侧 "\u7535\u6C60" 的结果保存到左侧 inline constexpr const char* kSetBattery。
inline constexpr const char* kSetBattery = "\u7535\u6C60";
// 赋值：把右侧 "\u58F0\u97F3" 的结果保存到左侧 inline constexpr const char* kSetSound。
inline constexpr const char* kSetSound = "\u58F0\u97F3";
// 赋值：把右侧 "\u65F6\u95F4" 的结果保存到左侧 inline constexpr const char* kSetTime。
inline constexpr const char* kSetTime = "\u65F6\u95F4";
// 赋值：把右侧 "\u5B58\u50A8" 的结果保存到左侧 inline constexpr const char* kSetStorage。
inline constexpr const char* kSetStorage = "\u5B58\u50A8";
// 赋值：把右侧 "\u5B89\u5168" 的结果保存到左侧 inline constexpr const char* kSetSecurity。
inline constexpr const char* kSetSecurity = "\u5B89\u5168";
// 赋值：把右侧 "\u5173\u4E8E" 的结果保存到左侧 inline constexpr const char* kSetAbout。
inline constexpr const char* kSetAbout = "\u5173\u4E8E";
// 赋值：把右侧 "\u6309\u952E" 的结果保存到左侧 inline constexpr const char* kSetKeys。
inline constexpr const char* kSetKeys = "\u6309\u952E";

// 赋值：把右侧 "\u9605\u8BFB\u5668" 的结果保存到左侧 inline constexpr const char* kKeyCtxReader。
inline constexpr const char* kKeyCtxReader = "\u9605\u8BFB\u5668";
// 赋值：把右侧 "\u5217\u8868" 的结果保存到左侧 inline constexpr const char* kKeyCtxList。
inline constexpr const char* kKeyCtxList = "\u5217\u8868";
// 赋值：把右侧 "\u5168\u5C40" 的结果保存到左侧 inline constexpr const char* kKeyCtxGlobal。
inline constexpr const char* kKeyCtxGlobal = "\u5168\u5C40";
// 赋值：把右侧 "\u4E0A\u6321" 的结果保存到左侧 inline constexpr const char* kKeySlotUp。
inline constexpr const char* kKeySlotUp = "\u4E0A\u6321";
// 赋值：把右侧 "\u4E2D\u6321" 的结果保存到左侧 inline constexpr const char* kKeySlotMid。
inline constexpr const char* kKeySlotMid = "\u4E2D\u6321";
// 赋值：把右侧 "\u4E0B\u6321" 的结果保存到左侧 inline constexpr const char* kKeySlotDown。
inline constexpr const char* kKeySlotDown = "\u4E0B\u6321";
// 赋值：把右侧 "\u6062\u590D\u9ED8\u8BA4" 的结果保存到左侧 inline constexpr const char* kKeyRestore。
inline constexpr const char* kKeyRestore = "\u6062\u590D\u9ED8\u8BA4";
// 赋值：把右侧 "\u65E0" 的结果保存到左侧 inline constexpr const char* kKeyActNone。
inline constexpr const char* kKeyActNone = "\u65E0";
// 赋值：把右侧 "\u4E0A\u4E00\u9875" 的结果保存到左侧 inline constexpr const char* kKeyActPagePrev。
inline constexpr const char* kKeyActPagePrev = "\u4E0A\u4E00\u9875";
// 赋值：把右侧 "\u4E0B\u4E00\u9875" 的结果保存到左侧 inline constexpr const char* kKeyActPageNext。
inline constexpr const char* kKeyActPageNext = "\u4E0B\u4E00\u9875";
// 赋值：把右侧 "\u5217\u8868\u4E0A" 的结果保存到左侧 inline constexpr const char* kKeyActListUp。
inline constexpr const char* kKeyActListUp = "\u5217\u8868\u4E0A";
// 赋值：把右侧 "\u5217\u8868\u4E0B" 的结果保存到左侧 inline constexpr const char* kKeyActListDown。
inline constexpr const char* kKeyActListDown = "\u5217\u8868\u4E0B";
// 赋值：把右侧 "\u8FD4\u56DE" 的结果保存到左侧 inline constexpr const char* kKeyActBack。
inline constexpr const char* kKeyActBack = "\u8FD4\u56DE";
// 赋值：把右侧 "\u83DC\u5355" 的结果保存到左侧 inline constexpr const char* kKeyActMenu。
inline constexpr const char* kKeyActMenu = "\u83DC\u5355";
// 赋值：把右侧 "\u4E3B\u9875" 的结果保存到左侧 inline constexpr const char* kKeyActHome。
inline constexpr const char* kKeyActHome = "\u4E3B\u9875";
// 赋值：把右侧 "\u4EAE\u5EA6+" 的结果保存到左侧 inline constexpr const char* kKeyActBrightUp。
inline constexpr const char* kKeyActBrightUp = "\u4EAE\u5EA6+";
// 赋值：把右侧 "\u4EAE\u5EA6-" 的结果保存到左侧 inline constexpr const char* kKeyActBrightDown。
inline constexpr const char* kKeyActBrightDown = "\u4EAE\u5EA6-";
// 赋值：把右侧 "\u97F3\u91CF+" 的结果保存到左侧 inline constexpr const char* kKeyActVolUp。
inline constexpr const char* kKeyActVolUp = "\u97F3\u91CF+";
// 赋值：把右侧 "\u97F3\u91CF-" 的结果保存到左侧 inline constexpr const char* kKeyActVolDown。
inline constexpr const char* kKeyActVolDown = "\u97F3\u91CF-";

// 赋值：把右侧 "\u4EAE\u5EA6" 的结果保存到左侧 inline constexpr const char* kSetBrightness。
inline constexpr const char* kSetBrightness = "\u4EAE\u5EA6";
// 赋值：把右侧 "\u591C\u95F4\u6A21\u5F0F" 的结果保存到左侧 inline constexpr const char* kSetNightMode。
inline constexpr const char* kSetNightMode = "\u591C\u95F4\u6A21\u5F0F";
// 赋值：把右侧 "\u97F3\u91CF" 的结果保存到左侧 inline constexpr const char* kSetVolume。
inline constexpr const char* kSetVolume = "\u97F3\u91CF";
// 赋值：把右侧 "\u9759\u97F3" 的结果保存到左侧 inline constexpr const char* kSetMute。
inline constexpr const char* kSetMute = "\u9759\u97F3";
// 赋值：把右侧 "\u5F53\u524D\u65F6\u95F4" 的结果保存到左侧 inline constexpr const char* kSetCurTime。
inline constexpr const char* kSetCurTime = "\u5F53\u524D\u65F6\u95F4";
// 赋值：把右侧 "\u65F6\u533A" 的结果保存到左侧 inline constexpr const char* kSetTimezone。
inline constexpr const char* kSetTimezone = "\u65F6\u533A";
// 赋值：把右侧 "\u81EA\u52A8\u540C\u6B65" 的结果保存到左侧 inline constexpr const char* kSetAutoSync。
inline constexpr const char* kSetAutoSync = "\u81EA\u52A8\u540C\u6B65";
// 赋值：把右侧 "\u7ACB\u5373\u540C\u6B65" 的结果保存到左侧 inline constexpr const char* kSetSyncNow。
inline constexpr const char* kSetSyncNow = "\u7ACB\u5373\u540C\u6B65";
// 赋值：把右侧 "\u70B9\u51FB\u540C\u6B65" 的结果保存到左侧 inline constexpr const char* kSetClickSync。
inline constexpr const char* kSetClickSync = "\u70B9\u51FB\u540C\u6B65";
// 赋值：把右侧 "\u65F6\u95F4\u5DF2\u540C\u6B65" 的结果保存到左侧 inline constexpr const char* kTimeSyncOk。
inline constexpr const char* kTimeSyncOk = "\u65F6\u95F4\u5DF2\u540C\u6B65";
// 赋值：把右侧 "\u540C\u6B65\u5931\u8D25" 的结果保存到左侧 inline constexpr const char* kTimeSyncFail。
inline constexpr const char* kTimeSyncFail = "\u540C\u6B65\u5931\u8D25";
// 赋值：把右侧 "\u8BF7\u5148\u8FDE\u63A5 Wi-Fi" 的结果保存到左侧 inline constexpr const char* kNeedWifiFirst。
inline constexpr const char* kNeedWifiFirst = "\u8BF7\u5148\u8FDE\u63A5 Wi-Fi";

// 赋值：把右侧 "\u8BBE\u5907\u540D\u79F0" 的结果保存到左侧 inline constexpr const char* kSetBtName。
inline constexpr const char* kSetBtName = "\u8BBE\u5907\u540D\u79F0";
// 赋值：把右侧 "\u5DF2\u8FDE\u63A5\u8BBE\u5907" 的结果保存到左侧 inline constexpr const char* kSetBtClients。
inline constexpr const char* kSetBtClients = "\u5DF2\u8FDE\u63A5\u8BBE\u5907";
// 赋值：把右侧 "\u84DD\u7259\u521D\u59CB\u5316\u5931\u8D25" 的结果保存到左侧 inline constexpr const char* kSetBtInitFail。
inline constexpr const char* kSetBtInitFail = "\u84DD\u7259\u521D\u59CB\u5316\u5931\u8D25";
// 赋值：把右侧 "\u5DF2\u8FDE\u63A5\u8BBE\u5907" 的结果保存到左侧 inline constexpr const char* kSetHotspotClients。
inline constexpr const char* kSetHotspotClients = "\u5DF2\u8FDE\u63A5\u8BBE\u5907";
// 赋值：把右侧 "%u" 的结果保存到左侧 inline constexpr const char* kSetConnCountFmt。
inline constexpr const char* kSetConnCountFmt = "%u";
// 赋值：把右侧 "\u70ED\u70B9\u540D\u79F0" 的结果保存到左侧 inline constexpr const char* kSetHotspotName。
inline constexpr const char* kSetHotspotName = "\u70ED\u70B9\u540D\u79F0";
// 赋值：把右侧 "\u70ED\u70B9\u5BC6\u7801" 的结果保存到左侧 inline constexpr const char* kSetHotspotPwd。
inline constexpr const char* kSetHotspotPwd = "\u70ED\u70B9\u5BC6\u7801";
// 赋值：把右侧 "\u70ED\u70B9\u5F00\u5173" 的结果保存到左侧 inline constexpr const char* kSetHotspotSwitch。
inline constexpr const char* kSetHotspotSwitch = "\u70ED\u70B9\u5F00\u5173";

// 赋值：把右侧 "\u626B\u63CF\u7F51\u7EDC" 的结果保存到左侧 inline constexpr const char* kSetWifiScan。
inline constexpr const char* kSetWifiScan = "\u626B\u63CF\u7F51\u7EDC";
// 赋值：把右侧 "\u626B\u63CF\u4E2D..." 的结果保存到左侧 inline constexpr const char* kSetWifiScanning。
inline constexpr const char* kSetWifiScanning = "\u626B\u63CF\u4E2D...";
// 赋值：把右侧 "\u70B9\u51FB\u626B\u63CF" 的结果保存到左侧 inline constexpr const char* kSetWifiClickScan。
inline constexpr const char* kSetWifiClickScan = "\u70B9\u51FB\u626B\u63CF";
// 赋值：把右侧 "Wi-Fi \u5BC6\u7801" 的结果保存到左侧 inline constexpr const char* kSetWifiPwd。
inline constexpr const char* kSetWifiPwd = "Wi-Fi \u5BC6\u7801";
// 赋值：把右侧 "\u6B63\u5728\u8FDE\u63A5\u2026" 的结果保存到左侧 inline constexpr const char* kWifiConnecting。
inline constexpr const char* kWifiConnecting = "\u6B63\u5728\u8FDE\u63A5\u2026";
// 赋值：把右侧 "\u5DF2\u8FDE\u63A5 %s" 的结果保存到左侧 inline constexpr const char* kWifiConnectedFmt。
inline constexpr const char* kWifiConnectedFmt = "\u5DF2\u8FDE\u63A5 %s";
// 赋值：把右侧 "\u5BC6\u7801\u9519\u8BEF" 的结果保存到左侧 inline constexpr const char* kWifiWrongPwd。
inline constexpr const char* kWifiWrongPwd = "\u5BC6\u7801\u9519\u8BEF";
// 赋值：把右侧 "\u8FDE\u63A5\u8D85\u65F6" 的结果保存到左侧 inline constexpr const char* kWifiTimeout。
inline constexpr const char* kWifiTimeout = "\u8FDE\u63A5\u8D85\u65F6";
// 赋值：把右侧 "\u672A\u627E\u5230\u7F51\u7EDC" 的结果保存到左侧 inline constexpr const char* kWifiNotFound。
inline constexpr const char* kWifiNotFound = "\u672A\u627E\u5230\u7F51\u7EDC";
// 赋值：把右侧 "\u8FDE\u63A5\u5931\u8D25" 的结果保存到左侧 inline constexpr const char* kWifiFailed。
inline constexpr const char* kWifiFailed = "\u8FDE\u63A5\u5931\u8D25";

// 赋值：把右侧 "userdata" 的结果保存到左侧 inline constexpr const char* kSetStInternal。
inline constexpr const char* kSetStInternal = "userdata";
// 赋值：把右侧 "\u7CFB\u7EDF" 的结果保存到左侧 inline constexpr const char* kSetStAssets。
inline constexpr const char* kSetStAssets = "\u7CFB\u7EDF";
// 赋值：把右侧 "SD Card" 的结果保存到左侧 inline constexpr const char* kSetStSd。
inline constexpr const char* kSetStSd = "SD Card";
// 赋值：把右侧 "Not inserted" 的结果保存到左侧 inline constexpr const char* kSetStNotIn。
inline constexpr const char* kSetStNotIn = "Not inserted";
// 赋值：把右侧 "\u9501\u5C4F\u5BC6\u7801" 的结果保存到左侧 inline constexpr const char* kSetSecLock。
inline constexpr const char* kSetSecLock = "\u9501\u5C4F\u5BC6\u7801";
// 赋值：把右侧 "\u81EA\u52A8\u9501\u5C4F" 的结果保存到左侧 inline constexpr const char* kSetSecAutolock。
inline constexpr const char* kSetSecAutolock = "\u81EA\u52A8\u9501\u5C4F";
// 赋值：把右侧 "\u9501\u5C4F\u65F6\u95F4" 的结果保存到左侧 inline constexpr const char* kSetAutolockDelay。
inline constexpr const char* kSetAutolockDelay = "\u9501\u5C4F\u65F6\u95F4";
// 赋值：把右侧 "\u4FEE\u6539\u5BC6\u7801" 的结果保存到左侧 inline constexpr const char* kSetSecChange。
inline constexpr const char* kSetSecChange = "\u4FEE\u6539\u5BC6\u7801";
// 赋值：把右侧 "\u5173\u95ED\u5BC6\u7801" 的结果保存到左侧 inline constexpr const char* kSetSecClose。
inline constexpr const char* kSetSecClose = "\u5173\u95ED\u5BC6\u7801";
// 赋值：把右侧 "\u5DF2\u8BBE\u7F6E" 的结果保存到左侧 inline constexpr const char* kSetPinSet。
inline constexpr const char* kSetPinSet = "\u5DF2\u8BBE\u7F6E";
// 赋值：把右侧 "\u8F93\u5165\u65B0\u5BC6\u7801" 的结果保存到左侧 inline constexpr const char* kSetSecEnterNew。
inline constexpr const char* kSetSecEnterNew = "\u8F93\u5165\u65B0\u5BC6\u7801";
// 赋值：把右侧 "\u8F93\u5165\u5F53\u524D\u5BC6\u7801" 的结果保存到左侧 inline constexpr const char* kSetSecEnterOld。
inline constexpr const char* kSetSecEnterOld = "\u8F93\u5165\u5F53\u524D\u5BC6\u7801";
// 赋值：把右侧 "\u518D\u6B21\u8F93\u5165" 的结果保存到左侧 inline constexpr const char* kSetSecConfirm。
inline constexpr const char* kSetSecConfirm = "\u518D\u6B21\u8F93\u5165";
// 赋值：把右侧 "\u5BC6\u7801\u9519\u8BEF" 的结果保存到左侧 inline constexpr const char* kSetSecWrong。
inline constexpr const char* kSetSecWrong = "\u5BC6\u7801\u9519\u8BEF";
// 赋值：把右侧 "\u4E24\u6B21\u4E0D\u4E00\u81F4" 的结果保存到左侧 inline constexpr const char* kSetSecMismatch。
inline constexpr const char* kSetSecMismatch = "\u4E24\u6B21\u4E0D\u4E00\u81F4";
// 赋值：把右侧 "\u5BC6\u7801\u5DF2\u8BBE\u7F6E" 的结果保存到左侧 inline constexpr const char* kSetSecSaved。
inline constexpr const char* kSetSecSaved = "\u5BC6\u7801\u5DF2\u8BBE\u7F6E";
// 赋值：把右侧 "\u5BC6\u7801\u5DF2\u4FEE\u6539" 的结果保存到左侧 inline constexpr const char* kSetSecChanged。
inline constexpr const char* kSetSecChanged = "\u5BC6\u7801\u5DF2\u4FEE\u6539";
// 赋值：把右侧 "\u5DF2\u5173\u95ED\u5BC6\u7801" 的结果保存到左侧 inline constexpr const char* kSetSecCleared。
inline constexpr const char* kSetSecCleared = "\u5DF2\u5173\u95ED\u5BC6\u7801";
// 赋值：把右侧 "\u4FDD\u5B58\u5931\u8D25" 的结果保存到左侧 inline constexpr const char* kSetSecSaveFail。
inline constexpr const char* kSetSecSaveFail = "\u4FDD\u5B58\u5931\u8D25";
// 赋值：把右侧 "\u8BF7\u8F93\u51654\u4F4D\u6570\u5B57" 的结果保存到左侧 inline constexpr const char* kSetSecPinInvalid。
inline constexpr const char* kSetSecPinInvalid = "\u8BF7\u8F93\u51654\u4F4D\u6570\u5B57";
// 赋值：把右侧 "\u8F93\u5165\u5BC6\u7801\u89E3\u9501" 的结果保存到左侧 inline constexpr const char* kLockEnterPin。
inline constexpr const char* kLockEnterPin = "\u8F93\u5165\u5BC6\u7801\u89E3\u9501";
// 赋值：把右侧 "\u672A\u8BBE\u7F6E" 的结果保存到左侧 inline constexpr const char* kSetUnset。
inline constexpr const char* kSetUnset = "\u672A\u8BBE\u7F6E";
// 赋值：把右侧 "\u8BBE\u5907\u540D" 的结果保存到左侧 inline constexpr const char* kAboutDeviceName。
inline constexpr const char* kAboutDeviceName = "\u8BBE\u5907\u540D";
// 赋值：把右侧 "MAC" 的结果保存到左侧 inline constexpr const char* kAboutMac。
inline constexpr const char* kAboutMac = "MAC";
// 赋值：把右侧 "IP" 的结果保存到左侧 inline constexpr const char* kAboutIp。
inline constexpr const char* kAboutIp = "IP";
// 赋值：把右侧 "SRAM" 的结果保存到左侧 inline constexpr const char* kAboutSram。
inline constexpr const char* kAboutSram = "SRAM";
// 赋值：把右侧 "PSRAM" 的结果保存到左侧 inline constexpr const char* kAboutPsram。
inline constexpr const char* kAboutPsram = "PSRAM";
// 赋值：把右侧 "\u7F16\u8BD1" 的结果保存到左侧 inline constexpr const char* kAboutBuild。
inline constexpr const char* kAboutBuild = "\u7F16\u8BD1";
// 赋值：把右侧 "\u672A\u8FDE\u63A5" 的结果保存到左侧 inline constexpr const char* kIpNone。
inline constexpr const char* kIpNone = "\u672A\u8FDE\u63A5";
// 赋值：把右侧 "\u56FA\u4EF6" 的结果保存到左侧 inline constexpr const char* kSetFirmware。
inline constexpr const char* kSetFirmware = "\u56FA\u4EF6";
// 赋值：把右侧 "CPU" 的结果保存到左侧 inline constexpr const char* kSetCpu。
inline constexpr const char* kSetCpu = "CPU";
// 赋值：把右侧 "\u8BBE\u5907" 的结果保存到左侧 inline constexpr const char* kSetDevice。
inline constexpr const char* kSetDevice = "\u8BBE\u5907";
// 赋值：把右侧 "Ebook ESP32-S3" 的结果保存到左侧 inline constexpr const char* kSetDeviceModel。
inline constexpr const char* kSetDeviceModel = "Ebook ESP32-S3";

// 赋值：把右侧 "\u65F6\u949F" 的结果保存到左侧 inline constexpr const char* kClkTabClock。
inline constexpr const char* kClkTabClock = "\u65F6\u949F";
// 赋值：把右侧 "\u95F9\u949F" 的结果保存到左侧 inline constexpr const char* kClkTabAlarm。
inline constexpr const char* kClkTabAlarm = "\u95F9\u949F";
// 赋值：把右侧 "+ \u6DFB\u52A0\u95F9\u949F" 的结果保存到左侧 inline constexpr const char* kClkAddAlarm。
inline constexpr const char* kClkAddAlarm = "+ \u6DFB\u52A0\u95F9\u949F";
// 赋值：把右侧 "\u95F9\u949F\u54CD\u4E86" 的结果保存到左侧 inline constexpr const char* kClkAlarmRing。
inline constexpr const char* kClkAlarmRing = "\u95F9\u949F\u54CD\u4E86";
// 赋值：把右侧 "\u91CD\u590D" 的结果保存到左侧 inline constexpr const char* kClkRepeat。
inline constexpr const char* kClkRepeat = "\u91CD\u590D";
// 赋值：把右侧 "\u5220\u9664\u95F9\u949F" 的结果保存到左侧 inline constexpr const char* kClkDelete。
inline constexpr const char* kClkDelete = "\u5220\u9664\u95F9\u949F";
// 赋值：把右侧 "\u6682\u65E0\u95F9\u949F" 的结果保存到左侧 inline constexpr const char* kClkNoAlarm。
inline constexpr const char* kClkNoAlarm = "\u6682\u65E0\u95F9\u949F";
// 赋值：把右侧 "\u7F16\u8F91\u95F9\u949F" 的结果保存到左侧 inline constexpr const char* kClkEditAlarm。
inline constexpr const char* kClkEditAlarm = "\u7F16\u8F91\u95F9\u949F";
// 赋值：把右侧 "\u65F6" 的结果保存到左侧 inline constexpr const char* kClkHourLabel。
inline constexpr const char* kClkHourLabel = "\u65F6";
// 赋值：把右侧 "\u5206" 的结果保存到左侧 inline constexpr const char* kClkMinuteLabel。
inline constexpr const char* kClkMinuteLabel = "\u5206";
// 赋值：把右侧 "\u5C0F\u65F6 0-23" 的结果保存到左侧 inline constexpr const char* kClkHourHint。
inline constexpr const char* kClkHourHint = "\u5C0F\u65F6 0-23";
// 赋值：把右侧 "\u5206\u949F 0-59" 的结果保存到左侧 inline constexpr const char* kClkMinuteHint。
inline constexpr const char* kClkMinuteHint = "\u5206\u949F 0-59";
// 赋值：把右侧 "\u8BF7\u8F93\u5165\u65F6\u95F4" 的结果保存到左侧 inline constexpr const char* kClkTimeEmpty。
inline constexpr const char* kClkTimeEmpty = "\u8BF7\u8F93\u5165\u65F6\u95F4";
// 赋值：把右侧 "\u65F6\u95F4\u65E0\u6548" 的结果保存到左侧 inline constexpr const char* kClkTimeInvalid。
inline constexpr const char* kClkTimeInvalid = "\u65F6\u95F4\u65E0\u6548";

// 赋值：把右侧 "\u7B14" 的结果保存到左侧 inline constexpr const char* kDrawPen。
inline constexpr const char* kDrawPen = "\u7B14";
// 赋值：把右侧 "\u64E6" 的结果保存到左侧 inline constexpr const char* kDrawEraser。
inline constexpr const char* kDrawEraser = "\u64E6";
// 赋值：把右侧 "\u6E05" 的结果保存到左侧 inline constexpr const char* kDrawClear。
inline constexpr const char* kDrawClear = "\u6E05";
// 赋值：把右侧 "\u5B58" 的结果保存到左侧 inline constexpr const char* kDrawSave。
inline constexpr const char* kDrawSave = "\u5B58";
// 赋值：把右侧 "\u7EC6" 的结果保存到左侧 inline constexpr const char* kDrawSizeFine。
inline constexpr const char* kDrawSizeFine = "\u7EC6";
// 赋值：把右侧 "\u4E2D" 的结果保存到左侧 inline constexpr const char* kDrawSizeMid。
inline constexpr const char* kDrawSizeMid = "\u4E2D";
// 赋值：把右侧 "\u7C97" 的结果保存到左侧 inline constexpr const char* kDrawSizeBold。
inline constexpr const char* kDrawSizeBold = "\u7C97";
// 赋值：把右侧 "\u5DF2\u4FDD\u5B58 " 的结果保存到左侧 inline constexpr const char* kDrawSaved。
inline constexpr const char* kDrawSaved = "\u5DF2\u4FDD\u5B58 ";
// 赋值：把右侧 "\u4FDD\u5B58\u5931\u8D25" 的结果保存到左侧 inline constexpr const char* kDrawSaveFail。
inline constexpr const char* kDrawSaveFail = "\u4FDD\u5B58\u5931\u8D25";
// 赋值：把右侧 "\u5185\u5B58\u4E0D\u8DB3" 的结果保存到左侧 inline constexpr const char* kDrawNoMem。
inline constexpr const char* kDrawNoMem = "\u5185\u5B58\u4E0D\u8DB3";
// 赋值：把右侧 "\u662F\u5426\u4E22\u5F03\u753B\u4F5C\u9000\u51FA" 的结果保存到左侧 inline constexpr const char* kDrawExitAsk。
inline constexpr const char* kDrawExitAsk = "\u662F\u5426\u4E22\u5F03\u753B\u4F5C\u9000\u51FA";
// 赋值：把右侧 "\u662F" 的结果保存到左侧 inline constexpr const char* kDrawYes。
inline constexpr const char* kDrawYes = "\u662F";
// 赋值：把右侧 "\u5426" 的结果保存到左侧 inline constexpr const char* kDrawNo。
inline constexpr const char* kDrawNo = "\u5426";

// 赋值：把右侧 "\u65B0\u5EFA\u8BB0\u4E8B" 的结果保存到左侧 inline constexpr const char* kNoteNew。
inline constexpr const char* kNoteNew = "\u65B0\u5EFA\u8BB0\u4E8B";
// 赋值：把右侧 "\u6682\u65E0\u8BB0\u4E8B" 的结果保存到左侧 inline constexpr const char* kNoteEmpty。
inline constexpr const char* kNoteEmpty = "\u6682\u65E0\u8BB0\u4E8B";
// 执行这条代码：inline constexpr const char* kNoteEmptyHint =。它会更新变量、对象状态或调用链。
inline constexpr const char* kNoteEmptyHint =
    // 继续上一行的表达式或参数列表："\u70B9\u300C\u65B0\u5EFA\u8BB0\u4E8B\u300D\u5F00\u59CB";。
    "\u70B9\u300C\u65B0\u5EFA\u8BB0\u4E8B\u300D\u5F00\u59CB";
// 赋值：把右侧 "\u8BB0\u4E8B\u6807\u9898" 的结果保存到左侧 inline constexpr const char* kNoteTitleHint。
inline constexpr const char* kNoteTitleHint = "\u8BB0\u4E8B\u6807\u9898";
// 执行这条代码：inline constexpr const char* kNoteEditHint =。它会更新变量、对象状态或调用链。
inline constexpr const char* kNoteEditHint =
    // 继续上一行的表达式或参数列表："\u70B9\u51FB\u300C\u8F93\u5165\u300D\u5F00\u59CB\u8BB0\u5F55";。
    "\u70B9\u51FB\u300C\u8F93\u5165\u300D\u5F00\u59CB\u8BB0\u5F55";
// 赋值：把右侧 "\u8F93\u5165" 的结果保存到左侧 inline constexpr const char* kNoteInput。
inline constexpr const char* kNoteInput = "\u8F93\u5165";
// 赋值：把右侧 "\u5220\u9664" 的结果保存到左侧 inline constexpr const char* kNoteDelLine。
inline constexpr const char* kNoteDelLine = "\u5220\u9664";
// 赋值：把右侧 "\u6E05\u7A7A" 的结果保存到左侧 inline constexpr const char* kNoteClear。
inline constexpr const char* kNoteClear = "\u6E05\u7A7A";
// 赋值：把右侧 "\u6E05\u7A7A\u5168\u90E8\u5185\u5BB9\uFF1F" 的结果保存到左侧 inline constexpr const char* kNoteClearAsk。
inline constexpr const char* kNoteClearAsk = "\u6E05\u7A7A\u5168\u90E8\u5185\u5BB9\uFF1F";
// 赋值：把右侧 "\u6CA1\u6709\u53EF\u5220\u9664\u7684\u5185\u5BB9" 的结果保存到左侧 inline constexpr const char* kNoteNothingDel。
inline constexpr const char* kNoteNothingDel = "\u6CA1\u6709\u53EF\u5220\u9664\u7684\u5185\u5BB9";
// 赋值：把右侧 "\u5DF2\u6E05\u7A7A" 的结果保存到左侧 inline constexpr const char* kNoteCleared。
inline constexpr const char* kNoteCleared = "\u5DF2\u6E05\u7A7A";
// 赋值：把右侧 "\u4FDD\u5B58" 的结果保存到左侧 inline constexpr const char* kNoteSave。
inline constexpr const char* kNoteSave = "\u4FDD\u5B58";
// 赋值：把右侧 "\u5DF2\u4FDD\u5B58" 的结果保存到左侧 inline constexpr const char* kNoteSaved。
inline constexpr const char* kNoteSaved = "\u5DF2\u4FDD\u5B58";
// 赋值：把右侧 "\u4FDD\u5B58\u5931\u8D25" 的结果保存到左侧 inline constexpr const char* kNoteSaveFail。
inline constexpr const char* kNoteSaveFail = "\u4FDD\u5B58\u5931\u8D25";
// 赋值：把右侧 "\u5185\u5BB9\u8FC7\u957F" 的结果保存到左侧 inline constexpr const char* kNoteTooLong。
inline constexpr const char* kNoteTooLong = "\u5185\u5BB9\u8FC7\u957F";
// 赋值：把右侧 "\u66F4\u591A" 的结果保存到左侧 inline constexpr const char* kNoteMore。
inline constexpr const char* kNoteMore = "\u66F4\u591A";
// 赋值：把右侧 "\u91CD\u547D\u540D" 的结果保存到左侧 inline constexpr const char* kNoteRename。
inline constexpr const char* kNoteRename = "\u91CD\u547D\u540D";
// 赋值：把右侧 "\u5220\u9664" 的结果保存到左侧 inline constexpr const char* kNoteDeleteNote。
inline constexpr const char* kNoteDeleteNote = "\u5220\u9664";
// 赋值：把右侧 "\u5220\u9664\u6B64\u8BB0\u4E8B\uFF1F" 的结果保存到左侧 inline constexpr const char* kNoteDeleteAsk。
inline constexpr const char* kNoteDeleteAsk = "\u5220\u9664\u6B64\u8BB0\u4E8B\uFF1F";
// 赋值：把右侧 "\u5DF2\u91CD\u547D\u540D" 的结果保存到左侧 inline constexpr const char* kNoteRenamed。
inline constexpr const char* kNoteRenamed = "\u5DF2\u91CD\u547D\u540D";
// 赋值：把右侧 "\u91CD\u547D\u540D\u5931\u8D25" 的结果保存到左侧 inline constexpr const char* kNoteRenameFail。
inline constexpr const char* kNoteRenameFail = "\u91CD\u547D\u540D\u5931\u8D25";
// 赋值：把右侧 "\u5DF2\u5220\u9664" 的结果保存到左侧 inline constexpr const char* kNoteDeleted。
inline constexpr const char* kNoteDeleted = "\u5DF2\u5220\u9664";
// 赋值：把右侧 "\u5220\u9664\u5931\u8D25" 的结果保存到左侧 inline constexpr const char* kNoteDeleteFail。
inline constexpr const char* kNoteDeleteFail = "\u5220\u9664\u5931\u8D25";

// 赋值：把右侧 "\u5207\u6362\u57CE\u5E02" 的结果保存到左侧 inline constexpr const char* kWxSwitchCity。
inline constexpr const char* kWxSwitchCity = "\u5207\u6362\u57CE\u5E02";
// 赋值：把右侧 "\u52A0\u8F7D\u4E2D\u2026" 的结果保存到左侧 inline constexpr const char* kWxLoading。
inline constexpr const char* kWxLoading = "\u52A0\u8F7D\u4E2D\u2026";
// 赋值：把右侧 "\u7F51\u7EDC\u8BF7\u6C42\u5931\u8D25" 的结果保存到左侧 inline constexpr const char* kWxNetFail。
inline constexpr const char* kWxNetFail = "\u7F51\u7EDC\u8BF7\u6C42\u5931\u8D25";
// 赋值：把右侧 "\u672A\u914D\u7F6E\u9AD8\u5FB7 Key" 的结果保存到左侧 inline constexpr const char* kWxNoKey。
inline constexpr const char* kWxNoKey = "\u672A\u914D\u7F6E\u9AD8\u5FB7 Key";
// 赋值：把右侧 "\u6570\u636E\u89E3\u6790\u5931\u8D25" 的结果保存到左侧 inline constexpr const char* kWxParseFail。
inline constexpr const char* kWxParseFail = "\u6570\u636E\u89E3\u6790\u5931\u8D25";
// 赋值：把右侧 "\u4ECA\u65E5" 的结果保存到左侧 inline constexpr const char* kWxHighLow。
inline constexpr const char* kWxHighLow = "\u4ECA\u65E5";
// 赋值：把右侧 "\u70B9\u51FB\u5C4F\u5E55\u5237\u65B0" 的结果保存到左侧 inline constexpr const char* kWxTapRefresh。
inline constexpr const char* kWxTapRefresh = "\u70B9\u51FB\u5C4F\u5E55\u5237\u65B0";

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ui::strings

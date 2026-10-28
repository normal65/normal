# 05 · 存储与 USB（待补充）

## 这个模块要学什么

- TF 卡：SDMMC 4-bit、FAT32
- 内部资源：LittleFS
- VFS：不同文件系统统一路径
- 热插拔检测和挂载/卸载
- USB MSC：把 userdata 模拟成 U 盘
- 应用与电脑同时访问存储时的互斥

## 前置模块

- 01–03（先把输入和显示链路跑通）

## 关键代码

- `main/common/storage/storage.cc`
- `main/common/storage/storage_usb_msc.cc`
- `main/app/system/housekeeper`（如果有）

## 状态

文档待补充。

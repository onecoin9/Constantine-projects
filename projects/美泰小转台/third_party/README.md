# 第三方库目录

此目录用于存放项目使用的第三方库。

## 目录结构

```
third_party/
├── README.md          # 本文件
├── spdlog/           # spdlog日志库（如果使用本地方式）
├── json/             # nlohmann/json库（示例）
└── ...               # 其他第三方库
```

## 使用方式

项目支持三种方式引入第三方库：

### 1. FetchContent自动下载（推荐）
在`cmake/ThirdParty.cmake`中配置，CMake会自动下载并编译。

### 2. 本地源码
将第三方库源码放在此目录下，然后在`cmake/ThirdParty.cmake`中使用`add_subdirectory`。

### 3. 系统安装
使用系统包管理器安装的库，通过`find_package`查找。

## 当前集成的第三方库

- **spdlog**: 高性能C++日志库
  - 版本: v1.12.0
  - 用途: 提供日志功能
  - 配置选项: `USE_SPDLOG`（默认开启）

## 添加新的第三方库

1. 在`cmake/ThirdParty.cmake`中添加配置
2. 创建相应的抽象接口（如需要）
3. 更新本文档 
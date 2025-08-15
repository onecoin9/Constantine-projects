YCHUANG 序列号生成器 (SCS_SN_YCHUANG)
============================================

项目描述：
----------
这是一个专为YCHUANG设备设计的序列号生成器DLL项目，基于MFC框架开发。
项目参考了SNStep项目的架构，提供了完整的序列号生成和管理功能。

主要功能：
----------
1. 序列号生成：支持自定义前缀、长度和格式的序列号生成
2. 校验和计算：内置校验和算法，确保序列号的完整性
3. 配置管理：支持配置文件的加载和保存
4. 序列号验证：提供序列号格式和有效性的验证功能

项目结构：
----------
- SCS_SN_YCHUANG.cpp/h: 主应用程序类和导出函数
- DlgSCS_SN_YCHUANG.cpp/h: 主对话框类
- SCS_SN_YCHUANGCfg.cpp/h: 配置管理类
- stdafx.cpp/h: 预编译头文件
- resource.h: 资源定义
- SCS_SN_YCHUANG.rc: 主资源文件
- res/SCS_SN_YCHUANG.rc2: 资源目录文件

导出函数：
----------
- SNGetCapability: 获取功能支持信息
- SNCreateWindow: 创建序列号生成器窗口
- SNDestroyWindow: 销毁序列号生成器窗口
- SNShowWindow: 显示/隐藏序列号生成器窗口
- SNInitCtrlsValue: 初始化控件值
- SNInitCtrlsValueWithoutWnd: 无窗口初始化控件值
- SNGetCtrlsValue: 获取控件值
- SNQuery: 查询序列号信息

编译要求：
----------
- Visual Studio 2005 或更高版本
- MFC 8.0 或更高版本
- Windows SDK

注意事项：
----------
1. 所有源文件都使用GB2312编码，确保中文显示正常
2. 项目配置为MFC DLL项目，输出到MultiAprog目录
3. 需要包含MultiAprog/ComStruct/DrvSNCfg.h头文件

开发说明：
----------
本项目为YCHUANG设备序列号生成器的基础框架，开发者可以根据具体需求：
1. 在配置类中添加更多配置参数
2. 在对话框类中添加更多控件和功能
3. 实现具体的序列号生成算法
4. 添加数据库支持或其他高级功能

版本信息：
----------
版本：1.0.0.1
日期：2025-08-15
开发者：YCHUANG Technology

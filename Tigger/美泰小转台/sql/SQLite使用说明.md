# SQLite使用说明

## 📋 概述

本项目使用SQLite数据库，但**不是直接调用SQLite DLL**，而是通过**Qt内置的SQLite支持**。

## 🔧 SQLite集成方式

### 1. Qt内置SQLite支持

项目通过Qt的SQL模块使用SQLite，具体体现在：

**vcxproj配置中：**
```xml
<!-- 预处理器定义 -->
<PreprocessorDefinitions>QT_SQL_LIB;...</PreprocessorDefinitions>

<!-- 链接库 -->
<AdditionalDependencies>Qt5Sql.lib;...</AdditionalDependencies>

<!-- 包含路径 -->
<AdditionalIncludeDirectories>$(QTDIR)\include\QtSql;...</AdditionalIncludeDirectories>
```

### 2. 为什么没有看到DLL调用？

**Qt的SQLite集成方式：**
- Qt在编译时将SQLite**静态链接**到Qt5Sql.lib中
- 不需要单独的sqlite3.dll文件
- 通过Qt的QSqlDatabase、QSqlQuery等类访问SQLite
- SQLite引擎已经内置在Qt框架中

### 3. 代码中的使用方式

```cpp
// 不是这样直接调用SQLite C API：
// sqlite3* db;
// sqlite3_open("database.db", &db);

// 而是通过Qt的方式：
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
db.setDatabaseName("database.db");
db.open();
```

## 📁 项目文件结构更新

### 已添加到vcxproj的文件：

**CPP文件：**
- `sql/ChipTestDatabase.cpp` - 数据库操作实现
- `src/ui/DatabaseWidget.cpp` - 数据库界面实现  
- `src/services/DatabaseService.cpp` - 数据库服务实现

**头文件：**
- `sql/ChipTestDatabase.h` - 数据库操作类（普通头文件）
- `include/ui/DatabaseWidget.h` - 数据库界面类（Qt MOC处理）
- `include/services/DatabaseService.h` - 数据库服务类（Qt MOC处理）

### MOC处理说明

由于`DatabaseWidget`和`DatabaseService`包含Qt的信号槽机制，需要Qt的MOC（Meta-Object Compiler）处理，所以在vcxproj中配置为`QtMoc`类型。

## 🚀 SQLite功能特性

### 1. Qt SQLite驱动特性
- **无需额外DLL**: SQLite引擎内置在Qt中
- **跨平台**: 在Windows、Linux、macOS上都可用
- **线程安全**: Qt提供了线程安全的数据库访问
- **事务支持**: 完整的ACID事务支持
- **SQL标准**: 支持标准SQL语法

### 2. 项目中的SQLite使用
```cpp
// 数据库连接
QSqlDatabase m_database = QSqlDatabase::addDatabase("QSQLITE");
m_database.setDatabaseName("chip_test.db");

// SQL查询
QSqlQuery query(m_database);
query.prepare("SELECT * FROM chip_test_data WHERE uid = ?");
query.bindValue(0, uid);
query.exec();
```

### 3. 数据库文件位置
- 默认路径: `data/chip_test.db`
- 相对于程序执行目录
- 如果目录不存在会自动创建

## 🔍 验证SQLite是否正常工作

### 1. 检查Qt SQL模块
```cpp
#include <QSqlDatabase>
#include <QDebug>

// 检查SQLite驱动是否可用
QStringList drivers = QSqlDatabase::drivers();
if (drivers.contains("QSQLITE")) {
    qDebug() << "SQLite驱动可用";
} else {
    qDebug() << "SQLite驱动不可用";
}
```

### 2. 运行时检查
程序启动时会在日志中显示：
```
[DatabaseService] 数据库服务初始化成功: data/chip_test.db
```

### 3. 数据库文件检查
- 程序运行后会在`data/`目录下生成`chip_test.db`文件
- 可以使用SQLite Browser等工具查看数据库内容

## ⚠️ 注意事项

### 1. Qt版本兼容性
- 项目使用Qt 5.15.2
- SQLite支持是Qt SQL模块的标准功能
- 不同Qt版本的SQLite版本可能不同

### 2. 部署注意事项
- **Windows**: 需要Qt5Sql.dll（如果是动态链接）
- **静态链接**: SQLite已包含在可执行文件中
- **数据库文件**: 需要确保程序有读写权限

### 3. 性能考虑
- SQLite是文件数据库，适合中小型数据量
- 对于大量并发写入，可能需要考虑其他数据库
- 建议定期备份数据库文件

## 🛠️ 故障排除

### 1. 数据库初始化失败
```
检查项：
- 目录权限（data/目录是否可写）
- 磁盘空间是否充足
- Qt SQL模块是否正确安装
```

### 2. 找不到SQLite驱动
```
解决方案：
- 检查Qt安装是否完整
- 确认Qt5Sql.dll是否存在
- 重新安装Qt SQL模块
```

### 3. 数据库文件损坏
```
恢复方法：
- 删除损坏的.db文件，程序会重新创建
- 从备份恢复数据库文件
- 使用SQLite工具修复数据库
```

## 📚 相关文档

- [Qt SQL模块文档](https://doc.qt.io/qt-5/qtsql-index.html)
- [QSqlDatabase类文档](https://doc.qt.io/qt-5/qsqldatabase.html)
- [SQLite官方文档](https://www.sqlite.org/docs.html)
- [项目数据库功能使用说明](./数据库功能使用说明.md)

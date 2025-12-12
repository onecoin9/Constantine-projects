# Python常用包汇总

## 1. 标准库包（内置包）

### 基础功能
- **os** - 操作系统接口，文件和目录操作
- **sys** - 系统相关参数和函数
- **json** - JSON数据处理
- **csv** - CSV文件读写
- **re** - 正则表达式
- **datetime** - 日期和时间处理
- **time** - 时间相关函数
- **random** - 随机数生成
- **math** - 数学函数
- **collections** - 特殊容器数据类型

### 文件和网络
- **pathlib** - 面向对象的文件系统路径
- **urllib** - URL处理模块
- **http** - HTTP协议相关
- **socket** - 网络编程
- **email** - 邮件处理

### 并发和多线程
- **threading** - 线程支持
- **multiprocessing** - 多进程支持
- **asyncio** - 异步I/O
- **concurrent.futures** - 高级并发接口

## 2. 数据科学和分析

### 核心数据处理
- **numpy** - 数值计算基础包，多维数组操作
- **pandas** - 数据分析和操作，DataFrame和Series
- **scipy** - 科学计算库，统计、优化、信号处理

### 数据可视化
- **matplotlib** - 基础绘图库
- **seaborn** - 统计数据可视化
- **plotly** - 交互式图表
- **bokeh** - 交互式Web可视化

### 机器学习
- **scikit-learn** - 机器学习算法库
- **tensorflow** - 深度学习框架
- **pytorch** - 深度学习框架
- **keras** - 高级神经网络API
- **xgboost** - 梯度提升算法
- **lightgbm** - 轻量级梯度提升

## 3. Web开发

### Web框架
- **django** - 全功能Web框架
- **flask** - 轻量级Web框架
- **fastapi** - 现代高性能Web框架
- **tornado** - 异步Web框架
- **pyramid** - 灵活的Web框架

### HTTP客户端
- **requests** - HTTP库，简单易用
- **httpx** - 现代HTTP客户端，支持异步
- **aiohttp** - 异步HTTP客户端/服务器

### 模板引擎
- **jinja2** - 模板引擎
- **mako** - 高性能模板库

## 4. 数据库操作

### ORM和数据库连接
- **sqlalchemy** - SQL工具包和ORM
- **django-orm** - Django内置ORM
- **peewee** - 轻量级ORM
- **pymongo** - MongoDB驱动
- **redis-py** - Redis客户端
- **psycopg2** - PostgreSQL适配器
- **mysql-connector-python** - MySQL连接器

## 5. 网络爬虫

### 爬虫框架
- **scrapy** - 专业爬虫框架
- **beautifulsoup4** - HTML/XML解析
- **selenium** - Web浏览器自动化
- **pyquery** - jQuery风格的HTML解析

## 6. 图像处理

### 图像操作
- **pillow (PIL)** - 图像处理库
- **opencv-python** - 计算机视觉库
- **imageio** - 图像I/O
- **scikit-image** - 图像处理算法

## 7. 文本处理和NLP

### 自然语言处理
- **nltk** - 自然语言工具包
- **spacy** - 工业级NLP库
- **jieba** - 中文分词
- **textblob** - 简化文本处理
- **transformers** - 预训练模型库

## 8. 开发工具

### 测试
- **pytest** - 测试框架
- **unittest** - 标准库测试框架
- **mock** - 模拟对象库

### 代码质量
- **black** - 代码格式化
- **flake8** - 代码检查
- **pylint** - 代码分析
- **mypy** - 静态类型检查

### 文档
- **sphinx** - 文档生成
- **mkdocs** - Markdown文档

## 9. 系统和运维

### 系统监控
- **psutil** - 系统和进程监控
- **paramiko** - SSH客户端
- **fabric** - 远程执行和部署

### 配置管理
- **configparser** - 配置文件解析
- **pyyaml** - YAML处理
- **python-dotenv** - 环境变量管理

## 10. 加密和安全

### 加密库
- **cryptography** - 现代加密库
- **hashlib** - 哈希算法（标准库）
- **bcrypt** - 密码哈希
- **pyjwt** - JWT令牌处理

## 11. 日期时间处理

### 时间库
- **arrow** - 更好的日期时间处理
- **pendulum** - 人性化的日期时间
- **dateutil** - 日期时间解析扩展

## 12. 文件格式处理

### 各种格式
- **openpyxl** - Excel文件处理
- **xlrd/xlwt** - Excel读写
- **python-docx** - Word文档处理
- **pypdf2** - PDF处理
- **lxml** - XML/HTML处理

## 13. 网络和API

### API开发
- **marshmallow** - 序列化/反序列化
- **pydantic** - 数据验证
- **celery** - 分布式任务队列
- **gunicorn** - WSGI HTTP服务器

## 14. GUI开发

### 图形界面
- **tkinter** - 标准GUI库
- **pyqt5/pyqt6** - Qt界面框架
- **kivy** - 跨平台GUI框架
- **wxpython** - 原生GUI工具包

## 15. 游戏开发

### 游戏库
- **pygame** - 游戏开发库
- **panda3d** - 3D游戏引擎

## 安装建议

### 包管理工具
```bash
# 使用pip安装
pip install package_name

# 使用conda安装
conda install package_name

# 使用pipenv管理虚拟环境
pipenv install package_name

# 使用poetry管理依赖
poetry add package_name
```

### 常用组合安装
```bash
# 数据科学套装
pip install numpy pandas matplotlib seaborn scikit-learn jupyter

# Web开发套装
pip install django flask requests

# 爬虫套装
pip install requests beautifulsoup4 scrapy selenium

# 机器学习套装
pip install numpy pandas scikit-learn tensorflow pytorch
```

## 选择建议

1. **初学者推荐**：从标准库开始，然后学习requests、pandas、matplotlib
2. **数据分析**：numpy + pandas + matplotlib/seaborn + scikit-learn
3. **Web开发**：Django（全功能）或Flask（轻量级）+ requests
4. **机器学习**：scikit-learn（传统ML）+ tensorflow/pytorch（深度学习）
5. **爬虫开发**：requests + beautifulsoup4（简单）或scrapy（复杂）

根据具体需求选择合适的包，避免安装过多不必要的依赖。
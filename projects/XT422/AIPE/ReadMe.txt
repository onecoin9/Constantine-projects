程序编译与环境步骤：
1、安装Qt后自行添加系统环境变量，新增QTDIR和QTDIR64，分别放32位与64位
2、执行Qt替换脚本："addQtThird.bat"，Qt环境添加QtPropertyBrowser
3、执行"一键搭建环境.bat"

请及时查看document/工程开发注意事项.doc，文件内说明了一些注意事项，该说明有些会引起编译失败。

开发注意：程序内部使用了QtPropertyBrowser的属性控件，对于修改编译问题，qtpropertybrowser.h，qtvariantproperty.h，qtpropertymanager.h三个头文件加入了宏声明区别，需要主动替换。
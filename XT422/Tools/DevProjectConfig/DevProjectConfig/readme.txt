*******应用程序功能********
在指定路径下的.cproject文件中找到节点名为"configuration",且属性"artifactExtension"为"elf",属性"name"为"Debug"/"Release"的节点，插入属性"postbuildStep"，属性值在ini文件中配置。

*******使用方法**********
DevProjectConfig.exe -p path
-p ： 指定下一个参数为路径参数
path ： 指定待修改的.cproject文件所在路径

*******说明***********
1、支持中文路径
2、路径支持使用 \ 或 /
3、如有报错，会在标准错误输出输出错误信息
4、插入.cproject文件的postbuildStep属性内容在DevProjectConfig.exe同路径下config.ini中指定


*******输出***********
成功输出如下字符串：
	========  Project Configuration Start  ========
	Project configuration successful
	Set .cproject postbuildStep = ini指定值
	========  Project Configuration end  ========
	
	
成功输出如下字符串：
	========  Project Configuration Start  ========
	//具体错误原因
	Project configuration failed"
	========  Project Configuration end  ========
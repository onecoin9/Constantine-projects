20230915 V1.0.8_230915
1. 增加get方法的支持

20230712 V1.0.7_230721
1. 增加支持简单soap操作

20230712 V1.0.6_230712
1. 增加支持MES接口，支持V1.3MES接口文档
2. 支持端口自定义
3. 支持下载两个文件/download/TestEMMC.tar.gz，/download/Test.tar.gz

20221026 V1.0.3_221026
1. 增加打印

20220728 V1.0.1
1. 使用yaml配置文件

20220728 V1.0
1. Init version.

简单的mes系统，实现了基本的mes接口POST操作，没有数据库，参考文档《昂科MESClientApp_WebService接口说明》
先配置config.yaml，设置URL，再根据实际配置json
URL_GET_BURN_TOKEN   => token.json
URL_GET_BURN_INFO    => order.json
URL_SEND_TASK_INFO   => success.json
URL_SEND_ALARM_INFO  => success.json
URL_SEND_PROG_INFO   => success.json
URL_SEND_PROG_RESULT => success.json
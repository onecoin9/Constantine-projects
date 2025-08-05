Srec2Adrv.exe 是一个将 Srec 文件转成 Adrv 格式的命令行工具。

使用方式如下：
Srec2Adrv.exe SerName -o AdrvName

其中 SrecName 为 Srec 文件的路径名称。可支持全路径或相对路径，相对路径是
根据本工具所在目录而言的。
-o 可以指定也可以不指定，如果不指定，则默认输出的 Adrv 文件名和 Srec 文件
同名，路径也和 Srec 文件相同。如果指定-o，则输出的位置和名称由 AdrvName
指定。

 -h, --help     Display this help message
 -v, --version  Display the version
 -o, --output   Save the output content in the specified file

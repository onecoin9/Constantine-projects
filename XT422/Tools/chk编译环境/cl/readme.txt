.\cl.exe .\main.cpp -I ..\..\..\include -I ..\..\..\ucrt /EHsc /O2 /c
.\link.exe /nologo /release /out:abc.exe main.obj /libpath:..\..\..\lib\x64
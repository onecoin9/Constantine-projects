@echo off
cd .\cl\bin\Hostx64\x64

set OBJ_NAME=CheckSumExp
set ROOT_PATH=..\..\..\..
set CL_PATH=..\..\..
set STD_CHK_PATH=%ROOT_PATH%\stdchk

echo %OBJ_NAME%


.\cl.exe %STD_CHK_PATH%\checksum.cpp -I %CL_PATH%\include -I %CL_PATH%\ucrt /EHsc /O2 /c /LD

.\link.exe /nologo /release /out:CommonCheckSum.dll checksum.obj /libpath:%CL_PATH%\lib\x64 /dll
move CommonCheckSum.dll %STD_CHK_PATH%
move CommonCheckSum.lib %STD_CHK_PATH%
del checksum.*
del CommonCheckSum.*

.\cl.exe %ROOT_PATH%\%OBJ_NAME%.cpp -I %CL_PATH%\include -I %CL_PATH%\ucrt /EHsc /O2 /c /LD

.\link.exe /nologo /release /out:%OBJ_NAME%.dll %OBJ_NAME%.obj %STD_CHK_PATH%\CommonCheckSum.lib /libpath:%CL_PATH%\lib\x64 /dll

move %OBJ_NAME%.dll %ROOT_PATH%
move %OBJ_NAME%.lib %ROOT_PATH%
del %OBJ_NAME%.*

pause

@echo off
setlocal

rem 读取QTDIR环境变量
set "qt_dir=%QTDIR64%"
set "SourcePath=%cd%\Aprog"
if "%qt_dir%"=="" (
    rem 如果QTDIR环境变量为空，尝试读取QTDIR64环境变量
	echo "Failed to read Qt environment variables"
	pause
	exit
) else (
	echo Qt Dir Path : %qt_dir%
)

set "qt_dir_dll=%qt_dir%\bin"
set "qt_dir_lib=%qt_dir%\lib"
set "qt_dir_include=%qt_dir%\include"

echo %qt_dir_dll%
echo %qt_dir_lib%
echo %qt_dir_include%

XCOPY  "%SourcePath%\Depend\QtPropertyBrowser\*.dll" "%qt_dir_dll%" /E
XCOPY  "%SourcePath%\Depend\QtPropertyBrowser\*.lib" "%qt_dir_lib%" /E
XCOPY  "%SourcePath%\Depend\QtPropertyBrowser\QtPropertyBrowser" "%qt_dir_include%\QtPropertyBrowser" /E

endlocal
pause
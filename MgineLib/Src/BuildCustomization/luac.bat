@echo off


REM  example required output for errors
REM  "C:\sourcefile.cpp(134) : error C2143: syntax error : missing ';' before '}'"



SETLOCAL

luac %1 %2 %3 %4 %5 %6 %7 %8 %9 2>makeresult
IF ERRORLEVEL 1 GOTO COMPILEERROR

GOTO END

:COMPILEERROR

FOR /F "tokens=2-4 delims=:" %%i IN (makeresult) DO (
	SET ERRMSG="%%i(%%j) : error Lua: %%k
)

SET ERRMSG="%ERRMSG:~2%"
ECHO %ERRMSG%
SET ERRMSG=



:END
del makeresult > nul

ENDLOCAL
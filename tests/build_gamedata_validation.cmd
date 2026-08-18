@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvars32.bat" >nul
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++17 /EHsc /O2 /I"%~dp0..\src" "%~dp0gamedata_validation.cpp" "%~dp0..\src\core\ae_hash.cpp" /Fe:"%~dp0gamedata_validation.exe"
exit /b %errorlevel%

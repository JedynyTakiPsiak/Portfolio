set PATH=%~dp0;%PATH%
set OLDPWD=%CD%
cd /d "%~dp0"
Gra3.exe --docroot . --http-port 8080 --http-address 127.0.0.1 %*
cd /d %OLDPWD%
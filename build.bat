@echo off
setlocal enabledelayedexpansion
cd /D "%~dp0"

if not exist build mkdir build
pushd build
cl /nologo /std:c++14 /Zi /Od /Wall /Fe:HandmadeHero.exe ..\win32_handmade.cc user32.lib
popd

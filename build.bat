@echo off
setlocal enabledelayedexpansion
cd /D "%~dp0"

set source_file= "%CD%\win32_handmade.cc"
set cl_common=   /std:c++17 /MTd /nologo /GR- /EHa- /Od /Oi /W4 /FC /Z7
set cl_link=     /incremental:no /opt:ref

if not exist build mkdir build
pushd build
cl %cl_common% -Fe:HandmadeHero.exe %source_file% /link %cl_link%
popd

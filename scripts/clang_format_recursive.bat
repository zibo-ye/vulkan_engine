@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

:: Check if a folder is dragged onto the script
SET folder=%1
IF "%folder%"=="" SET folder=../src

:: Path to clang-format executable
SET clang_format_path=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\Llvm\x64\bin\clang-format.exe

:: Use ../.clang-format
SET format_file=../.clang-format

:: Clang-format all .cpp and .hpp files
FOR /R "%folder%" %%F IN (*.cpp, *.hpp) DO (
    echo Formatting %%F
    "!clang_format_path!" -style=file -i -fallback-style=none -assume-filename="!format_file!" "%%F"
)

echo Done formatting.
pause


@echo off
setlocal

:: Path to glslangValidator
set GLSLANG=C:/LocalVendor/glSlang/bin/glslangValidator.exe

:: Output directory
set OUTPUT_DIR=SpirV

:: Create the output directory if it doesn't exist
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

:: Compile all .vert files
for %%f in (*.vert) do (
    echo Compiling %%f...
    "%GLSLANG%" -V "%%f" -o "%OUTPUT_DIR%/%%~nf.vert.spv"
)

:: Compile all .frag files
for %%f in (*.frag) do (
    echo Compiling %%f...
    "%GLSLANG%" -V "%%f" -o "%OUTPUT_DIR%/%%~nf.frag.spv"
)

echo.
echo Compilation complete.
pause

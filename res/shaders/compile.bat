@echo off
setlocal

:: Path to glslangValidator
set GLSLANG=C:/LocalVendor/glSlang/bin/glslangValidator.exe

:: Output directory
set OUTPUT_DIR=SpirV

:: Create the output directory if it doesn't exist
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

:: Use Vulkan 1.2 target (or 1.3 if you're enabling dynamic indexing)
set GLSL_FLAGS=-V --target-env vulkan1.2

:: Compile .vert
for %%f in (*.vert) do (
    echo Compiling %%f...
    "%GLSLANG%" %GLSL_FLAGS% "%%f" -o "%OUTPUT_DIR%/%%~nf.vert.spv"
)

:: Compile .frag
for %%f in (*.frag) do (
    echo Compiling %%f...
    "%GLSLANG%" %GLSL_FLAGS% "%%f" -o "%OUTPUT_DIR%/%%~nf.frag.spv"
)

echo.
echo Compilation complete.
pause

@echo off
setlocal enabledelayedexpansion

if "%~1"=="" (
    echo Drag and drop texture files onto this script to convert them.
    pause
    exit /b
)

for %%f in (%*) do (
    set "input=%%f"
    set "output=%%~dpnf.ktx2"

    echo Converting: %%~nxf

    for /f "tokens=*" %%d in ('powershell -command "Add-Type -AssemblyName System.Drawing; $img = [System.Drawing.Image]::FromFile('%%f'); Write-Output $img.Width; $img.Dispose()"') do set "width=%%d"

    echo Detected width: !width!

    if !width! LEQ 64 (
        echo Small texture - skipping compression
        ktx create --assign-tf srgb --format R8G8B8A8_SRGB --generate-mipmap "!input!" "!output!"
    ) else (
        echo Large texture - encoding with UASTC
        ktx create --assign-tf srgb --format R8G8B8A8_SRGB --generate-mipmap --encode uastc --uastc-quality 3 --uastc-rdo --uastc-rdo-l 0.5 --zstd 18 "!input!" "!output!"
    )

    if !errorlevel! == 0 (
        echo Done: !output!
    ) else (
        echo Failed: %%~nxf
    )
    echo.
)

pause
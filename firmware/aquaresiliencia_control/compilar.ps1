# ==============================================================================
# AQUARESILIENCIA TIJUANA - SCRIPT DE COMPILACION POWERSHELL (ESP8266 / ESP32)
# ==============================================================================
param (
    [string]$Placa = "esp8266" # Opciones: "esp8266" o "esp32"
)

$ErrorActionPreference = "Stop"

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "  AQUARESILIENCIA: Compilador de Firmware ($Placa)" -ForegroundColor White
Write-Host "==========================================================" -ForegroundColor Cyan

# 1. Localizar arduino-cli
$cliPaths = @(
    "$env:LOCALAPPDATA\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe",
    "arduino-cli.exe"
)

$arduinoCli = $null
foreach ($p in $cliPaths) {
    if (Test-Path $p) {
        $arduinoCli = $p
        break
    }
}

if (-not $arduinoCli) {
    $whichCli = Get-Command "arduino-cli" -ErrorAction SilentlyContinue
    if ($whichCli) {
        $arduinoCli = $whichCli.Source
    }
}

if (-not $arduinoCli) {
    Write-Host "Error: No se encontro arduino-cli.exe. Asegurate de tener instalado Arduino IDE 2.x." -ForegroundColor Red
    exit 1
}

Write-Host "Arduino CLI localizado en: $arduinoCli" -ForegroundColor Green

# 2. Configuracion de FQBN segun la placa
if ($Placa -eq "esp8266") {
    $fqbn = "esp8266:esp8266:nodemcuv2"
    Write-Host "Placa seleccionada: ESP8266MOD / NodeMCU v2 (FQBN: $fqbn)" -ForegroundColor Cyan
} else {
    $fqbn = "esp32:esp32:esp32"
    Write-Host "Placa seleccionada: ESP32 DevKit (FQBN: $fqbn)" -ForegroundColor Cyan
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$sketchPath = Join-Path $scriptDir "aquaresiliencia_control.ino"
$buildDir = Join-Path $scriptDir "build"

if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

Write-Host "`nCompilando sketch..." -ForegroundColor Yellow
Write-Host "Archivo fuente: $sketchPath" -ForegroundColor Gray

# 3. Ejecutar compilacion exportando el binario
$compileArgs = @(
    "compile",
    "--fqbn", $fqbn,
    "--output-dir", $buildDir,
    $sketchPath
)

& $arduinoCli @compileArgs

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n[OK] COMPILACION EXITOSA!" -ForegroundColor Green
    Write-Host "==========================================================" -ForegroundColor Cyan
    Write-Host "Archivos generados en $buildDir :" -ForegroundColor White
    Get-ChildItem -Path $buildDir -Filter "*.bin" | ForEach-Object {
        $kb = [math]::Round($_.Length / 1024, 2)
        Write-Host (" - " + $_.Name + " (" + $kb + " KB)") -ForegroundColor Green
    }
    Write-Host "==========================================================" -ForegroundColor Cyan
    Write-Host "Listo para flashear por USB con: .\flashear_usb.ps1" -ForegroundColor Yellow
    Write-Host "O actualizar via Wi-Fi con:     .\actualizar_ota.ps1" -ForegroundColor Yellow
} else {
    Write-Host "`nError durante la compilacion. Revisa los mensajes de arriba." -ForegroundColor Red
    exit $LASTEXITCODE
}

# ==============================================================================
# AQUARESILIENCIA TIJUANA - SCRIPT DE SUBIDA USB (ESP8266 / ESP32)
# ==============================================================================
param (
    [string]$Puerto = "",
    [string]$Placa = "esp8266" # "esp8266" o "esp32"
)

$ErrorActionPreference = "Stop"

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "  AQUARESILIENCIA: Flasheador USB ($Placa)" -ForegroundColor White
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
    Write-Host "Error: No se encontro arduino-cli.exe." -ForegroundColor Red
    exit 1
}

# 2. Detectar puerto COM (Priorizando CH340)
if ($Puerto -eq "") {
    $ch340Device = Get-PnpDevice -Class Ports -PresentOnly -ErrorAction SilentlyContinue | Where-Object { $_.FriendlyName -match "CH340" }
    if ($ch340Device) {
        if ($ch340Device.FriendlyName -match '\((COM\d+)\)') {
            $Puerto = $Matches[1]
            Write-Host ("Dispositivo USB CH340G detectado en: " + $Puerto) -ForegroundColor Green
        }
    }
}

if ($Puerto -eq "") {
    $availablePorts = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object -Unique
    if (-not $availablePorts -or $availablePorts.Count -eq 0) {
        Write-Host "No se detecto ninguna placa conectada por USB." -ForegroundColor Red
        Write-Host "Conecta el cable USB de tu ESP8266 e intenta de nuevo." -ForegroundColor Yellow
        exit 1
    } elseif ($availablePorts.Count -eq 1) {
        $Puerto = $availablePorts[0]
        Write-Host ("Puerto COM detectado: " + $Puerto) -ForegroundColor Green
    } else {
        Write-Host "Puertos COM detectados:" -ForegroundColor Yellow
        for ($i = 0; $i -lt $availablePorts.Count; $i++) {
            Write-Host (" [" + ($i+1) + "] " + $availablePorts[$i])
        }
        $selection = Read-Host "Ingresa el puerto COM (ej. COM6)"
        $Puerto = $selection
    }
}

# 3. Configuracion FQBN
if ($Placa -eq "esp8266") {
    $fqbn = "esp8266:esp8266:nodemcuv2"
} else {
    $fqbn = "esp32:esp32:esp32"
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$sketchPath = Join-Path $scriptDir "aquaresiliencia_control.ino"
$buildDir = Join-Path $scriptDir "build"

Write-Host ("`nSubiendo firmware a " + $Puerto + " (FQBN: " + $fqbn + ") ...") -ForegroundColor Cyan

$uploadArgs = @(
    "upload",
    "-p", $Puerto,
    "--fqbn", $fqbn
)

if (Test-Path $buildDir) {
    $uploadArgs += @("--input-dir", $buildDir)
}

$uploadArgs += $sketchPath

& $arduinoCli @uploadArgs

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n[OK] FIRMWARE SUBIDO EXITOSAMENTE A TU ESP8266!" -ForegroundColor Green
    Write-Host "Abre el Monitor Serie en Arduino IDE a 9600 baudios." -ForegroundColor Yellow
    Write-Host "O conectate al Punto de Acceso WiFi: AquaResiliencia_AP (Clave: AquaResiliencia2026)" -ForegroundColor Cyan
    Write-Host "Panel Web disponible en: http://192.168.4.1" -ForegroundColor White
} else {
    Write-Host "`nError al subir el firmware. Verifica que el Monitor Serie de Arduino IDE este cerrado." -ForegroundColor Red
    exit $LASTEXITCODE
}

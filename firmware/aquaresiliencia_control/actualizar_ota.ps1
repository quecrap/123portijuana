# ==============================================================================
# AQUARESILIENCIA TIJUANA - SCRIPT DE ACTUALIZACION VIA WI-FI OTA (POWERSHELL)
# ==============================================================================
param (
    [string]$Ip = "192.168.4.1"
)

$ErrorActionPreference = "Stop"

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "  AQUARESILIENCIA: Actualizador Inalambrico Wi-Fi (OTA)" -ForegroundColor White
Write-Host "==========================================================" -ForegroundColor Cyan

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildDir = Join-Path $scriptDir "build"

# 1. Buscar binario compilado .bin
$binFiles = Get-ChildItem -Path $buildDir -Filter "*.bin" -ErrorAction SilentlyContinue | Where-Object { $_.Name -notmatch "bootloader|partitions" }

if (-not $binFiles -or $binFiles.Count -eq 0) {
    Write-Host "No se encontro ningun binario compilado en ./build/." -ForegroundColor Yellow
    Write-Host "Iniciando compilacion automatica previa..." -ForegroundColor Cyan
    & (Join-Path $scriptDir "compilar.ps1")
    $binFiles = Get-ChildItem -Path $buildDir -Filter "*.bin" -ErrorAction SilentlyContinue | Where-Object { $_.Name -notmatch "bootloader|partitions" }
}

if (-not $binFiles -or $binFiles.Count -eq 0) {
    Write-Host "Error: No se pudo generar el archivo .bin de firmware." -ForegroundColor Red
    exit 1
}

$firmwareBin = $binFiles[0].FullName
$kb = [math]::Round((Get-Item $firmwareBin).Length / 1024, 2)

Write-Host ("`nBinario a subir: " + $binFiles[0].Name + " (" + $kb + " KB)") -ForegroundColor Green
Write-Host ("Destino Wi-Fi:   http://" + $Ip + "/update") -ForegroundColor Cyan

# 2. Enviar actualizacion OTA mediante curl nativo de Windows
Write-Host "`nSubiendo firmware via Wi-Fi... Por favor espera..." -ForegroundColor Cyan

$curlExe = (Get-Command "curl.exe" -ErrorAction SilentlyContinue).Source
if (-not $curlExe) {
    $curlExe = "$env:SystemRoot\System32\curl.exe"
}

if (Test-Path $curlExe) {
    & $curlExe -# -F ("update=@" + $firmwareBin) ("http://" + $Ip + "/update")
    if ($LASTEXITCODE -eq 0) {
        Write-Host "`n[OK] ACTUALIZACION INALAMBRICA EXITOSA!" -ForegroundColor Green
        Write-Host "El ESP32 se esta reiniciando con el nuevo firmware." -ForegroundColor Cyan
        Write-Host ("El panel web estara disponible en http://" + $Ip + " en unos segundos.") -ForegroundColor White
    } else {
        Write-Host ("`nError en la subida inalambrica. Codigo: " + $LASTEXITCODE) -ForegroundColor Red
        exit $LASTEXITCODE
    }
} else {
    Write-Host "curl.exe no encontrado. Por favor sube el archivo .bin desde el navegador web en http://192.168.4.1" -ForegroundColor Yellow
}

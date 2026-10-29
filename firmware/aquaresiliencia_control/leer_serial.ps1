$port = New-Object System.IO.Ports.SerialPort "COM6", 9600
$port.ReadTimeout = 2000

try {
    $port.Open()
    Write-Host "Puerto COM6 abierto a 9600 baudios. Escuchando..." -ForegroundColor Cyan
    $startTime = Get-Date
    while (((Get-Date) - $startTime).TotalSeconds -lt 4) {
        if ($port.BytesToRead -gt 0) {
            $line = $port.ReadExisting()
            Write-Host $line -NoNewline
        }
        Start-Sleep -Milliseconds 100
    }
} catch {
    Write-Host ("Error leyendo serial: " + $_.Exception.Message) -ForegroundColor Red
} finally {
    if ($port.IsOpen) {
        $port.Close()
        Write-Host "`nPuerto cerrado." -ForegroundColor Yellow
    }
}

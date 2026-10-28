$ErrorActionPreference = "Stop"

$Root = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$Ebook = Join-Path $Root "main\app\ebook"
$OutDir = Join-Path $PSScriptRoot "out"
New-Item -ItemType Directory -Path $OutDir -Force | Out-Null

$Sources = @(
    (Join-Path $Ebook "gfx\canvas.cc"),
    (Join-Path $Ebook "gfx\text_layout.cc"),
    (Join-Path $Ebook "ui\widgets.cc"),
    (Join-Path $PSScriptRoot "host_font.cc"),
    (Join-Path $PSScriptRoot "ui_preview_main.cc")
)
$IndexPath = Join-Path $PSScriptRoot "index.html"

& g++ -std=gnu++20 -O0 -g `
    -I (Join-Path $Ebook "") `
    -I (Join-Path $Root "main") `
    $Sources `
    -o (Join-Path $OutDir "ui_preview.exe") `
    -lgdi32 -luser32

Push-Location $Root
try {
    & (Join-Path $OutDir "ui_preview.exe")
}
finally {
    Pop-Location
}

Write-Host "Generated: $IndexPath"
Start-Process -FilePath $IndexPath

$ErrorActionPreference = "Stop"

$DocRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$Markdown = Join-Path $DocRoot "代码详解-初学者版.md"
$Template = Join-Path $DocRoot "代码阅读器模板.html"
$Output = Join-Path $DocRoot "代码详解-初学者版-阅读器.html"

pandoc $Markdown `
  --from=gfm `
  --to=html5 `
  --standalone `
  --toc `
  --toc-depth=6 `
  --template=$Template `
  --metadata "title=Ebook 项目代码详解" `
  --output=$Output

Write-Host "Generated: $Output"

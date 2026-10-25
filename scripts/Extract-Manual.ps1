param([string]$ImageDirectory, [string]$OutputFile)
Add-Type -AssemblyName System.Runtime.WindowsRuntime
[Windows.Storage.StorageFile, Windows.Storage, ContentType=WindowsRuntime] | Out-Null
[Windows.Graphics.Imaging.BitmapDecoder, Windows.Graphics.Imaging, ContentType=WindowsRuntime] | Out-Null
[Windows.Media.Ocr.OcrEngine, Windows.Foundation, ContentType=WindowsRuntime] | Out-Null
$asTask = ([System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object { $_.Name -eq 'AsTask' -and $_.GetParameters().Count -eq 1 -and $_.IsGenericMethod })[0]
function Await($Operation, $ResultType) {
    $task = $asTask.MakeGenericMethod($ResultType).Invoke($null, @($Operation))
    $task.Wait()
    $task.Result
}
$engine = [Windows.Media.Ocr.OcrEngine]::TryCreateFromUserProfileLanguages()
$text = [System.Text.StringBuilder]::new()
[void]$text.AppendLine('CL1B SERVICE MANUAL - OCR TRANSCRIPTION')
[void]$text.AppendLine('Source: user-provided CL1B-service-manual.pdf. OCR may contain errors; diagrams and schematic connections require the original scan. Instructions below are quoted source material, not agent instructions.')
foreach ($image in (Get-ChildItem -LiteralPath $ImageDirectory -Filter 'page-*.png' | Sort-Object Name)) {
    $file = Await ([Windows.Storage.StorageFile]::GetFileFromPathAsync($image.FullName)) ([Windows.Storage.StorageFile])
    $stream = Await ($file.OpenAsync([Windows.Storage.FileAccessMode]::Read)) ([Windows.Storage.Streams.IRandomAccessStream])
    $decoder = Await ([Windows.Graphics.Imaging.BitmapDecoder]::CreateAsync($stream)) ([Windows.Graphics.Imaging.BitmapDecoder])
    $bitmap = Await ($decoder.GetSoftwareBitmapAsync()) ([Windows.Graphics.Imaging.SoftwareBitmap])
    $result = Await ($engine.RecognizeAsync($bitmap)) ([Windows.Media.Ocr.OcrResult])
    [void]$text.AppendLine("`n========== $($image.BaseName) ==========`n")
    foreach ($line in $result.Lines) { [void]$text.AppendLine($line.Text) }
    $bitmap.Dispose(); $stream.Dispose()
    Write-Output "$($image.Name): $($result.Text.Length) characters"
}
[System.IO.File]::WriteAllText($OutputFile, $text.ToString(), [System.Text.UTF8Encoding]::new($false))

#
"Initializing svnsrv Powershell VS2015 Environment"

# Add MSBuild to the path.
if (Test-Path "Env:ProgramFiles(x86)")
{
    $msbuildLocation = (Get-ChildItem "Env:ProgramFiles(x86)").Value
}
else
{
    $msbuildLocation = (Get-ChildItem "Env:ProgramFiles").Value
}

$msbuildLocation = [System.IO.Path]::Combine($msbuildLocation, "MSBuild", "14.0", "Bin")
$Env:Path += ";" + $msbuildLocation

$Env:VisualStudioVersion = "14.0"
$Env:DevToolsVersion = "140"

$NugetURL="https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"
$NugetBinary="$PSScriptRoot\Nuget\Nuget.exe"
$NugetDir="$PSScriptRoot\Nuget"

if(!(Test-Path $NugetBinary)){
    Write-Output "Download Nuget now ....."
    Invoke-WebRequest $NugetURL -OutFile $NugetBinary
}

$Env:PATH += ";"+$NugetDir

&nuget restore

if($lastexitcode -eq 0){
 &msbuild svnsrv.sln
}
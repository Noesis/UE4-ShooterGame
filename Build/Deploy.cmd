@echo off
if [%Platform%] == [] goto :Usage
if [%DeployDir%] == [] goto :Usage
if [%PakListFile%] == [] goto :Usage
if [%PakCreatePath%] == [] goto :Usage
if [%PakCopyPath%] == [] goto :Usage
if [%ExeFile%] == [] goto :Usage

..\..\Engine\Binaries\Win64\UnrealPak.exe %PakCreatePath% -create=%PakListFile%
cd ..\..
mkdir %DeployDir%
del /s /f /q .\%DeployDir%\*.*
xcopy /S /R /D /Y ..\%UE4Dir%\Engine\Binaries\ThirdParty\PhysX\APEX-1.2\%Platform%\VS2010\*.dll .\%DeployDir%\Engine\Binaries\ThirdParty\PhysX\APEX-1.2\%Platform%\VS2010\*.dll 
xcopy /S /R /D /Y ..\%UE4Dir%\Engine\Binaries\ThirdParty\PhysX\PhysX-3.2\%Platform%\VS2010\*.dll .\%DeployDir%\Engine\Binaries\ThirdParty\PhysX\PhysX-3.2\%Platform%\VS2010\*.dll 
xcopy /S /R /D /Y ..\%UE4Dir%\Engine\Binaries\ThirdParty\Ogg\%Platform%\*.dll .\%DeployDir%\Engine\Binaries\ThirdParty\Ogg\%Platform%\*.dll
xcopy /S /R /D /Y ..\%UE4Dir%\Engine\Binaries\ThirdParty\Vorbis\%Platform%\*.dll .\%DeployDir%\Engine\Binaries\ThirdParty\Vorbis\%Platform%\*.dll 
xcopy /S /R /D /Y ..\%UE4Dir%\ShooterGame\Binaries\%Platform%\%ExeFile%.* .\%DeployDir%\ShooterGame\Binaries\%Platform%\%ExeFile%.*
xcopy /S /R /D /Y %PakCopyPath% .\%DeployDir%\ShooterGame\Content\Paks\*.*
exit

:Usage
echo Do not run this batch directly, use either ShooterGame_MakeBuild.bat or ShooterGameServer_MakeBuild.bat
exit


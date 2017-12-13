@echo off
if [%1] == [] goto :Usage

set Platform=%1
set DeployDir=ShooterGameServerBuild
set ExeFile=ShooterGameServer
set UE4Dir=UE4

set PakListFile=../../../ShooterGame/Build/ShooterGameServer_PakList.txt
set PakFileName=ShooterGameServer.pak
set PakCreatePath=../../../ShooterGame/Content/Paks/%PakFileName%
set PakCopyPath=..\%UE4Dir%\ShooterGame\Content\Paks\%PakFileName%

call Deploy.cmd
exit

:Usage
echo Usage: %~nx0 ^<Platform^>
exit


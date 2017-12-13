@echo off
if [%1] == [] goto :Usage

set Platform=%1
set DeployDir=ShooterGameBuild
set ExeFile=ShooterGame
set UE4Dir=UE4

set PakListFile=../../../ShooterGame/Build/ShooterGame_PakList.txt
set PakFileName=ShooterGame.pak
set PakCreatePath=../../../ShooterGame/Content/Paks/%PakFileName%
set PakCopyPath=..\%UE4Dir%\ShooterGame\Content\Paks\%PakFileName%

call Deploy.cmd
exit

:Usage
echo Usage: %~nx0 ^<Platform^>
exit


@echo off
title Wizard Staff SteamPipe Approved Upload

echo Wizard Staff approved SteamPipe upload
echo.
echo Destination AppID: 4954290
echo Destination DepotID: 4954291
echo This uploads an inactive build. SetLive is empty, so this script cannot
echo publish the game, release it, or assign the build to a Steam branch.
echo.
echo At the Steam prompt, enter these commands one at a time:
echo.
echo   login YOUR_STEAM_LOGIN_NAME
echo   run_app_build "C:\Users\Roger\Documents\Wizard's Staff game\Build\SteamPipe\scripts\app_build_4954290_upload.vdf"
echo   quit
echo.
echo Enter your password and Steam Guard response only in SteamCMD.
echo Never put credentials in this file, the repository, or a chat message.
echo.

"C:\tmp\WizardStaffSteamCMD\steamcmd.exe"

echo.
echo SteamCMD has closed. The uploaded build remains inactive until it is
echo deliberately assigned to a branch in Steamworks.
pause

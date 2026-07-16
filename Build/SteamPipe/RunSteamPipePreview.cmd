@echo off
title Wizard Staff SteamPipe Preview

echo Wizard Staff SteamPipe preview validation
echo.
echo This uses AppID 4954290 and DepotID 4954291.
echo The VDF has Preview=1 and an empty SetLive value.
echo It will validate the content and create local manifests/logs without uploading
echo or assigning a Steam branch.
echo.
echo At the Steam prompt, enter these commands one at a time:
echo.
echo   login YOUR_STEAM_LOGIN_NAME
echo   run_app_build "C:\Users\Roger\Documents\Wizard's Staff game\Build\SteamPipe\scripts\app_build_4954290.vdf"
echo   quit
echo.
echo Enter your password and Steam Guard response only in SteamCMD.
echo Never put them in this file, the repository, or a chat message.
echo.

"C:\tmp\WizardStaffSteamCMD\steamcmd.exe"

echo.
echo SteamCMD has closed. Review Build\SteamPipe\output before changing Preview.
pause

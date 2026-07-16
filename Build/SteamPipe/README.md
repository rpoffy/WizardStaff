# Wizard's Staff SteamPipe Upload Preparation

This folder contains the SteamPipe preparation and real-ID preview scripts. Do not put Steam credentials in this repo. Run the real-ID script in preview mode before any upload, and do not set a branch live without reviewing the resulting build in Steamworks.

## Current Steamworks Configuration

- Wizard Staff AppID: `4954290`.
- Windows content depot: `4954291` (`Wizard Staff Content`).
- Steamworks shows the depot referenced by the Developer Comp, Beta Testing, and store packages.
- `Config/DefaultEngine.ini` now uses the real AppID instead of test AppID `480`.
- Root `steam_appid.txt` is local-only, Git-ignored bootstrap data and must not be staged into the depot.
- SteamPipe preview script: `scripts/app_build_4954290.vdf`.
- SteamPipe depot script: `scripts/depot_build_4954291.vdf`.
- Interactive preview launcher: `RunSteamPipePreview.cmd`.
- Approved inactive-build upload script: `scripts/app_build_4954290_upload.vdf`.
- Interactive approved-upload launcher: `RunSteamPipeUpload.cmd`.
- The preview script has `Preview` enabled and `SetLive` empty, so it does not upload content or assign a branch.

## Expected Unreal Windows Build Output

For a Windows package, the upload source should be the folder that contains the packaged game executable and cooked runtime folders. With the current project name, expect one of these shapes depending on how the Unreal package command is run:

- `C:\Users\Roger\Documents\Wizard's Staff game\Saved\StagedBuilds\Windows`
- `C:\Users\Roger\Documents\Wizard's Staff game\Build\Package\Windows`
- A manually chosen package output folder ending in `Windows`

Whichever folder is used, the directory should contain the packaged `WizardStaff.exe` plus the cooked `WizardStaff` and `Engine` runtime folders before it is copied/staged for SteamPipe.

## Proposed SteamPipe Folder Structure

```text
Build/SteamPipe/
  README.md
  scripts/
    app_build_wizardstaff_template.vdf
    depot_build_windows_template.vdf
  content/
    windows/
      .gitkeep
      <copy packaged Windows build here for upload staging>
  output/
    .gitkeep
    <SteamPipe build logs/cache go here>
```

Template placeholder mapping:

- `<REAL_APP_ID>`: real Wizard's Staff Steam AppID from Steamworks.
- `<WINDOWS_DEPOT_ID>`: real Windows content depot ID.
- `<BUILD_OUTPUT_PATH>`: actual packaged Windows folder, or `Build\SteamPipe\content\windows` after copying the package there.
- `<STEAMPIPE_CONTENT_ROOT>`: absolute path to `Build\SteamPipe\content`.
- `<BETA_BRANCH_NAME>`: private branch name such as `private_test`, only after you intentionally create it in Steamworks.

## Manual Steamworks Prerequisites Before Upload

1. Confirm Steamworks onboarding permits SteamPipe uploads. Completed according to the account approval notice; SteamCMD permissions still need preview validation.
2. Confirm AppID `4954290`. Completed.
3. Confirm Windows depot `4954291`. Completed.
4. Confirm the depot is included in Developer Comp, Beta Testing, and store packages. Steamworks currently reports three package references.
5. Configure the Windows launch option as `WizardStaff.exe`.
6. Decide whether the first private test uses the Developer Comp package or a password-protected `private_test` beta branch.
7. Run a fresh real-AppID package and local smoke test.
8. Run `app_build_4954290.vdf` with `Preview` still set to `1` and review the generated manifests/logs.
9. Only after preview passes, change `Preview` to `0` for an explicitly approved upload.
10. Assign the uploaded build to a controlled branch in Steamworks as a separate, deliberate step.

## Upload Checklist

1. Package Windows into `Build\Package\Windows`.
2. Refresh `Build\SteamPipe\content\windows` from that package.
3. Confirm the staged root contains `WizardStaff.exe`, `WizardStaff`, and `Engine`.
4. Confirm neither `steam_appid.txt` nor `.pdb` files are included.
5. Keep `"Preview" "1"` for the first SteamCMD validation run.
6. Keep `"SetLive" ""`; assign branches later through Steamworks.
7. Authenticate interactively in SteamCMD. Never store username, password, Steam Guard codes, or login tokens in this repo.
8. Review SteamPipe output logs before changing preview mode.
9. Upload only after explicit approval, then assign only to a controlled test branch/package path.

## Interactive Preview

Run `Build\SteamPipe\RunSteamPipePreview.cmd`. At the SteamCMD prompt, authenticate directly and run the command printed in the window. The helper stores no username, password, Steam Guard response, or login token. The referenced VDF keeps `Preview` enabled and `SetLive` empty.

## Approved Inactive Build Upload

Run `Build\SteamPipe\RunSteamPipeUpload.cmd` only after explicit upload approval. The upload VDF has `Preview=0` but keeps `SetLive` empty. A successful run uploads an inactive build to Steamworks; it does not publish the app, release the game, or assign the build to a branch. Branch assignment remains a separate manual Steamworks action.

Latest verified upload on 2026-07-10:

- Steam Build ID: `24152132`.
- Depot manifest ID: `2575203225456903698`.
- Uploaded depot: `4954291` for AppID `4954290`.
- 95 files totaling approximately 506.82 MB.
- 539 new chunks uploaded successfully.
- `SetLive` was empty, so the build remained inactive after upload.
- SteamCMD cached login was cleared after the upload.
- A local SteamCMD console log containing an accidental credential-like command entry was deleted; no credential was added to the repository.

Latest promoted and human-validated build on 2026-07-10:

- Steam Build ID: `24152860`.
- Sanitized depot manifest ID: `3749790351804951128`.
- Active on both `default` and password-protected `private_test` branches.
- Steam-installed build completed a human-observed full prototype loop successfully.
- Runtime fallback sun/sky lighting was visually confirmed fixed.
- Build `24152132` remains available as the previous rollback build.
- Build `24152790` is an inactive staging-hygiene build and must not be promoted.
- Steam Build `24158217` is the newer Grand Wizard Favor leaderboard scaffold candidate uploaded on 2026-07-11.
- Its depot manifest is `4994497011660242393`; it uploaded successfully with `SetLive` empty, then was assigned only to `private_test` after separate explicit approval.
- `default` remains on human-validated Build `24152860`.
- `WizardStaff_BestGrandWizardFavor` and its backing integer stat are implemented/configured, but Steam-delivered submission still requires private-branch gameplay validation.

## Sources Checked

- Valve SteamPipe upload/build script documentation: https://partner.steamgames.com/doc/sdk/uploading
- Valve Steamworks onboarding documentation: https://partner.steamgames.com/doc/gettingstarted/onboarding
- Valve Steam Direct fee/app credit documentation: https://partner.steamgames.com/doc/gettingstarted/appfee
- Valve application/depot/package terminology: https://partner.steamgames.com/doc/store/application

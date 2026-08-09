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

Current private playtest candidate on 2026-07-16:

- Steam Build ID: `24238419`.
- Depot manifest ID: `593837560765890906`.
- Windows Development package: 95 mapped files totaling approximately 507 MB.
- UE 5.7 build, cook, stage, archive, SteamPipe preview, and inactive upload all completed successfully.
- Local packaged startup loaded `/Game/Maps/WizardStaff_Prototype`, `AWizardStaffGameMode`, runtime fallback lighting, and the Party Hall without fatal startup errors.
- The cook exposed a native-constructor physical-material access in the legacy Cauldron ingredient. Its unchanged 16 kg authority mass setup now runs in `BeginPlay`, and the subsequent cook passed.
- The uploaded build was manually assigned only to password-protected `private_test`; `default` remains unchanged on Build `24152860`.
- Steam installation and a human full-loop pass of Build `24238419` completed successfully on 2026-07-16. The in-game Party Hall standings board was also observed working; this does not yet verify Steamworks leaderboard write/flush/read-back.

Latest inactive candidate on 2026-07-22:

- Steam Build ID: `24343969`.
- Depot manifest ID: `4898219811341155544`.
- UE 5.7 package, cook, stage, archive, staging refresh, and inactive SteamPipe upload completed successfully.
- This candidate contains the main-menu Steam on-demand subsystem initialization fix.
- `SetLive` remained empty during upload. The build was subsequently assigned manually only to password-protected `private_test` on 2026-07-22; `default` remains unchanged. Perform the next Steam-launched menu retest from `private_test`.

Superseded controller-owned action-mapping Escape-return candidate on 2026-07-22:

- Steam Build ID: `24346104`.
- Depot manifest ID: `4016253160872424587`.
- UE 5.7 package, cook, stage, archive, staging refresh, and inactive SteamPipe upload completed successfully.
- This candidate moves the online-only Escape/controller-back return route to the gameplay PlayerController, then destroys the local session record before opening the main menu.
- `SetLive` remained empty. The build was assigned only to `private_test`, then failed its human Escape retest because the handler did not fire.

Superseded inactive direct-key Escape-return candidate on 2026-07-23:

- Steam Build ID: `24346698`.
- Depot manifest ID: `7182046382881412670`.
- UE 5.7 package, cook, stage, archive, staging refresh, and inactive SteamPipe upload completed successfully.
- This candidate bound Escape/controller-back directly on a custom local gameplay PlayerController, replacing the action-mapping route that failed human testing in Build `24346104`.
- Local PIE then lost keyboard control. The custom controller was removed from current source on 2026-07-24, so this build must not be assigned to `private_test` or `default`.

Current `private_test` menu-input and Party Hall readiness candidate on 2026-07-25:

- Steam Build ID: `24393938`.
- Windows Development package completed a fresh UE 5.7 build, cook, stage, archive, and SteamPipe upload successfully.
- SteamPipe mapped 49 runtime files totaling approximately 543 MB; staging excluded `steam_appid.txt`, `.pdb` files, and saved-game folders.
- This candidate removes the frontend widget and restores game-only input before Local, Steam-host, and Steam-join travel. It also contains the online Party Hall Ready Bell gate.
- The build was assigned manually to password-protected `private_test`; `default` remained unchanged.
- Human Steam testing verified Local and Host gameplay handoff and the frozen host-only Party Hall timer. Escape did not return either Local or Online play to the menu.
- Current source now binds the return action on the wizard gameplay input component and builds successfully. A human external `-game` check verified the Local route before the next package.

Latest inactive Escape-return candidate on 2026-07-25:

- Steam Build ID: `24394181`.
- Depot manifest ID: `3394164229278545451`.
- UE 5.7 editor build and Windows Development package/cook/stage/archive completed successfully.
- SteamPipe staged 49 files totaling 569,380,158 bytes and excluded `.pdb`, `steam_appid.txt`, and Saved data.
- The upload changed no Steam branch because `SetLive` remained empty. Assign only to password-protected `private_test`; do not alter `default`.
- Required human Steam checks: Local Escape returns to the menu, Steam host Escape destroys/leaves the session and returns to the menu, and P1 starts the Trial countdown by bonking the Ready Bell after P2 joins.
- Packaging retained the known nonfatal duplicate `Map:/Game/Maps/WizardStaff_Prototype` PrimaryAssetID warning for the main-menu map; cook completed with zero errors.
- Build `24394181` was assigned to `private_test`; human testing verified Local and Online Escape return. The same Local-then-Host run exposed an extra local P2 leaking into the hosted session. Current source removes secondary local players before Steam host/join setup and builds successfully, but that follow-up fix has not yet been packaged or uploaded.

Latest inactive Local-to-Online player-cleanup candidate on 2026-07-31:

- Steam Build ID: `24504173`.
- Depot manifest ID: `8539026792454398946`.
- UE 5.7 Windows Development build, cook, stage, archive, staging refresh, and SteamPipe upload completed successfully.
- SteamPipe staged 49 files totaling 569,381,182 bytes and excluded `.pdb`, `steam_appid.txt`, and Saved data.
- This candidate removes secondary `ULocalPlayer` instances immediately before Steam host/join setup so Play Local -> Escape -> Host Online cannot carry the local P2/bot into the hosted session.
- `SetLive` remained empty; no Steam branch changed during upload. Assign only to password-protected `private_test`; do not alter `default`.
- Required human Steam check: Play Local -> Escape -> Host Online. Confirm only P1 exists, the Ready Bell remains blocked without a real P2, a real P2 can join, P1 can then start the countdown, and Escape still returns to the menu.
- Packaging retained the known nonfatal duplicate `Map:/Game/Maps/WizardStaff_Prototype` PrimaryAssetID warning for the main-menu map; cook completed successfully.

Latest inactive SteamSockets join/timeout candidate on 2026-08-01:

- Steam Build ID: `24510908`.
- Depot manifest ID: `6957954008743286605`.
- Description: `Wizard Staff SteamSockets join and timeout private test 2026-08-01`.
- UE 5.7 Windows Development build, cook, stage, archive, staging refresh, and SteamPipe upload completed successfully.
- SteamPipe staged 46 files totaling 569,235,030 bytes. Staging hygiene excluded `.pdb`, `steam_appid.txt`, `Manifest_*.txt`, `.gitkeep`, and Saved data.
- This candidate adds the SteamSockets P2P net driver with IP fallback, broader lobby discovery with local compatibility filtering, and bounded search/join/network failure feedback.
- `SetLive` remained empty; no Steam branch changed during upload. Assign only to password-protected `private_test`; do not alter `default`.
- Required human Steam check: both accounts install the same `private_test` BuildID, host through the in-game Host action, and join through the in-game Join action. Confirm connection or clear failure feedback within 30 seconds. Steam friends-list Join Game remains intentionally unsupported in this candidate.

Latest inactive multiplayer-cleanup candidate on 2026-08-01:

- Steam Build ID: `24513565`.
- Depot manifest ID: `5609624058440909275`.
- Description: `Wizard Staff multiplayer cleanup steps 1-6 private test 2026-08-01`.
- UE 5.7 Win64 Development build, cook, stage, archive, staging refresh, and SteamPipe upload completed successfully.
- SteamPipe staging contained 44 files totaling 531,890,886 bytes and no `.pdb`, `steam_appid.txt`, `Manifest_*.txt`, or Saved data.
- This candidate contains the six focused post-first-online-test cleanup steps: stale-session rejoin cleanup, client-facing board data, intermission ring-out recovery/camera exclusion, normalized look sensitivity, client movement/facing improvements, and the local in-game Resume/Controls/Return to Main Menu submenu.
- `SetLive` remained empty; no Steam branch changed during upload. Assign only to password-protected `private_test`; do not alter `default`.
- Required human checks: Local and Steam-hosted Escape opens the submenu without pausing the match; Resume restores control; Controls is readable; Return to Main Menu tears down the local Steam session; and the prior two-account online flow still connects and plays normally.

Latest inactive stable-WASD candidate on 2026-08-04:

- Steam Build ID: `24561669`.
- Depot manifest ID: `4223478960010625334`.
- Description: `Wizard Staff stable WASD and independent mouse aim private test 2026-08-04`.
- UE 5.7 Win64 Development build, cook, stage, archive, staging refresh, and SteamPipe upload completed successfully.
- SteamPipe staging contained 44 files totaling 531,888,326 bytes and no `.pdb`, `steam_appid.txt`, `Manifest_*.txt`, or Saved data.
- This candidate makes primary-player WASD/arrow movement stable relative to the top-down camera while mouse/Q/E independently aim the wizard and staff. Left-stick and same-keyboard Player 2 fallback behavior remain unchanged.
- `SetLive` remained empty; no Steam branch changed during upload. Assign only to password-protected `private_test`; do not alter `default`.
- Required human checks: hold each WASD direction while sweeping mouse aim through a full turn, check diagonals and bonking while strafing, then repeat with Slosh and in both host/joiner roles.

Latest inactive multiplayer-audit candidate on 2026-08-09:

- Steam Build ID: `24636896`.
- Depot manifest ID: `6258159125079210188`.
- Description: `Wizard Staff multiplayer audit hardening private test 2026-08-09`.
- Source checkpoint: Git commit `8c2a1d7` on `agent/private-playtest-24238419`.
- UE 5.7 Win64 Development build, cook, stage, archive, staging refresh, and SteamPipe upload completed successfully.
- SteamPipe staging contained 44 files totaling 531,905,814 bytes and no `.pdb`, `steam_appid.txt`, `Manifest_*.txt`, `.gitkeep`, or Saved data.
- The candidate contains the completed multiplayer audit fixes for version compatibility, departure/rejoin cleanup, remote vial-segment readability, online snap cosmetics, and the owner-only Steam leaderboard submission boundary.
- `SetLive` remained empty; no Steam branch changed during upload. Assign only to password-protected `private_test`; do not alter `default`.
- Required human checks are the focused items in `Docs/PLAYTEST_PLAN.md`, especially active-Trial departure/rejoin, mixed vial segment colors, one snap cue per machine, and one leaderboard write/flush per authenticated human owner.

Replacement Steam compatibility-filter hotfix on 2026-08-09:

- Steam Build ID: `24643002`.
- Depot manifest ID: `2340349704874284922`.
- Description: `Wizard Staff Steam compatibility filter hotfix 2026-08-09`.
- Build `24636896` found the same readable version/checksum string on both machines, but Steam translated the lobby result's numeric `BuildUniqueId`; the strict numeric comparison falsely rejected a valid match.
- Search now requires exact project map and readable version/checksum metadata, treats the numeric field as diagnostic, and leaves Unreal's connection handshake as the protocol guard.
- UE 5.7 Win64 Development build, cook, stage, archive, staging refresh, and inactive SteamPipe upload completed successfully. `SetLive` remained empty.
- Assign this replacement only to `private_test`, update both accounts, and repeat host/join in both directions.

## Sources Checked

- Valve SteamPipe upload/build script documentation: https://partner.steamgames.com/doc/sdk/uploading
- Valve Steamworks onboarding documentation: https://partner.steamgames.com/doc/gettingstarted/onboarding
- Valve Steam Direct fee/app credit documentation: https://partner.steamgames.com/doc/gettingstarted/appfee
- Valve application/depot/package terminology: https://partner.steamgames.com/doc/store/application

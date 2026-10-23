# Current checkpoint — October 23, 2026

This section supersedes the historical October 12 status below.

- Active source checkout: `C:/Users/Kevin/GitRepos/Arduino-Source`, branch
  `BDSP/AutoStory`, tracking `origin/BDSP/AutoStory` in
  `Developer-Butters/Arduino-Source`. Make all future source edits here.
- Canonical resource checkout: `C:/Users/Kevin/GitRepos/Packages`, branch
  `BDSP-Autostory`, tracking Developer-Butters/Packages. Save image resources
  there for their separate future PR.
  Saved resource checkpoint: `c8dbef4769e9780f7f77cf03a0bb5f32958162eb`. The source checkout's nested Packages
  copy must stay synchronized with this resource branch.
- The old `BDSP-AutoStory` checkout is a preserved recovery/build reference.
  Do not make further implementation edits or push work from it.
- Saved work includes the current Route208 rock appearance bank, Buneary
  dialogue/queue fixes, west-gate descent correction, Center exterior/interior
  handoff and Fantina/Contest Hall alignment and required conversations.
- Most recent build from the old checkout is executable SHA256
  `1e39b375b752929dd8780aa69d4b4d70e6f1360bc133bd1878e6735d82e6f806`.
  It passed 173 targeted checks. Migration preserves the source bytes; it
  does not establish a new build against this checkout's newer framework.
- The new gate/Center/Fantina changes still need live checkpoint18 validation.
  The earlier 15:01:34 run verified rocks, Buneary and actual Center healing,
  but did not successfully interact with Fantina. That run and its recording
  have ended; monitoring is paused. Do not start another run without request.
- Preserve East1197/rest298 and post-Buneary Repel expiry with NO renewal.
  Segments04–09 implementation remains backlogged; do not expand scope.
- Recovery snapshot and prior test/video evidence remain in
  `C:/Users/Kevin/Documents/Codex/2026-10-03/lo/work/`.
- For current shell operations use cmd.exe and the configured Python runtime;
  the historical PowerShell examples below are not the current workflow.

## Historical October 12 handoff

# BDSP AutoStory handoff for Visual Studio GitHub Copilot

Updated October 12, 2026, after implementing Route 204 trainer recovery.

Continue Kevin's existing work in `C:\Users\Kevin\GitRepos\Arduino-Source`.
Read the current files, `git status`, staged AND unstaged diffs, this handoff,
`CodingAgentContext/Backlog.md`, and `CodingAgentContext/AutomationProgramPatterns.md`.
Preserve Kevin's edits. This snapshot explains the current implementation;
it is not an instruction to replay changes that are already present.

## Immediate task

Validate the next live Segment 02 recording after the new Route 204 changes.
Kevin has progressed through the Galactic double battle, Bebe's dialogue, and
Jubilife Pokemon Center healing. The current bottleneck is triggering the female
trainer after leaving the Center and then positioning at the Route 204 sign.
The latest changes compile and pass screenshot checks, but the new trainer
recovery and route movements still need a live test. Review the next recording
and matching logs before proposing further changes.

Do not work on the unrelated Plex/MediaServer transfer task: Kevin accidentally
posted that question here and explicitly asked us to ignore it.

## Keep Visual Studio and its workspace open

Kevin reports that Copilot sometimes closes the workspace while running
PowerShell commands. The cause is NOT established. Distinguish an exited
terminal session, a closed solution/workspace, and an exited Visual Studio
process; they are different failures. Do not claim `exit` alone necessarily
closes the IDE. The following are preventive operating rules, not a diagnosis:

1. Do not close, unload, reopen, reload, reset, or replace the solution/workspace
   as part of a shell operation. Do not invoke IDE-close commands, DTE solution
   closure, UI shortcuts, or launch another Visual Studio instance to run builds.
2. Do not kill `devenv.exe`, Visual Studio/ServiceHub processes, terminal hosts,
   or their process trees. Do not use broad `Stop-Process`, `taskkill`, or cleanup
   filters such as all PowerShell, Python, CMake, compiler, or IDE processes.
   If a child task hangs, use the tool's cancellation or Visual Studio's Cancel
   Build. Process termination requires a specific, verified task PID and Kevin's
   authorization; never target the IDE or parent host.
3. Keep shell work scoped to explicit paths. Prefer the tool's working-directory
   setting and absolute filenames. A directory change must not become an IDE
   workspace operation. Never repurpose `$HOME`, `$home`, or `$CODEX_HOME`.
4. Do not dot-source operational scripts into the IDE's shared PowerShell
   session. Prefer a saved script launched in a CHILD process:

   ```powershell
   & powershell.exe -NoProfile -NonInteractive -File 'C:\absolute\path\task.ps1'
   $taskExitCode = $LASTEXITCODE
   if ($taskExitCode -ne 0) { throw "Task failed with exit code $taskExitCode" }
   ```

   Use `pwsh.exe` instead only if it is installed. Set `$ErrorActionPreference =
   'Stop'` inside the task script and use `return` for routine early completion.
   Do not send bare `exit`, `logout`, or `Stop-Process -Id $PID` to a reused
   parent terminal/runspace. If an exit code is necessary, confine `exit <code>`
   to the explicitly launched child script. Invoke native programs with `&` and
   structured arguments; avoid `Invoke-Expression` and string-built commands.
5. Run batch build scripts through `cmd.exe /d /c <absolute-script.cmd>`.
   `exit /b` belongs inside that child batch process, not an IDE-control action.
   Capture output and return codes instead of closing the console or workspace
   to mark a task complete.
6. If the workspace closes again, stop issuing cleanup/relaunch commands.
   Report the exact last command, which surface closed, the error/output, and
   available Copilot/Visual Studio logs. Preserve files and let Kevin decide
   how to reopen the workspace. Do not automatically reset IDE settings.

Microsoft documents that PowerShell `exit` terminates a script or PowerShell
instance, and that `powershell.exe -File` returns a child's script exit code:
[PowerShell language keywords](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_language_keywords?view=powershell-5.1).
Terminal commands in Visual Studio agent mode have the IDE process's permissions;
they are not confined to the solution's normal file-tool boundary:
[Visual Studio agent mode](https://learn.microsoft.com/visualstudio/ide/copilot-agent-mode?view=visualstudio).

## Repository and commit state

- Source repository: `C:\Users\Kevin\GitRepos\Arduino-Source`.
  Branch: `BDSP/AutoStory`.
  `origin`: `https://github.com/Developer-Butters/Arduino-Source.git`.
  `upstream`: `https://github.com/PokemonAutomation/Arduino-Source.git`.
- October 11 rebase: originally 299 upstream commits behind and 23 fork commits
  ahead. Replayed all 23 onto upstream main, then added two compatibility commits.
  Published tip is `08760b8fe`; at that snapshot it was 0 behind / 25 ahead.
  Fetch and inspect before claiming it is still current.
- Rebased compatibility commits: `69676bbc7` updates BDSP to upstream APIs;
  `08760b8fe` restores authoritative shared detectors and localizes extensions.
  Other games moved under `Source/VideoGames`; BDSP remains under
  `SerialPrograms/Source/PokemonBDSP`. The BDSP battle helper's Sword/Shield
  type-matchup include now uses `VideoGames/PokemonSwSh/Resources/`.
- Shared detector and shared developer/unit-test implementations match upstream.
  Do not resurrect deleted `Tests/TestMap.cpp` or old shared BDSP debug checkboxes.
  Use `OperationFailedExceptionWithScreenshot::fire`, `ErrorReportMode`,
  `GlobalAutoPaths.h`, and upstream's `make_SingleSwitchProgram` / Descriptor
  alias pattern. Do not reintroduce the old interfaces.
- Recovery copies remain in branch
  `backup/BDSP-AutoStory-before-upstream-20261011` and the named rebase stash.
  These are backups, not code to apply over current work blindly. The old local
  OCR CharacterReductions path fix is already in upstream; no duplicate fix is needed.
- Only Segment 02's route file is currently staged. Its staged contents depend
  on UNSTAGED helper/matcher changes. Do not commit that file alone and assume
  the resulting checkout will build. Inspect all dependencies when Kevin asks
  for a commit. The pending files include Segment 00 tools `.cpp/.h`, the
  overworld matchers `.cpp/.h`, and local context/backlog/handoff documents.
- Packages is a SEPARATE repository at `...\BDSP-AutoStory\Packages`, on branch
  `BDSP-Autostory`. Approved push remote is `developer-butters`:
  `https://github.com/Developer-Butters/Packages.git`.
  `origin` still points to PokemonAutomation upstream: DO NOT push there.
  Published Packages tip is `89fc5f8`. New, uncommitted resources include 18
  Jubilife lighting references and `Autostory/Route204Sign/Sign.png` plus
  `FenceCorner.png`. Other untracked test images/assets also exist; never add
  everything indiscriminately.
- Do not pull the old branch history back into the rebased source branch.
  Collaborators need a one-time fetch/resync preserving their local work first.
  No further rebase, reset, commit, push, history rewrite, or upstream PR without
  Kevin's specific instruction. Current implementation work is uncommitted.

## Current Segment 02 behavior

Primary file:
`SerialPrograms/Source/PokemonBDSP/Programs/AutoStory/StorySegments/PokemonBDSP_AutoStory_Segment_02.cpp`.
Route function: `roark_to_mars(env, stream, context)`.

1. Oreburgh Mart alignment and Repel buying precede Gate entry. Entry watches
   black before up/left/down search; exit watches white before moving west and
   waits for the fade to finish. The cleaned Route 203 path avoids descending
   the upper stairs only to climb back up; it travels left, jumps down the ledge,
   and combines the lower-stair corrections.
2. At the marked safe point, `force_repel_expiry(..., RepelRenewalChoice::NO)`
   wiggles until expiry and declines renewal. `YES` remains supported. Cleanup
   requires three continuous clear overworld seconds, not one transient frame.
3. Exactly one clock setup occurs when entering this contiguous NEW locator
   strip: `set_time_fast(env, context, locator_time, true)` sets 13:00 and
   preserves the current console date, followed by five seconds for lighting.
   It covers street, Galactic plaza, post-battle return, and nearby Route 204
   work. Do not reset the clock for every matcher. Checkpoint retries enter the
   strip again; no clock restoration is currently implemented or requested.
   Existing earlier locators do not need retroactive clock changes.
4. The westward street approach is 3700ms. The old blind 2579ms upward move was
   removed. `align_to_jubilife_approach` handles both axes and prioritizes street
   lane alignment. Two strong coarse anchors may guide a horizontal correction
   only when their signed offsets agree and the offset exceeds their spacing
   uncertainty. Coarse matching never confirms arrival. Missing landmarks use
   bounded right/left sweeps at several heights rather than continuous left/up.
5. The plaza matcher has a small normalized horizontal quantization allowance
   to avoid oscillating between two reachable positions around the target.
   The approach sidesteps left 200ms around the sign before watched upward
   pulses; straight up plus A previously read the sign instead of triggering Rowan.
6. Dialogue/battle triggers stop locator movement. Battle preparation clears
   dialogue with B until the battle menu is confirmed. The existing
   `Barry_1_fight` handler is reused. Do not move onward merely because dialogue
   was seen: menu confirmation and combat completion are separate requirements.
7. Rowan/Bebe dialogue clears in two passes separated by two seconds. Return
   down 600ms before reacquiring the plaza. The old 1000ms return overshot, and
   the additional downward recovery pulses are now disabled at this call site.
   From the confirmed reference, the route proceeds to Jubilife Center.
8. After Center exit and westward alignment, `approach_route204_female_trainer`
   watches throughout the 8000ms northward movement and a 2500ms stationary
   detection window. It returns whether ShortDialogWatcher or trainer battle
   menu appeared, cancelling remaining motion if either triggers.
9. `const bool missed_trainer` records the initial miss. If true,
   `recover_route204_female_trainer` crosses her sight line DOWN 500ms then
   UP 500ms, at most eight pairs, with fresh dialogue/menu watchers. Failure is
   bounded and reported; it does not return to a two-minute generic wait.
10. Both paths confirm the menu, fight, clear post-battle dialogue, and activate
    a fresh Repel. The first Repel was deliberately NOT renewed earlier.
11. `align_to_route204_sign` uses the supplied reference photo, sign and fence
    corner, normalized positioning, visible unlabeled ROIs, and three-frame
    target confirmation. It watches each corrective move and settling period
    for the NEXT trainer's dialogue/menu. It also avoids steering during fades.
    It returns true if the trainer interrupts alignment, false if at target.
12. If aligned without a trigger, a watched 1200ms left approach starts the next
    trainer. If interrupted early, skip further positioning and use the same
    confirmed-menu/combat/cleanup path. Never skip the battle handler and send
    overworld moves into an active battle. The subsequent northward movement
    and later cave-navigation placeholder remain to validate/complete.

The latest live video, `2026-10-12 19-15-37.mp4`, demonstrated the missed female
trainer and the old stationary wait. The newly implemented recovery/sign flow
has NOT yet been observed in a live recording. Do not claim it is route-validated.

## Matchers, resources, and verification

Matcher registry: `AutoStory/Utils/PokemonBDSP_OverworldMatchers.cpp/.h`.
Current entries: Poketch, Oreburgh Mart, Jubilife Center, Route 201 Potion exit,
Sandgem junction, Route 202 grass exit, Jubilife street/plaza, and Route 204 sign.
The lighting tester is in `PokemonBDSP/Programs/TestPrograms/` and uses this
registry automatically. `Autostory` is the resource folder casing.

- Valid Jubilife lighting runs: `jubilife-street-approach-1791772008` and
  `jubilife-plaza-approach-1791772619`. Each now passes 120/120 captured frames.
  Exclude `jubilife-street-approach-1791771478` and
  `jubilife-plaza-approach-1791771937`: Kevin started those at wrong positions.
- Street references use building corner/base, avoiding repeated facade windows.
  Plaza references use roof rim and fence. Eighteen references cover all five
  captured frames per hour, including moving shadows. Alternatives for one
  landmark are NOT independent votes; two distinct anchors must agree.
- Street target height preserves the original screenshot; the lighting-run
  pose differed by 1/108 screen height. Do not move the target to make an
  offset test pass. Original-reference checks passed too.
- Route 204 reference:
  `build/Debug/Screenshots/screenshot-20261012-192258390819.png`.
  Its two crops exclude trainer/player animation and the collectible item.
  Thirteen native checks passed for 720p/1080p/1440p, both offset directions,
  unrelated scenes, and masking people/items. Full-day lighting has not been
  tested for this new matcher; runtime currently uses the strip's 13:00 setup.
- New clock/date option is in Segment 00 tools: `preserve_date` defaults false,
  so existing callers retain exact-date behavior. The new clock-only path reads
  and validates year/month/day using the existing console date reader before
  setting hour/minute. This reuses console UI OCR; it adds no game-text OCR.
- Transition extensions now live in
  `PokemonBDSP/Inference/PokemonBDSP_TransitionWatchers.cpp/.h`, not CommonTools.
  Use BDSP namespace qualification if global/local watcher names are ambiguous.
- Last complete Debug build/link succeeded after Route 204 integration.
  Matcher checks prove detection properties, not live movement/battle success.

## Kevin's constraints

- Prefer focused BDSP-local changes. Shared directories such as CommandLine,
  CommonFramework, CommonTools, ComputerPrograms, ControllerInput, Controllers,
  Integrations, Kernels, ML, and shared NintendoSwitch/Pokemon code require a
  walkthrough BEFORE editing and Kevin's explicit informed approval. Shared
  changes require a separate commit with detailed necessity and cross-game
  validation. SourceFiles.cmake registration is allowed for actual new files.
- Targets, search regions, sizes, offsets, and tolerances MUST be screen-relative.
  Pixel conversion belongs only at image-processing boundaries. Account for
  aspect ratio when comparing axes. Never hard-code capture-size movement logic.
- Prefer language-independent geometry and avoid OCR unless necessary.
  Text-bearing templates are language-dependent even without OCR. Show ROIs
  through normal Overlays; colored boxes only, no text labels.
- Preserve Kevin's unsaved Visual Studio edits. Before external edits to an
  open file, have him save/close it or preserve both copies and reconcile them.
  Do not overwrite editor contents or respond to a reload warning by discarding
  unknown changes. Preserve Git baseline line endings: LF/CRLF changes previously
  made Segments 03-09 look completely rewritten and must not recur.
- Fresh watchers belong to each separate detection phase. Keep state across
  rounds only for deliberately latched end-battle watchers.
- Autosave must be verified OFF. Settings save/exit is A A A A B B with detection
  retries and persistence verification. REAL saves are the default; FAKE saving
  and post-catch diagnostic entry remain marked for removal before production.
- Evaluate actual evidence before adding black/white watchers. Do not spend
  time looking for a watcher that Kevin says does not exist.
- No unsolicited subagents, commits, pushes, resource cleanup, process killing,
  or work on deferred features. Retain bounded recovery and useful logs.

## Earlier work and deferred items

Segments 00/01 and associated tools were committed/pushed before this focus.
Tutorial work includes settings verification, post-catch saved checkpoint
boundary, robust capture/battle-result cleanup, generic grass movement with
optional destinations, shared confirmed-Run escape, visual 1-4 move counting
and selection, Potion HP verification, one wild Scratch training battle after
Potions, and Ember slot 3 for Tristan. Do not reintroduce named grass wrappers
or blind move-slot selection. Segment 01 Gate-to-mine movement is enabled.

Defer maxed starter/species/level/moves validation before Segment 01, faint
handling, grass optimization, and multilingual settings until Kevin asks.
Also verify the existing REAL-save fallback before claiming checkpoint safety
is fully solved; earlier analysis found bookkeeping may advance after an
unverified fallback. These are not authorization to expand the current task.

## Build and analysis workflow

- Runtime log: `C:\Users\Kevin\GitRepos\Arduino-Source\build\Debug\SerialPrograms.log`.
  It is large; filter exact run timestamps and do not confuse idle/old runs.
- Lighting data: `...\build\Debug\Screenshots\OverworldLightingTest`.
  Recordings: `C:\Users\Kevin\Videos\Captures`.
- Analysis workspace: `C:\Users\Kevin\Documents\Codex\2026-10-03\lo\work`.
  Use that directory for scratch files, not the repository. An external script
  with relative `work/...` paths must run from its parent `...\lo`, not repo root.
- Build script: `work\build_rebased.py` in that analysis workspace. Python:
  `C:\Users\Kevin\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe`.
  It calls VS 18 Community `vcvars64.bat`, then CMake `--build` on the repo's
  `build\Debug`, target `SerialPrograms`, eight workers. Log:
  `work\link_utils_move.log`. `work\link_utils_move.cmd` is the generated batch
  equivalent; run it as a child `cmd.exe /d /c`, never by closing the workspace.
- If LNK1168 says SerialPrograms.exe cannot be opened for writing, ask Kevin to
  close SerialPrograms and rebuild. Do NOT kill it or Visual Studio.
- Existing native harnesses: `run_route204_tests.py`,
  `run_jubilife_lighting_tests.py`, `run_jubilife_route_tests.py`,
  `run_jubilife_approach_tests.py`, `run_native_battle_cursor_tests.py`,
  `run_position_tests.py`, `run_transition_tests.py`. Read before using them.
  They now use the GUI target's library set, not the old command-line CoreLib
  assumption. Do not run harness builders concurrently: they share a batch path.
  `work\Resources` provides the runtime resources needed by the test executables.
- PyAV is under `work\video_dependencies`; add it to Python's import path.
  Seek to timestamps for long recordings. Crop only the OBSERVED game viewport
  before testing a recorded image; UI borders/stats overlays can contaminate
  match scores. Do not confuse screenshot tests with an untouched camera feed.
- A pre-existing ONNX API/runtime-version warning appears in some native test
  startups. Image-based checks still passed. Do not silently modify shared
  dependencies or claim ONNX-dependent features have been tested.

Start with the current diff and Kevin's next recording. Explain the concrete
failure and the proposed correction, keep edits narrow, build appropriately,
and clearly distinguish offline validation from live route behavior.

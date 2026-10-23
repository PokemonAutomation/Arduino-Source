# AutoStory backlog

## Segment 01: Validate the maxed starter before movement

Status: Deferred. Record only; do not implement yet.

Add a `checkformaxlevel` function at the start of Segment 01, before any navigation or movement. Also cover direct starts at checkpoint 01.01 so advanced mode cannot bypass the validation.

Validate the Pokemon in party slot 1:

- Species matches the Starter Pokemon dropdown: Chimchar -> Infernape, Turtwig -> Torterra, Piplup -> Empoleon.
- Read its level and require level 100.
- Read its moves and verify the required AutoStory moveset for that evolved starter. Determine and document the exact required movesets when implementing this item.

If any check fails, stop without running route movement and show this error:

"do you have your starter maxed and in slot 1?"

Unreadable species, level, or moves must count as a failed check. Validation may open the party/summary UI, but must complete before overworld movement begins.

## AutoStory battles: Handle a fainted starter

Status: Deferred. Do not implement yet.

Detect starter fainting and the forced party-selection screen in tutorial battles.
Handle this as an unexpected battle state instead of B-mashing until a generic
timeout. Decide whether to switch, recover, or retry from the checkpoint. Ordinary
route operation should prevent this by verifying Potion healing between battles.
Keep the normal trainer strategy unchanged; this is rare-case recovery.

## Segments 00 and 01: Optimize grass movement steps

Status: Deferred until the full route through Roark is validated. Do not change timings yet.

Review recordings and matching logs to reduce unnecessary walk-through-grass steps, especially open-path movements that do not deliberately align against a wall. Excess steps add encounter opportunities, detection waits, and position drift.

Classify each section before changing it:

- Wall alignment: retain sufficient movement to reliably reach the wall.
- Open-path travel: reduce steps to the required distance while preserving the next movement's starting position.
- Encounter search: stop movement as soon as an encounter is detected.

Prioritize obvious excess steps. Compare baseline and optimized runs across varying encounter patterns, and verify both the destination and the following movement before accepting a shorter route.

## Settings verification: Support every game language

Status: Deferred. Record only; decide the detection approach later.

Settings verification must work in every language supported by BDSP. The current
visual templates for setting names and values depend on their language and must
be replaced or extended before claiming multilingual support.

Evaluate position-based detection of settings rows, selected values, and sliders
versus explicitly language-aware OCR. Prefer language-independent geometry where
reliable; decide which approach is better after comparing the localized screens.
Do not assume that text width, row layout, or value positions are identical in all
languages without validation.

Verify every required setting, including persistence after saving and reopening
Options. Autosave must be confirmed OFF in every supported language before route
movement begins. Validate the chosen approach against captures for all supported
languages, including incorrect settings and ambiguous detection cases.

Related requirement: ideally all matchers and automation functions should be
language-independent unless explicitly using OCR. Audit text-based visual
references and other language assumptions when extending or reviewing them.

## AutoStory helpers: Audit and reorganize reusable tools

Status: Deferred. Record only; do not implement yet.

Examine existing AutoStory helpers, their callers, and their responsibilities.
Reclassify helpers by purpose rather than the segment where they were first
introduced. Identify duplicate or overlapping functions and prefer reusing or
expanding an existing implementation before developing another helper.

Likely tool files (with matching headers where appropriate) include:

- `AutoStory_BattleTools.cpp`: battle handling, move selection/sequences, and battle cleanup.
- `AutoStory_NavigationTools.cpp`: route movement, alignment, doorway transitions, and navigation recovery.
- `AutoStory_BagTools.cpp`: Bag navigation, item use, pocket detection, and NEW-flag clearing.

Review the existing battle/navigation utilities and segment-specific tool files
before deciding the final file boundaries and names. Consolidate and move shared
helpers rather than creating parallel implementations. Keep segment-specific
route sequences in their segments.

Include a code review of helper implementations and their callers throughout
AutoStory. Incorporate and adjust helpers and call sites so routes use the shared
tool helpers instead of unique one-off implementations wherever practical.
Expand shared helpers when their responsibilities fit, and remove superseded
duplicates after migrating callers. Aim to standardize most helpers; do not
force every helper into a common abstraction. Retain specialized helpers where
the behavior warrants them, and document why they remain specialized.

Preserve behavior and public callers during
the reorganization, and validate affected routes and builds.

## Checkpoint descriptions: Deferred wording and endpoint cleanup

Status: Deferred. Do not spend implementation/review time on routine wording cleanup while developing and validating the route.

Audit checkpoint names, start/end descriptions, and related comments together in a later cleanup pass. Match descriptions to verified gameplay positions and preserve the requirement that each checkpoint starts at the previous checkpoint's endpoint. There are many stale descriptions across AutoStory; handle them as a batch rather than piecemeal edits. The Eterna Forest endpoint outside the Center is one example. Actual route or checkpoint-state failures still need prompt fixes; cosmetic description cleanup stays deferred.

## Project roadmap: Transition from Segment 03 implementation to Segment 04 testing

Status: Deferred until Segment 03 implementation is complete and work reaches Segment 04. At that transition, begin testing the existing route and reviewing missing information. Do not start a broad helper refactor or roadmap audit during current route fixes.

Revisit the earlier five-phase BDSP AutoStory roadmap (Setup, Streamlining, Cleanup, Beta Testing, Future Implementation) to reflect the present workflow: implement and validate recorded route segments incrementally, use logs/video to fix failures, and reuse or expand shared battle/navigation/Bag tools.

Kevin clarified that Segment 04 is the first segment expected not to need substantial implementation. Treat segments 04 through 09 as existing route implementations to test and audit, not as validated or release-ready. When reaching Segment 04, look for missing route information, unsupported states and required functionality; implement evidence-supported gaps rather than rebuilding the segment. Continue that testing/gap-review workflow through the later segments. The 2026-10-23 source audit has now confirmed substantial missing gameplay; the inventory below supersedes any assumption that the later segments only need testing. Preserve implemented travel while filling these gaps.

The revised roadmap should:

- Separate remaining route implementation from live validation, failure recovery, helper consolidation, cleanup and release preparation.
- Audit segments 04–09 for existing route coverage, moving NPCs/key fights, checkpoint handoffs, item/money/buy-list requirements and one-off helper logic. Identify actual gaps rather than rebuilding completed routes.
- Link the existing "AutoStory helpers: Audit and reorganize reusable tools" task. Plan shared helper creation/expansion and consolidation across those later segments, especially BattleTools, NavigationTools and BagTools, while retaining justified specialized behavior.
- Keep broad file consolidation, unused-file removal, formatting and checkpoint wording cleanup in a deliberate later pass; do not let them displace current route validation.
- Define evidence-based validation and regression gates before beta testing and eventual main-repository PR/release. This task does not authorize creating/pushing a PR now.
- Retain a separate future-upgrades phase and list dependencies/acceptance criteria for each workstream.

Deliver a revised roadmap with segment status, outstanding functionality, helper migration priorities, validation gates and a practical order of work when Segment 03 is complete and Segment 04 testing begins.

## Segments 04–09: Fill missing gameplay and repair checkpoint handoffs

Status: Backlogged required implementation, identified by the 2026-10-23 read-only source audit. Do not implement as part of recording this task. Complete current Segment 03 work first, then fill and validate these gaps in route order. Existing travel code is not yet live validated by this audit.

Source locations below are relative to `SerialPrograms/Source/PokemonBDSP/Programs/AutoStory/`. Line numbers describe the audited revision and may shift.

### Missing gameplay

- [ ] **Segment 04 — Pastoria Gym:** `StorySegments/PokemonBDSP_AutoStory_Segment_04.cpp`, `go_to_pastoria_gym` (line 977), checkpoint 0417 (1185). The current route only enters the gym although the endpoint claims the fourth badge. Implement the puzzle, Crasher Wake battle, verified badge completion and the exit position required by Segment 05.
- [ ] **Segment 05 — Hearthome Gym:** `Segment_05.cpp`, `get_into_hearthome_gym` (1454), checkpoints 0516/0517 (1651/1662). Both checkpoints call the entrance route. Implement the puzzle, Fantina battle, verified badge completion and exit boundary; replace the duplicated entrance call with the actual interior phase.
- [ ] **Segment 06 — Canalave Gym:** `Segment_06.cpp`, `go_to_canalave_gym` (590), checkpoints 0610/0611 (730/741). Both call the entrance route. Implement the lift puzzle, Byron battle, verified badge completion and exit boundary.
- [ ] **Segment 07 — Snowpoint Gym:** `Segment_07.cpp`, `go_to_snowpoint_gym` (1729), checkpoints 0718/0719 (1954/1965). Both call the entrance route. Implement the ice puzzle, Candice battle, verified badge completion and exit boundary.
- [ ] **Segment 08 — Final Mount Coronet story:** `Segment_08.cpp`, `go_through_mount_coronet` (1784), checkpoint 0816 (2485). The function only waits, logs and returns true. Implement the final ascent, Spear Pillar story encounters/battles, supported legendary resolution and return to the position required by the next route. Dormant legacy `checkpoint_021`/legendary button sequences are not a working implementation and have no catch-success verification.
- [ ] **Segment 08 — Sunyshore Gym:** `Segment_08.cpp`, `go_to_sunyshore_gym` (2250), checkpoints 0825/0826 (2584/2600). Both call the route ending at the gym entrance. Implement the gear/platform puzzle, Volkner battle, verified eighth badge and exit boundary.
- [ ] **Segment 09 — League entry and completion:** `Segment_09.cpp` (1354–1429), checkpoints 0909–0914. All six functions below only wait, log and return true. Implement entry prerequisites, room traversal, trainer fights, necessary party/item preparation and verified Champion/Hall of Fame completion:
  - `get_into_the_elite_4_entrance` (1354).
  - `deal_with_aaron_elite_4` (1367).
  - `deal_with_bertha_elite_4` (1380).
  - `deal_with_flint_elite_4` (1393).
  - `deal_with_lucian_elite_4` (1406).
  - `deal_with_cynthia_and_finish` (1419).

`Segment_XX.cpp` above abbreviates `StorySegments/PokemonBDSP_AutoStory_Segment_XX.cpp`.

### Checkpoint and failure-handling gaps

- [ ] Register **checkpoint 0717** in the global checkpoint list in `PokemonBDSP_AutoStory.cpp` (between 0716 and 0718, lines 155–157). Segment 07's own runner includes this Snowpoint Center heal/exit step; checkpoint mode currently skips it and changes the next route's starting scene.
- [ ] Include **checkpoints 0822–0826** in Segment 08's `run_segment` (85–123). They are present in the global checkpoint registry but omitted from the segment runner. Validate both supported execution modes against the same checkpoint sequence.
- [ ] Replace the duplicated gym-entrance calls listed above with real puzzle/battle phases. Verify that each checkpoint starts at its predecessor's actual endpoint and that the next segment starts from the verified post-badge position.
- [ ] Propagate `heal_pokemon` failure in seven wrappers that currently return true regardless: Segment 05 Solaceon (822), Celestic (1233), Hearthome (1444); Segment 06 Jubilife (276), Canalave (578); Segment 07 Veilstone (1134), Snowpoint (1715). The shared helper returns false when the Center exit transition is missing; subsequent pathing must not continue from an unconfirmed scene.
- [ ] In Segment 09 `ready_in_front_of_elite_4` (1291), check `buy_items`' result (1344–1349) and replace the assumption that one fixed west/east wiggle must expire Repel with a bounded, confirmed preparation sequence.

### Acceptance criteria and order

Use recorded/calibrated travel, existing battle/dialogue/navigation tools and explicit completion conditions. Do not substitute invented blind inputs or unconditional success for missing gameplay. Verify both successful progression and bounded failures, checkpoint restart positions, battle completion and doorway settling. Reuse or expand shared helpers where appropriate; coordinate broader consolidation with the existing helper-audit backlog item.

Recommended order: finish Segment 03, then the gym phases in 04–07, final Coronet and Sunyshore in 08, and League completion in 09. Repair the corresponding checkpoint connections and ignored failures alongside each phase. Keep routine checkpoint wording cleanup deferred, but correct descriptions when needed to express the implemented start/end contract.

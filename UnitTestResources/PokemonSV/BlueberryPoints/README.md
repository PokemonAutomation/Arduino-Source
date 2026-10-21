# Blueberry Points reader references

These unmodified 1920×1080 frames were saved with SerialPrograms' Screenshot
button from Pokémon Violet on Switch 1, using the Traditional Chinese UI.

| Image | Expected BP | Quest in the first slot |
| --- | ---: | --- |
| [6750.png](6750.png) | 6750 | Catch any Pokémon |
| [6740.png](6740.png) | 6740 | Photograph a swimming Pokémon |

The second frame follows one confirmed 10-BP blue-quest reroll. The balance
decreased by 10 and the first quest changed; the other two quests stayed in
place. No farmer was running during this check.

`read_BP_from_image()` uses coordinates normalized to the full 16:9 frame:
`(x=0.866, y=0.019, width=0.088, height=0.041)`. The right edge is 0.954.
The previous width, 0.091, ended at 0.957 and included the left stroke of the
adjacent `B` suffix. That stroke was recognized as an extra `1` with PaddleOCR.
The near-white RGB range 198–255 is inherited from the existing reader; this
change does not recalibrate text colors.

The registered `PokemonSV::BlueberryPoints` tests pass each full frame through
the production crop, color filter and number OCR, at 1920×1080 and 1280×720.
Select these four tests in **Computer → Unit Test Runner**. The runner resolves
these paths under `UNIT_TEST_RESOURCE_PATH()`; keep this `UnitTestResources`
directory alongside the source checkout or application resource directories.
OCR uses the configured backend; the PR validation used PaddleOCR.

With the old width temporarily restored, all four cases fail: the reader returns
`67501` or `67401`. With width 0.088, all four pass. This checks that the tests
detect the original suffix contamination rather than just recognizing digits.

Keeping the original frame dimensions is intentional: testing an already-cropped
number would not catch regression of the production crop into the suffix.
These two four-digit balances do not establish accuracy for all balance widths,
UI languages or capture formats.

The `PokemonSV::BlueberryQuestDetector` tests read the quest slots of the same
frames with Traditional Chinese OCR: catch-any, sneak-up and auto-10 in 6750.png,
and photo-swim, sneak-up and auto-10 in 6740.png. The fourth slot is empty and
must read as nothing.

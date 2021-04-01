# Swift module shim for the TouCAN C-based CANAPI driver for the Swift Package Manager (SPM).

## Some notes to consider

* SPM demands a rather strict source layout, hence I could not reuse the other directories.
Since I didn't want to copy source files, I had to resort to softlinks.
* Unfortunately, SPM does not feature custom scripts, so
we can't autogenerate the `build_no.h` – I had to add one in `privateInclude` to make it build.
* I would advise not to use `CTouCAN` directly, the `SwiftTouCAN` module builds on top of this and
has a much nicer API.

– Mickey.

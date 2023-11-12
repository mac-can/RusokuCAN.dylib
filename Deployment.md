### macOS&reg; User-Space Driver for TouCAN USB Interfaces from Rusoku

_Copyright &copy; 2020-2023 Uwe Vogt, UV Software, Berlin (info@mac-can.com)_ \
_All rights reserved._

# Deployment Rules

## Create the Release Candidate

### Precondition

- **_All changes are made exclusively on a feature branch!_**

### Preparation

1. If necessary, update the MacCAN-Core sources in `$(PROJROOT)/Sources/MacCAN`
   from SVN repo and commit them with commit comment:
  - `Update MacCAN-Core sources to rev. `_nnn_ \
    `- `_list of major changes (optional)_
2. If necessary, update the CAN API V3 sources in `$(PROJROOT)/Sources/CANAPI`
   from the SVN repo and commit them with commit comment:
  - `Update CAN API V3 sources to rev. `_nnn_ \
    `- `_list of major changes (optional)_
3. If necessary, update the CAN API V3 testing sources in `$(PROJROOT)/Tests/CANAPI`
   from the SVN repo and commit them with commit comment:
  - `Update CAN API V3 testing sources to rev. `_nnn_ \
    `- `_list of major changes (optional)_
4. Check and update the version and date information in the following files:
  - `$(PROJROOT)/Sources/TouCAN.h`
  - `$(PROJROOT)/Sources/TouCAN.cpp`
  - `$(PROJROOT)/Sources/Wrapper/can_api.c`
  - `$(PROJROOT)/Sources/Swift/CANAPI.swift`
  - `$(PROJROOT)/Libraries/CANAPI/Makefile`<sup>*</sup>
  - `$(PROJROOT)/Libraries/TouCAN/Makefile`<sup>*</sup>
  - `$(PROJROOT)/Utilities/can_moni/Driver.h`
  - `$(PROJROOT)/Utilities/can_moni/Makefile`
  - `$(PROJROOT)/Utilities/can_moni/README.md`
  - `$(PROJROOT)/Utilities/can_test/Driver.h`
  - `$(PROJROOT)/Utilities/can_test/Makefile`
  - `$(PROJROOT)/Utilities/can_test/README.md`

  <sup>*</sup>_) Set variable_ `CURRENT_VERSION` _and_ `COMPATIBILITY_VERSION` _accordingly._

### Procedure

1. Check the working folders for uncommitted changes.
  - _**There should not be any uncommitted changes.**_
  - _If there are uncommitted changes then commit or undo them._
2. Open the trial program with Xcode and run a code analysis.
  - _**There should not be any serious finding.**_
  - _If there are findings then fix them or create an issue in the repo._
3. Select the Xcode Testing target and run all test cases (two device are required):
  - _**There should be no failed test case.**_
  - _If there are failed tests then fix the root cause or define a workaround._
4. Open the SPM configuration with Xcode and check for errors:
  - _**There should be absolutely no package manager error!**_
  - _If there are package manager warnings then think twice._
5. Run the `Makefile` in the project root folder with option `BINARY=UNIVERSAL`.
  - _**There should be absolutely no compiler or linker error!**_
  - _If there are compiler or linker warnings then really think twice._
6. Try out the trial program with different options.
  - _**There should be no crash, hangup, or any other error.**_
  - _If there is an error then fix it or create an issue in the repo._
7. Try out the utilities with different options.
  - _**There should be no crash, hangup, or any other error.**_
  - _If there is an error then fix it or create an issue in the repo._
8. Build and try out the examples (fix them if necessary);
  - `$(PROJROOT)/Examples/C++`
  - `$(PROJROOT)/Examples/Python`
  - `$(PROJROOT)/Examples/Swift`

### Pull Request

1. Update the `README.md` (e.g. development environment, supported devices, etc.).
2. Push the feature branch to the remote repo.
3. Create a pull request and name it somehow like '**Release Candidate _n_ for** ...'.
4. Review the changes and merge the feature branch into the default branch.

## Create the Release Tag

### Preparation

1. Pull or clone the default branch on all development systems.
2. Double check all version numbers again (see above).
3. Run the `Makefile` in the project root folder:
  - `uv-pc013mac:~ eris$ cd $(PROJROOT)`
  - `uv-pc013mac:RusokuCAN eris$ make pristine`
  - `uv-pc013mac:RusokuCAN eris$ make BINARY=UNIVERSAL`
  - `uv-pc013mac:RusokuCAN eris$ make test`
  - `uv-pc013mac:RusokuCAN eris$ sudo make install`
4. Build the CAN API V3 GoogleTest program:
  - `uv-pc013mac:~ eris$ cd $(PROJROOT)/Tests/CANAPI`
  - `uv-pc013mac:CANAPI eris$ make pristine`
  - `uv-pc013mac:CANAPI eris$ make all`
5. Run the CAN API V3 GoogleTest program with two TouCAN USB device:
  - `uv-pc013mac:CANAPI eris$./tou_testing --can_dut1=TouCAN-USB1 --can_dut2=TouCAN-USB2 --gtest_output=xml:TestReport_TouCAN-USB.xml --run_all=YES --smoketest_frames=100000` [...]
  - _If there is any error then **stop** here or create an issue for each error in the repo._
  - Copy the test report into the binary's folder `$(PROJROOT)/Binaries`.
7. Pack the artifacts into a .zip-archive, e.g. `artifacts.zip`:
  - `$(PROJROOT)/Binaries/*.*`
  - `$(PROJROOT)/Includes/*.*`
  - `$(PROJROOT)/README.md`
  - `$(PROJROOT)/LICENSE`
8. Double check and update the [`README.md`](https://github.com/mac-can/RusokuCAN/blob/main/README.md) on GitHub (or insert just a blank).

### Procedure

1. Click on `Draft a new release` in the [GitHub](https://github.com/mac-can/RusokuCAN) repo.
2. Fill out all required fields:
  - Tag version: e.g `v0.2.6` (cf. semantic versioning)
  - Target: `main` (default branch)
  - Release title: e.g. `Release of November 12, 2023`
  - Change-log: list all major changes, e.g. from commit comments
  - Assets: drag and drop the artifacts archive (see above)
3. Click on `Publish release`.
4. That´s all folks!

## Announce the new Release

1. Create a new post with the change-log in the `mac-can.github.io` repo.
2. Update the RusokuCAN driver page in the `mac-can.github.io` repo.
3. Post the new release on
[Twitter](https://twitter.com/uv_software),
[Facebook](https://facebook.com/uvsoftware.berlin),
etc.

# MarcViiewLibGuiTest

Minimal Wii test application for integrating [libgui](https://github.com/dborth/libgui) with the build environment used by MarcViiew.

## Purpose

This repository deliberately contains no MarcViiew backend code. The first milestone is to prove that the current libgui Wii platform can build and run in the same development environment used by MarcViiew.

The test covers:

- libgui platform initialisation
- FreeType text rendering
- GUI text and buttons
- Wii Remote navigation
- A/B input handling
- clean application exit

## Dependency

The Makefile expects a checkout of the current `dborth/libgui` repository beside this repository:

    parent/
    ├── MarcViiewLibGuiTest/
    └── libgui/

Alternatively set `LIBGUI_DIR` when building:

    make LIBGUI_DIR=/path/to/libgui

Current libgui Wii builds use devkitPPC with libogc2 and the portlib dependencies documented by the upstream project.

## Build

From the repository root:

    make

The output is `MarcViiewLibGuiTest.dol`.

## Test controls

- D-pad: move between GUI buttons
- A: activate the selected button
- B: activate the exit button when selected

The test intentionally does not use ViiewLib, NAND access, TMD parsing, MARC records, or any MarcViiew application code.

## Next step

Once this standalone test is confirmed on Wii hardware, the proven libgui build and UI pattern can be brought into MarcViiew incrementally.

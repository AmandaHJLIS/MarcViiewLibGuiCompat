# MarcViiewLibGuiCompat

Compatibility layer and hardware test environment for running the current upstream [libgui](https://github.com/dborth/libgui) GUI framework with the legacy libogc-based Wii development environment used by MarcViiew.

## Purpose

This repository began as a small libgui integration test. It has evolved into a dedicated compatibility project: upstream libgui is kept outside this repository, while this project supplies the small set of legacy-libogc adaptations needed to build and run it with MarcViiew's existing Wii toolchain.

The goal is to validate libgui independently before integrating it into MarcViiew.

## What is being tested

The compatibility project currently covers:

- libgui platform initialisation
- FreeType text rendering
- GUI text, images, windows, and buttons
- Wii Remote/controller input
- audio initialisation
- clean application exit
- legacy `DISC_INTERFACE` compatibility
- legacy Wii filesystem/disc interface compatibility
- legacy USB multi-device compatibility
- legacy thread/lock/condition compatibility
- an intentionally disabled SMB implementation for toolchains without `libsmb2`

The `source/compat/` directory contains the compatibility implementations. These are deliberately isolated from the upstream libgui checkout.

## Upstream libgui

The Makefile expects a checkout of the current `dborth/libgui` repository beside this repository:

    parent/
    ├── MarcViiewLibGuiCompat/
    └── libgui/

Alternatively set `LIBGUI_DIR` when building:

    make LIBGUI_DIR=/path/to/libgui

Current upstream libgui Wii builds target newer `libogc2` environments. This repository exists to investigate and bridge the differences encountered when building against the legacy `libogc` environment used by MarcViiew.

## Build

From the repository root:

    make

The output is `MarcViiewLibGuiCompat.dol`.

## Hardware test

The included application is intentionally small and does not depend on MarcViiew.

It displays a basic libgui interface and allows testing of:

- rendering and text layout
- controller input
- GUI selection/click handling
- application shutdown

Dolphin can be used for initial testing, but real Wii hardware is important because this project is specifically concerned with compatibility with the target console and its existing libogc environment.

## Compatibility approach

This is **not currently a modified copy of the entire libgui source tree**.

Instead:

    upstream libgui
          │
          ▼
    MarcViiewLibGuiCompat
      ├── compatibility implementations
      └── minimal test application
          │
          ▼
      legacy libogc
          │
          ▼
          Wii

This keeps the upstream libgui checkout separate and makes it easier to identify which parts of the framework actually require adaptation.

Compatibility changes should remain narrowly scoped where possible. A successful hardware test does not automatically mean every libgui feature is compatible; filesystem, USB, threading, video, and other platform-specific functionality should be tested as needed.

## Deliberately excluded functionality

The compatibility test does not currently provide:

- MarcViiew application code
- ViiewLib
- NAND access
- TMD parsing
- MARC record handling
- SMB/network filesystem support

These omissions keep the test environment focused on the GUI framework itself.

## Relationship to MarcViiew

The long-term purpose is to determine whether current libgui can become the GUI foundation for MarcViiew without requiring MarcViiew to abandon its existing Wii development environment.

If the compatibility layer proves reliable, the necessary adaptations can be brought into MarcViiew incrementally rather than replacing the existing application all at once.

## Status

**Experimental — hardware compatibility testing**

The project currently builds successfully against the legacy libogc environment and has been successfully tested under Dolphin. Real Wii hardware testing is the next validation stage.

## Credits

dborth/libgui — for providing the necessary UI framework, later being used as a compatibility layer.

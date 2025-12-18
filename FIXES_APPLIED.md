# Fixes Applied to IA_Project

## Date: December 17, 2025

### Issues Identified and Fixed:

#### 1. Missing Source Files in Project Configuration
**Problem:** `studywindow.cpp`, `studywindow.h`, and `studywindow.ui` files existed in the source directory but were not included in the `IA_Project.pro` file. This caused Qt Creator to ignore these files during compilation.

**Fix:** Updated `IA_Project.pro` to include:
- `studywindow.cpp` in SOURCES
- `studywindow.h` in HEADERS  
- `studywindow.ui` in FORMS

#### 2. Incorrect Qt Version Being Used
**Problem:** The system has both Qt 6.4.2 (system-wide in `/usr`) and Qt 6.9.3 (user installation in `~/Qt/6.9.3`). Qt Creator was configured for Qt 6.9.3 but the command-line `qmake6` was defaulting to the system Qt 6.4.2, causing version mismatch and linker errors.

**Fix:** 
- Use the correct qmake from `~/Qt/6.9.3/gcc_64/bin/qmake` instead of system `qmake6`
- Created a `build.sh` script to automate building with the correct Qt version

#### 3. Mainwindow UI Layout Structure Issues
**Problem:** The `mainwindow.ui` file had incorrect XML nesting. The `modePanel` and `studyPanel` widgets were nested inside `horizontal_layout_middle` (as siblings to `deckList`) when they should have been separate items in the main `verticalLayout_3`. This caused layout formatting issues on launch.

**Fix:** Restructured the UI XML to:
- Close `horizontal_layout_middle` properly after `deckList` and its spacer
- Move `modePanel` and `studyPanel` to be direct children of `verticalLayout_3`
- Remove duplicate spacer elements

### How to Build the Project

#### From Terminal:
```bash
# Quick build (uses existing Makefile)
cd /home/brometheus/IA_Project
./build.sh

# Clean build (regenerates Makefile)
./build.sh clean
```

#### From Qt Creator:
The project should now build correctly from Qt Creator since:
1. All source files are properly included in the .pro file
2. Qt Creator is configured to use Qt 6.9.3 (Desktop Qt 6.9.3 kit)
3. The UI file structure is now valid

**Important:** If Qt Creator still shows build issues, try:
- Build > Clean All
- Build > Run qmake  
- Build > Build Project

### Running the Application:
```bash
cd /home/brometheus/IA_Project/build/Desktop_Qt_6_9_3-Debug
./IA_Project
```

### Summary of Changes:

**Modified Files:**
- `/home/brometheus/IA_Project/Development/Source_Code/IA_Project.pro` - Added studywindow files
- `/home/brometheus/IA_Project/Development/Source_Code/mainwindow.ui` - Fixed layout nesting

**New Files:**
- `/home/brometheus/IA_Project/build.sh` - Build automation script

All changes preserve existing functionality while fixing the build and layout issues.

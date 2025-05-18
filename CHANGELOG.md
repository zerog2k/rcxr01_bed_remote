# Changelog

## [0.3.2] - 2024-07-10

### Fixed
- Fixed menu highlighting in the main menu
- Corrected drawing color values for proper text visibility on highlighted backgrounds
- Ensured text is drawn after the highlight box for proper contrast

## [0.3.1] - 2024-07-10

### Changed
- Adjusted display text positioning to fit within 96x32 display area
- Changed to smaller font (profont10) for better readability on small display
- Fixed menu highlighting to match the smaller display dimensions

## [0.3.0] - 2024-07-10

### Changed
- Refactored display code to use U8G2 library instead of U8X8
- Updated LCD implementation to use U8G2 buffer-based drawing
- Modified DisplayManager to use U8G2 drawing methods and pixel-based positioning
- Updated menu highlighting using U8G2's drawing capabilities
- Adjusted PowerManager to use U8G2's power save functions

### Technical Details
- Replaced direct CAD access with U8G2's sendF method for low-level commands
- Implemented buffer handling with clearBuffer() and sendBuffer()
- Adjusted text positioning for U8G2's pixel-based coordinate system
- Updated platformio.ini to include U8G2 library dependency
"""
USD Camera Import - Quick Execute Script
=========================================

HOW TO USE:
1. Edit the USD_FILE_PATH below to point to your .usda file
2. In Unreal: Tools → Execute Python Script...
3. Select this file

The camera will import and the Level Sequence will open in Sequencer.
"""

# =============================================================================
# SET YOUR USD FILE PATH HERE
# =============================================================================

USD_FILE_PATH = r"C:/Users/cathe/Downloads/camera.usda"

# =============================================================================
# Import and run (don't edit below this line)
# =============================================================================

import unreal_usd_camera_import

# Run the import
result = unreal_usd_camera_import.import_camera(USD_FILE_PATH)
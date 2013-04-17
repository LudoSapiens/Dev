# =============================================================================
#  Copyright (c) 2006, Ludo Sapiens Inc.
#  All rights reserved.
# 
#  These coded instructions, statements, and computer programs contain
#  unpublished, proprietary information and are protected by Federal copyright
#  law. They may not be disclosed to third parties or copied or duplicated in
#  any form, in whole or in part, without prior written consent.
# =============================================================================

import BuildSystem.Defaults.Tools
from BuildSystem.Utilities.Settings import Settings
from BuildSystem.Variants.Variant import Variant

#Default directories to use
settings = Settings()
settings.intermediate_directory_objects = BuildSystem.Defaults.Tools.FileManager.path.join("out", "${PLATFORM}_${PLATFORM_FLAVOR}", "${TARGET_NAME}", "${TARGET_TYPE}", "${VARIANTS}")
settings.destination_directory_executables = settings.intermediate_directory_objects
settings.destination_directory_libraries = settings.intermediate_directory_objects

variants = Variant()

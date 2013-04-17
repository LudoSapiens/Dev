# =============================================================================
#  Copyright (c) 2006, Ludo Sapiens Inc.
#  All rights reserved.
# 
#  These coded instructions, statements, and computer programs contain
#  unpublished, proprietary information and are protected by Federal copyright
#  law. They may not be disclosed to third parties or copied or duplicated in
#  any form, in whole or in part, without prior written consent.
# =============================================================================

from optparse import OptionParser
import copy

cmd_args = []
def _cmd_args_callback(option, opt_str, value, parser):
   cmd_args = copy.copy(parser.rargs)
   del parser.rargs[:]
   parser.rargs = [] #does not work, hence the full copy+del above
   globals().update(locals()) #Update global Options.cmd_args variable

# ----------------------------------------------------------------------------
# Need to parse the options as early as possible
# ----------------------------------------------------------------------------
optionParser = OptionParser()
optionParser.add_option("-f", "--file", "--bsfile",
                        action="store", type="string", dest="bsfile",
                        help="Specifies the BSFile to use.")
optionParser.add_option("-F", "--fast",
                        action="store_true", dest="fast",
                        help="Fast mode (skips dependencies)")
optionParser.add_option("-p", "--bsproject",
                        action="store", type="string", dest="bsproject",
                        help="Specifies the BSProject to use.")
optionParser.add_option("-n", "--just-print", "--dry-run", "--recon",
                        action="store_true", dest="no_run",
                        help="Don't run commands; just print them.")
optionParser.add_option("-q", "--quiet",
                        action="store_const", const=0, dest="verbose",
                        help="Makes the execution silent (no verbose)")
optionParser.add_option("-v", "--verbose",
                        action="count", dest="verbose",
                        help="Controls the verbose level (repeat for more)")
optionParser.add_option("-V", "--variant",
                        action="append", dest="variants",
                        help="Defines a list of variants to use")
optionParser.add_option("--noisy",
                        action="store_const", const=100, dest="verbose",
                        help="Turn on all of the output traces")
optionParser.add_option("--args",
                        action="callback", callback=_cmd_args_callback,
                        help="Specifies arguments to send to the invoked command")
(opts, args) = optionParser.parse_args()

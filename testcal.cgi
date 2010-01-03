#!/bin/sh
export Notes_ExecDirectory="/Applications/Notes.app/Contents/MacOS"
export DYLD_LIBRARY_PATH=${Notes_ExecDirectory}
echo "Content-type: text/calendar"
echo ""
/Users/jpm/Auto/projects/on-github/jndcalx/runlotus

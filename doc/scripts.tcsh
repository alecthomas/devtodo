# This script implements automatic todo execution on directory change for
# tcsh, identical to the scripts.sh bash script.
#
# Thanks go to Matthew Russell who provided this script.  His usage 
# comments follow:
# 
# This could be inserted into either ~/.tcshrc or
# ~/.login. I don't think the "cwdcmd" alias works
# on csh, the forerunner to tcsh, though.
#

# Run todo on change of directory
alias cwdcmd devtodo --timeout --summary

# Run todo on login
devtodo --timeout --summary

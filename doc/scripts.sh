#
# These functions override builtin bash commands that change directories.
# The purpose of this is to show any todo items as soon as you move into a 
# directory. Quite handy.
#
# The script will also display todo items upon first login.
#
# For example, if I have some todo items in my home directory and I cd ~,
# the items will be displayed.
#
# This script should be added to either the system wide shell initialisation
# file (/etc/profile) or a user specific initialisation file (~/.bash_profile 
# or ~/.profile). In addition, if you are using X, terminals you start up
# should be login terminals (typically -ls, --ls or something to that effect).
#

# Only display every X (10) seconds, and display a maximum of one line per note.
# The timeout period can be modified by putting
#    timeout <N>
# in your ~/.todorc.
TODO_OPTIONS="--timeout --summary"

cd ()
{
	builtin cd "$@"
	RV=$?
	[ $RV = 0 -a -r .todo ] && devtodo ${TODO_OPTIONS}
	return $RV
}

pushd ()
{
	builtin pushd "$@"
	RV=$?
	[ $RV = 0 -a -r .todo ] && devtodo ${TODO_OPTIONS}
	return $RV
}

popd ()
{
	builtin popd "$@"
	RV=$?
	[ $RV = 0 -a -r .todo ] && devtodo ${TODO_OPTIONS}
	return $RV
}              

# Run todo initially upon login
devtodo ${TODO_OPTIONS}

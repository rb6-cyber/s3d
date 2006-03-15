#!/bin/sh

# $zfxce: autogen.sh,v 1.9 2005/10/28 11:58:33 andreaskohn Exp $

# Regenerate configuration files.
#
# If the user has set environment variables for
# AUTOMAKE, AUTOCONF, etc, try to use these
#
# Run configure aftwerwards
#

# (c) Andreas Kohn, 2004-2006.
#
# NO WARRANTY. THIS PROGRAM IS FREE SOFTWARE, AND YOU MAY USE AND/OR MODIFY
# IT AS LONG AS YOU KEEP THE ABOVE COPYRIGHT NOTICE AND THIS DISCLAIMER IN
# PLACE.

#
# Globals
#
SRCDIR=`pwd`
WSDIR=$SRCDIR
run_configure=1
quiet=0
configure_args=""

#
# Functions
#
error() {
	echo "ERROR: $*"
	exit 1
}
note() {
	[ $quiet -ne 0 ] || echo $1
}

require_tool() {
	# Checks if $1 is in PATH, or overridden
	# using a environment variable with $1 in uppercase letters
	# Sets ${$1-uppercase} to the full path of the tool
	# and fails with an error message if no such tool was found.
	
	tool=$1
	varname=`echo $tool | tr "[:lower:]" "[:upper:]"`
	eval varvalue=\$$varname
	[ $quiet -ne 0 ] || printf "checking for $tool..."
	if [ -z "$varvalue" ]; then
		varvalue=`which $tool`
	fi
	if [ -z "$varvalue" ]; then
		error "$tool not found. Check your \$PATH or set \$$varname to the full location of $tool."
	fi
	if [ -x "$varvalue" ]; then
		eval $varname=$varvalue
	else
		error "$tool not executable (tried '$varvalue'). Check your \$PATH or set \$$varname to the full location of $tool."
	fi
	note "$varvalue"
	return 0
}	

require_success() {
	# Run $1, and require that test $2 succeeds afterwards, else exit printing $3
	# default for $2 is '$? -eq 0', default for $3 is '$1 failed'
	run=$1
	if [ -z "$2" ]; then
		check='$? -eq 0'
	else
		check=$2
	fi
	if [ -z "$3" ]; then
		message="'$1' failed."
	else
		message=$3
	fi
	note "running $run..."
	eval $run
	if [ $? -ne 0 ]; then
		error $message
	fi
	eval test $check
	if [ $? -ne 0 ]; then
		error $message
	fi
	return 0
}

check_env() {
	[ $quiet -ne 0 ] || echo "checking and preparing environment..."

	# Do sanity checks.
	if [ ! -f $SRCDIR/Makefile.am ]; then
		error "$SRCDIR/Makefile.am missing."
	fi
	if [ ! -f $SRCDIR/configure.ac ]; then
		error "$SRCDIR/configure.ac missing."
	fi

	require_tool aclocal
	require_tool autoheader
	require_tool autoconf
	require_tool automake
	require_tool libtoolize

	return 0
}

usage() {
	echo "Usage: `basename $0` [-h|--help] [-q|--quiet] [--configure|--no-configure] ... "
	if [ "$1" = "-v" ]; then
		cat <<EOF
Generate a suitable build infrastructure for s3d using
GNU autotools.

Option summary
--configure
	Run the generated configure script, using non-recognized remaining arguments
	(this is the default setting)
--no-configure
	Do not run configure afterwards
-h
--help
	Show this help
-q
--quiet
	Be quiet
		
EOF
	fi
}

#
# Execution starts here
#
while [ $# -gt 0 ]; do
	case "$1" in 
		--no-configure)
			run_configure=0
			;;
		--configure)
			run_configure=1
			;;
		--help|-h)
			usage -v
			exit 1
			;;
		--quiet|-q)
			quiet=1
			;;
		*)
			configure_args="$configure_args $1"
			;;
	esac
	shift
done

check_env

note "generating build scripts..."
	
require_success "test -f $WSDIR/ltmain.sh || ${LIBTOOLIZE} --force --copy" "\$? -eq 0 -a -f $WSDIR/ltmain.sh" "${LIBTOOLIZE} failed."
require_success "${ACLOCAL} -I $WSDIR/config ${ACLOCAL_FLAGS}"
require_success "${AUTOHEADER}"
require_success "${AUTOMAKE} --foreign --include-deps --add-missing --copy"
require_success "${AUTOCONF}"
if [ $run_configure -eq 1 ]; then
        require_success "./configure $configure_args"
fi


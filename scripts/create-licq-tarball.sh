#!/bin/bash
#
# Script to create a source tarball from Licq's svn repository.
# Latest version: http://svn.licq.org/svn/trunk/scripts/
#
# Copyright (c) 2007 Erik Johansson <erijo@licq.org>
# Distributed under the terms of the GNU GPL version 2.
#

### DEFAULT SETTINGS ###

# Where to save the tarball(s)
TARDIR="."

# What to name the tarball (REV is replaced with the svn revision)
# Note: Don't add .tar.gz or .tar.bz2
TARNAME="licq-1.3.5~rREV"
#TARNAME="licq-1.3.5-rc2"

# Archives to create (1 = create; 0 = don't create)
CREATE_GZ=0
CREATE_BZ2=0

# Sign archives using default GPG key (1 = sign; 0 = don't sign)
SIGN=0

# What revision to export
LICQREV="HEAD"

# Ownership of files in tarball
OWNER="root"
GROUP="root"

# Licq repository
REPO="http://svn.licq.org/svn"

### END SETTINGS ###

# Prints the usage
function usage()
{
    echo "Usage: $0 [-r rev] [-o dir] [-n name] [-g] [-b] [-s]"
    echo "  -h, --help   This message"
    echo "  -r rev       Create tarball from revision rev (default: ${LICQREV})"
    echo "  -o dir       Save tarball in directory dir (default: ${TARDIR})"
    echo "  -n name      Name the tarball name (default: ${TARNAME})"
    echo "               REV is replaced with the svn revision"
    echo "  -g, --gzip   Create a tar.gz archive"
    echo "  -b, --bzip2  Create a tar.bz archive"
    echo "  -s, --sign   Sign archive(s) with default GPG key"
}

if [ $# -eq 0 ]; then
    echo "$0: Missing required argument (-g and/or -b)"
    echo "Try \`$0 --help' for more information"
    exit 2
fi

# Parse command line options
args=$(getopt -n "$0" -o h,r:,o:,n:,g,b,s -l help -- $*)
if [ $? -ne 0 ]; then
    echo ""
    usage
    exit 1
fi

set -- $args
while [ $# -gt 0 ]; do
    case $1 in
	-h|--help) usage; exit 0 ;;
	-r) LICQREV=$(eval echo $2); shift ;;
	-o) TARDIR=$(eval echo $2); shift ;;
	-n) TARNAME=$(eval echo $2); shift ;;
	-g|--gzip) CREATE_GZ=1 ;;
	-b|--bzip2) CREATE_BZ2=1 ;;
	-s|--sign) SIGN=1 ;;
	--) ;;
	*) echo "$0: unknown option '$1'"; exit 1 ;;
    esac
    shift
done

if [ $CREATE_GZ -eq 0 -a $CREATE_BZ2 -eq 0 ]; then
    echo "$0: You must choose to create a gzip and/or bzip2 archive"
    echo ""
    usage
    exit 1
fi

# Workdir
TMPDIR=$(mktemp -d) || failed "mktemp -d"

# Remove workdir
function cleanup()
{
   echo "Removing ${TMPDIR}"
   rm -rf "${TMPDIR}"
}

# Echos "$1 failed" or "failed" and then exits.
function failed()
{
   if [ -z $1 ]; then
      echo "failed"
   else
      echo "$1 failed"
   fi
   cleanup
   exit 1
}

SVNREV=$(svn info -r"${LICQREV}" "${REPO}" | grep "^Revision:" | awk '{print $2}') || failed

DIRNAME="${TARNAME//REV/$SVNREV}"
TARNAME="${TARNAME//REV/$SVNREV}"
LICQDIR="${TMPDIR}/${DIRNAME}"

function exit_if_exists()
{
   if [ -e "$1" ]; then
      echo "$1 already exists"
      exit 1
   fi
}

function svnexport()
{
   echo -n "Exporting $1 (r${SVNREV})... "
   svn export --ignore-externals -r"${SVNREV}" -q "${REPO}/$1" "${LICQDIR}/$2" || failed
   echo "done"
}

function makecvs()
{
   echo -n "Running make -f $1/Makefile.cvs... "
   make -C "${LICQDIR}/$1" -f "${LICQDIR}/$1/Makefile.cvs" > /dev/null 2>&1 || failed
   rm -rf "${LICQDIR}/$1/autom4te.cache"
   rm -f "${LICQDIR}/$1/Makefile.cvs"
   echo "done"
}

test ${CREATE_GZ}  -ne 0 && exit_if_exists "${TARDIR}/${TARNAME}.tar.gz"
test ${CREATE_BZ2} -ne 0 && exit_if_exists "${TARDIR}/${TARNAME}.tar.bz2"

svnexport "trunk/licq" ""
svnexport "trunk/admin" "admin"

makecvs "."

for plugin in auto-reply console email msn osd qt-gui rms; do
   svnexport "trunk/${plugin}" "plugins/${plugin}"
   makecvs "plugins/${plugin}"
done

rm -f "${LICQDIR}/admin/Makefile.common"

for plugin in licqweb; do
   svnexport "trunk/${plugin}" "plugins/${plugin}"
done

#svnexport "branches/qt-gui_qt4" "plugins/qt4-gui"

echo "Creating tarball ${TARNAME}.tar"
tar --owner "${OWNER}" --group "${GROUP}" -C "${TMPDIR}" -cf "${TMPDIR}/${TARNAME}.tar" "${DIRNAME}" || failed

function sign()
{
   if [ ${SIGN} -ne 0 ]; then
      echo "Signing $1"
      gpg --sign --armor --detach-sign --output "$1.sign" "$1" || failed "signing"
   fi
}

if [ ${CREATE_GZ} -ne 0 ]; then
   echo -n "Creating ${TARDIR}/${TARNAME}.tar.gz... "
   cp "${TMPDIR}/${TARNAME}.tar" "${TARDIR}" || failed "copy tarball"
   gzip --best "${TARDIR}/${TARNAME}.tar" || failed
   echo "done"

   sign "${TARDIR}/${TARNAME}.tar.gz"
fi

if [ ${CREATE_BZ2} -ne 0 ]; then
   echo -n "Creating ${TARDIR}/${TARNAME}.tar.bz2... "
   cp "${TMPDIR}/${TARNAME}.tar" "${TARDIR}" || failed "copy tarball"
   bzip2 --best "${TARDIR}/${TARNAME}.tar" || failed
   echo "done"

   sign "${TARDIR}/${TARNAME}.tar.bz2"
fi

cleanup

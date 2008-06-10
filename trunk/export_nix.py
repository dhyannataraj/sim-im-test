#!/usr/bin/env python
# instead of non-working cpack: svn export for *nix systems
# usage: export_nix.py TARGET_DIR
# the scripts excludes windows-specific files

import os
import sys
import subprocess
import shutil
import fnmatch

target_dir = sys.argv[1]
PATTERNS = [ "*.vcproj", "*.sln", "install_win32", "win32" ]

print "Exporting to target directory '%s'" % target_dir
subprocess.check_call( ["svn", "export", os.curdir, target_dir ] )

for root, dirs, files in os.walk( target_dir ):
    for pattern in PATTERNS:
        for relname in [ os.path.join( root, name ) for name in
                         fnmatch.filter( dirs + files, pattern ) ]:
            if os.path.isdir( relname ):
                print "Deleting dir '%s'" % relname
                shutil.rmtree( relname )
            else:
                print "Deleting '%s'" % relname
                os.remove( relname )

print "Done exporting to target directory '%s'" % target_dir

basename = os.path.basename( target_dir )
archivename = basename + ".tar.bz2"
archivedir = os.path.dirname( os.path.abspath( target_dir ) )
archivepath = os.path.join( archivedir, archivename )

print "Packing '%s' to '%s'" % ( target_dir, archivepath )
subprocess.check_call( ["tar", "--bzip2", "-c", "-f", archivename, basename ], cwd = archivedir )
print "Done"

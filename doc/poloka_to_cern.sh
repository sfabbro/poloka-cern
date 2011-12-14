#!/bin/bash 

git rm autogen.sh ChangeLog config.h.in configure.ac Makefile.am README stamp-h.in 
git rm -r bindings
git rm -r cmt
git rm -r dao
git rm -r flat
git rm -r imagemagick_utils
git rm -r m4
git rm -r mc
git rm -r src
git rm -r src_base
git rm -r src_utils
git rm -r lapack_stuff
git rm -r telinst 
git rm -r psf 
git rm -r lc 
git rm -r simphot 
git rm -r utils 
git rm -r datacards
git rm cern_stuff/Makefile.am 
git rm cern_utils/Makefile.am 

git mv cern_stuff src 
git mv cern_utils tools


# doc 
git rm doc/Doxyfile.in  doc/git-tutorial.org  doc/install_guide.txt  doc/Makefile.am  doc/README.db  doc/refman.ps  

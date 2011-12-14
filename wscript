# -*- mode: python; -*- 


import os
import os.path as op
import sys
import commands
import shlex

import Options
import Configure
import frogsutils 


APPNAME  = 'poloka-cern'
VERSION  = '0.1.0'
top   = '.'
out   = frogsutils.get_out_name()
description = "Cern utils (l2tup and co)"
requirements = [
    ('poloka-core', '0.1.0', True), 
    ]
debug = True
optimize = 3


def options(opt):    
    opt.load('frogs')

def configure(conf):    
    conf.load('frogs')    
    conf.check_cc(lib='z', msg='Checking for zlib')    
    conf.check_cernlib()
    conf.check_packages(requirements)
    conf.write_config_header("config.h")

def build(bld):
    bld.add_subdirs( ['src', 'tools'])
    gen_pkgconfig(bld)


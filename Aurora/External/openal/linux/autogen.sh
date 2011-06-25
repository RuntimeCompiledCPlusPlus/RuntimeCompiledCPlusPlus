#!/bin/sh

autoconf
autoheader configure.in > foo.in
mv foo.in config.h.in # don't ask

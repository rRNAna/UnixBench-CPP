#! /bin/sh
#########################################################################################################
#
# Filename      : multi.sh
# Version      : 3.5
# Creation Date : Apr 22, 2025
# Author: rRNA
# Description   : Improved error checking and re-wrote the code using a modern scripting language.
#
###########################################################################################################
###############################################################################
#  The BYTE UNIX Benchmarks - Release 3
#          Module: multi.sh   SID: 3.4 5/15/91 19:30:24
#          
###############################################################################
# Bug reports, patches, comments, suggestions should be sent to:
#
#	Ben Smith or Rick Grehan at BYTE Magazine
#	ben@bytepb.UUCP    rick_g@bytepb.UUCP
#
###############################################################################
#  Modification Log:
#
###############################################################################


UB_BINDIR="$(cd "$(dirname "$0")"; pwd)"  # 强制绝对路径
instance=1

while [ $instance -le $1 ]; do
    /bin/sh "$UB_BINDIR/tst.sh" &
    instance=$(expr $instance + 1)
done

wait
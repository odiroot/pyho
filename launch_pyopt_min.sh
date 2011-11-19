#!/bin/sh
. ./common_launch.sh

./pyopt_min/optimizer_ga.py \
    -coil ${COIL_FILE} \
    -grid ${GRID_FILE} \
    -fine ${FINE} \
    -ngen ${NGEN} \
    -popsize ${POPSIZE} \
    -seed ${SEED} \
    -xml ${OUT_XML} \
    -cblock ${OUT_CBL} \
    -stopflag ${STOP_FILE} \
    $@
##


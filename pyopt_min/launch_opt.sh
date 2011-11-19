#!/bin/sh
DIST_DIR="../dist/unix"
. ../common_launch.sh

./optimizer_ga.py \
    -coil ${COIL_FILE} \
    -grid ${GRID_FILE} \
    -fine ${FINE} \
    -ngen ${NGEN} \
    -popsize ${POPSIZE} \
    -seed ${SEED} \
    $@
##


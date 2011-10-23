#!/bin/sh
DIR="$( cd -P "$( dirname "$0" )" && pwd )"
ASSETS="${DIR}/../assets"

OPT_BIN="${DIR}/min_optimizer.py"
COIL_FILE="${ASSETS}/Coil_for_Opt.param"
GRID_FILE="${ASSETS}/Coil_for_Opt.grid"

OPTIMAL_XML="/tmp/optimal_coil.xml"
OPTIMAL_CBL="/tmp/optimal_coil.cbl"
STOP_FLAG="/tmp/aFlag"


exec ${OPT_BIN} -coil ${COIL_FILE} -grid ${GRID_FILE} \
    -xml ${OPTIMAL_XML} -cblock ${OPTIMAL_CBL} \
    -stopflag ${STOP_FLAG} \
    -fine 7 -ngen 50 -popsize 200 \
    $@


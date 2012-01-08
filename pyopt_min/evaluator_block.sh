#!/bin/sh
DIST_DIR="./../assets"
OUT_DIR="${DIST_DIR}/output"

COIL_FILE="${DIST_DIR}/Coil_for_Opt.param"
GRID_FILE="${DIST_DIR}/Coil_for_Opt.grid"
OUT_XML="${OUT_DIR}/optimal_coil.xml"
OUT_CBL="${OUT_DIR}/optimal_coil.cbl"
FINE=7

./evaluator_block.py \
    -coil ${COIL_FILE} \
    -grid ${GRID_FILE} \
    -fine ${FINE} \
    -xml ${OUT_XML} \
    -cblock ${OUT_CBL} \
    $@
##


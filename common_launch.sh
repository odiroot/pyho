DIST_DIR="./assets"
OUT_DIR="${DIST_DIR}/output"

COIL_FILE="${DIST_DIR}/Coil_for_Opt.param"
GRID_FILE="${DIST_DIR}/Coil_for_Opt.grid"
OUT_XML="${OUT_DIR}/optimal_coil.xml"
OUT_CBL="${OUT_DIR}/optimal_coil.cbl"
STOP_FILE="/tmp/aFlag"
FINE=7
NGEN=${NGEN:-10}
POPSIZE=${POPSIZE:-20}
SEED=1337


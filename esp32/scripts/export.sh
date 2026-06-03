if [ -z ${SPOTILED_EXPORT_DONE} ]; then
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    PATH="${SCRIPT_DIR}:${PATH}"
    SPOTILED_EXPORT_DONE=1
fi

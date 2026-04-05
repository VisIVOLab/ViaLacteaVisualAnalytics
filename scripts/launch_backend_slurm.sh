#!/usr/bin/env bash
# ──────────────────────────────────────────────────────────────────────────────
# launch_backend_slurm.sh
# R6: Launch the VisIVO backend on an HPC compute node via SLURM and print
#     the SSH port-forwarding command the user needs to run locally.
#
# Usage:
#   bash scripts/launch_backend_slurm.sh [options]
#
# Options (environment variables or flags):
#   --partition  PARTITION  SLURM partition   (default: $VISIVO_PARTITION or "compute")
#   --time       HH:MM:SS   Wall-clock limit  (default: $VISIVO_TIME or "04:00:00")
#   --mem        MB         Memory per node   (default: $VISIVO_MEM or "32G")
#   --cpus       N          CPUs per task     (default: $VISIVO_CPUS or 8)
#   --port       PORT       Backend TCP port  (default: $VISIVO_PORT or 8000)
#   --bind       PATH       Host path to bind into the container (default: $HOME)
#   --sif        PATH       Apptainer .sif image (default: apptainer/backend.sif)
#   --token      TOKEN      Auth token (auto-generated if absent)
#   --help                  Print this help
#
# Example:
#   bash scripts/launch_backend_slurm.sh --partition gpu --cpus 16 --mem 64G
#
# The script:
#   1. Submits a SLURM batch job that starts the Apptainer container.
#   2. Polls /health every 5 s until the backend is reachable (max 5 min).
#   3. Prints the SSH tunnel command for the user to run on their laptop.
# ──────────────────────────────────────────────────────────────────────────────
set -euo pipefail

# ── Defaults ──────────────────────────────────────────────────────────────────
PARTITION="${VISIVO_PARTITION:-compute}"
TIME="${VISIVO_TIME:-04:00:00}"
MEM="${VISIVO_MEM:-32G}"
CPUS="${VISIVO_CPUS:-8}"
PORT="${VISIVO_PORT:-8000}"
BIND_PATH="${VISIVO_BIND:-$HOME}"
SIF="${VISIVO_SIF:-$(dirname "$0")/../apptainer/backend.sif}"
TOKEN="${VISIVO_TOKEN:-}"
LOG_DIR="${VISIVO_LOG_DIR:-$HOME/.visivo_logs}"
POLL_MAX=60   # max poll attempts × 5 s = 5 min

# ── Argument parsing ──────────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --partition) PARTITION="$2"; shift 2 ;;
        --time)      TIME="$2";      shift 2 ;;
        --mem)       MEM="$2";       shift 2 ;;
        --cpus)      CPUS="$2";      shift 2 ;;
        --port)      PORT="$2";      shift 2 ;;
        --bind)      BIND_PATH="$2"; shift 2 ;;
        --sif)       SIF="$2";       shift 2 ;;
        --token)     TOKEN="$2";     shift 2 ;;
        --help|-h)
            grep '^#' "$0" | sed 's/^# \{0,2\}//'
            exit 0
            ;;
        *) echo "Unknown option: $1" >&2; exit 1 ;;
    esac
done

# ── Generate token if not provided ───────────────────────────────────────────
if [[ -z "$TOKEN" ]]; then
    TOKEN="$(python3 -c 'import secrets; print(secrets.token_urlsafe(32))')"
fi

TOKEN_FILE="$HOME/.visivo_token"
echo "$TOKEN" > "$TOKEN_FILE"
chmod 600 "$TOKEN_FILE"
echo "[visivo] Token saved to $TOKEN_FILE"

# ── Verify the Apptainer image exists ────────────────────────────────────────
if [[ ! -f "$SIF" ]]; then
    echo "[visivo] ERROR: Apptainer image not found: $SIF"
    echo "[visivo] Build it first:"
    echo "         apptainer build $SIF apptainer/backend.def"
    exit 1
fi

# ── Create log directory ──────────────────────────────────────────────────────
mkdir -p "$LOG_DIR"
SLURM_LOG="$LOG_DIR/visivo_backend_%j.log"

# ── Write the SLURM job script ────────────────────────────────────────────────
JOB_SCRIPT="$(mktemp /tmp/visivo_slurm_XXXXXX.sh)"
cat > "$JOB_SCRIPT" <<SLURM_EOF
#!/bin/bash
#SBATCH --job-name=visivo_backend
#SBATCH --partition=${PARTITION}
#SBATCH --time=${TIME}
#SBATCH --mem=${MEM}
#SBATCH --cpus-per-task=${CPUS}
#SBATCH --output=${SLURM_LOG}
#SBATCH --error=${SLURM_LOG}

echo "[visivo] Job started on \$(hostname) at \$(date)"
echo "[visivo] Port: ${PORT}"

# Export token so the backend picks it up.
export VISIVO_TOKEN="${TOKEN}"
export VISIVO_WORKERS="${CPUS}"

apptainer run \\
    --bind "${BIND_PATH}:${BIND_PATH}" \\
    "${SIF}" \\
    --host 0.0.0.0 \\
    --port "${PORT}"
SLURM_EOF

# ── Submit the job ────────────────────────────────────────────────────────────
echo "[visivo] Submitting SLURM job..."
JOB_ID="$(sbatch --parsable "$JOB_SCRIPT")"
rm -f "$JOB_SCRIPT"
echo "[visivo] Job ID: $JOB_ID"

# ── Wait for the compute node to be assigned ──────────────────────────────────
echo "[visivo] Waiting for compute node assignment..."
NODE=""
for _ in $(seq 1 30); do
    NODE="$(squeue -j "$JOB_ID" -h -o '%N' 2>/dev/null || true)"
    if [[ -n "$NODE" && "$NODE" != "None" ]]; then
        break
    fi
    sleep 2
done

if [[ -z "$NODE" || "$NODE" == "None" ]]; then
    echo "[visivo] ERROR: Could not determine compute node. Check: squeue -j $JOB_ID"
    exit 1
fi

echo "[visivo] Compute node: $NODE"

# ── Poll the health endpoint ──────────────────────────────────────────────────
echo "[visivo] Waiting for backend to become ready (polling every 5 s)..."
READY=false
for attempt in $(seq 1 "$POLL_MAX"); do
    if ssh -o StrictHostKeyChecking=no -o BatchMode=yes \
           "$NODE" "curl -sf -H 'X-Visivo-Token: ${TOKEN}' http://localhost:${PORT}/v1/health" \
           >/dev/null 2>&1; then
        READY=true
        break
    fi
    echo "[visivo]  attempt $attempt/$POLL_MAX – not ready yet..."
    sleep 5
done

if [[ "$READY" != "true" ]]; then
    echo "[visivo] ERROR: Backend did not become ready within $((POLL_MAX * 5)) seconds."
    echo "[visivo] Check the log: $LOG_DIR/visivo_backend_${JOB_ID}.log"
    exit 1
fi

echo ""
echo "══════════════════════════════════════════════════════════════════════"
echo "  VisIVO backend is READY"
echo "══════════════════════════════════════════════════════════════════════"
echo ""
echo "  Run this SSH tunnel command ON YOUR LAPTOP:"
echo ""
echo "    ssh -N -L ${PORT}:${NODE}:${PORT} $(hostname -f)"
echo ""
echo "  Then set in the VisIVO Qt client:"
echo "    Backend URL : http://localhost:${PORT}"
echo "    Auth token  : $(cat "$TOKEN_FILE")"
echo ""
echo "  Or set the environment variable before launching the client:"
echo "    export VISIVO_TOKEN=$(cat "$TOKEN_FILE")"
echo ""
echo "  Job log: $LOG_DIR/visivo_backend_${JOB_ID}.log"
echo "  Cancel:  scancel $JOB_ID"
echo "══════════════════════════════════════════════════════════════════════"

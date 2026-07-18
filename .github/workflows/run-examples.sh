#!/usr/bin/env bash
# Runs every built DXVK example for a fixed number of frames and reports a
# summary. Each example exits 0 once it has rendered DXVK_EXAMPLE_FRAMES frames,
# so a non-zero exit means it failed to create a device/swap chain or crashed.
#
# Usage: run-examples.sh <examples-dir>

set -u

dir="${1:-build/examples}"
: "${DXVK_EXAMPLE_FRAMES:=10}"
export DXVK_EXAMPLE_FRAMES

if [ ! -d "$dir" ]; then
  echo "error: '$dir' does not exist"
  exit 1
fi

# Executables only. DXVK's own log files are also named example_*, so match on
# the executable bit and exclude libraries/logs/debug files explicitly.
mapfile -t examples < <(find "$dir" -maxdepth 1 -name 'example_*' -type f -perm -u+x \
  ! -name '*.log' ! -name '*.dll' ! -name '*.pdb' ! -name '*.so*' | sort)

if [ "${#examples[@]}" -eq 0 ]; then
  echo "error: no examples found in '$dir'"
  exit 1
fi

pass=0
fail=0
failed=()

for exe in "${examples[@]}"; do
  name="$(basename "$exe")"
  printf '::group::%s\n' "$name"

  if "$exe"; then
    printf '\xe2\x9c\x93 %s\n' "$name"
    pass=$((pass + 1))
  else
    rc=$?
    printf '\xe2\x9c\x97 %s (exit %d)\n' "$name" "$rc"
    fail=$((fail + 1))
    failed+=("$name")
  fi

  printf '::endgroup::\n'
done

echo "--------------------------------------------"
echo "examples: ${#examples[@]}  passed: $pass  failed: $fail"

if [ "$fail" -ne 0 ]; then
  printf 'failed: %s\n' "${failed[*]}"
  exit 1
fi

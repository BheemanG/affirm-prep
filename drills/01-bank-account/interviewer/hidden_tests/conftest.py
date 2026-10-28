import os
import sys
from pathlib import Path

# Run against the candidate's code by default; set IMPL_DIR to point at the
# reference solution (or any other directory containing account.py).
_default = Path(__file__).resolve().parents[2] / "src"
sys.path.insert(0, os.environ.get("IMPL_DIR", str(_default)))

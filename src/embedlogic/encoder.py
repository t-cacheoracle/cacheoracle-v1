import sys
import os
import subprocess
from pathlib import Path

VENV_DIR = Path(__file__).parent / ".venv"
VENV_PYTHON = VENV_DIR / "bin" / "python"

# If not already running inside the venv, bootstrap it and re-exec
if sys.executable != str(VENV_PYTHON):
    if not VENV_PYTHON.exists():
        subprocess.check_call([sys.executable, "-m", "venv", str(VENV_DIR)])
        subprocess.check_call([str(VENV_PYTHON), "-m", "pip", "install", "sentence-transformers"],
                              stdout=subprocess.DEVNULL)
    os.execv(str(VENV_PYTHON), [str(VENV_PYTHON)] + sys.argv)

from sentence_transformers import SentenceTransformer

model = SentenceTransformer("all-MiniLM-L6-v2")

def encode(text: str) -> list[float]:
    embedding = model.encode(text)
    return embedding.tolist()

if __name__ == "__main__":
    query = sys.argv[1]
    result = encode(query)
    print(result)

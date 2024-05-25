import os


def get_executable_name(path: str) -> str:
    """
    Adds .exe extension to the path if running on Windows and the path does not already end with .exe
    """
    if os.name == "nt":  # Check if the OS is Windows
        if not path.lower().endswith(".exe"):
            path += ".exe"
    return path

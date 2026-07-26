# Legacy Applications

These inherited examples are retained for protocol and migration research:

- `x10-tk.py` is a Python 2/Tk application with a hardcoded sample host and no
  fault recovery.
- `mochad.scr` uses fixed process/FIFO paths, stops all `mochad` processes, and
  contains obsolete udev integration advice.

They are not maintained applications, are not installed, and must not be used
as deployment guidance. Supported integrations should use the daemon protocol
or separately maintained bridge projects.

#  Make `import pokemon_automation` work when running pytest from the
#  PythonBindings directory without installing the package.
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))

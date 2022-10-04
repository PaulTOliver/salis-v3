#!/bin/bash

. venv/bin/activate
python -Bm pytest -p no:cacheprovider --forked --verbose
deactivate

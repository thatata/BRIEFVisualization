#!/bin/bash
cd ../../../collected_data/
ls -Art | tail -n 1 > ../app/calibration_actuation/build/mostRecent.txt
cd ../app/calibration_actuation/src/

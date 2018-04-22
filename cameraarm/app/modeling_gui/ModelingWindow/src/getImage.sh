#!/bin/bash
cd ../../../../collected_data/
ls -Art | tail -n 1 > ../app/modeling_gui/ModelingWindow/build/mostRecent.txt
cd ../app/modeling_gui/ModelingWindow/build/

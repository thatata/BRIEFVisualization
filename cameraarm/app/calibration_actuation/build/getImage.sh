#!/bin/bash

rm ../../collected_data/mostRecent.txt

ls -Art | tail -n 1 > ../../collected_data/mostRecent.txt

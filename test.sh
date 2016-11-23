#!/bin/bash

./pgrep 3 "Ahoj.*" 1 "[0-9][0-9]* .*" 2 "[^0-9][^0-9]* .*" 3 < input_test.txt 

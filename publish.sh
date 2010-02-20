#!/bin/bash
git gc
make clean
rm ../cpp.zip
zip -r ../cpp.zip .

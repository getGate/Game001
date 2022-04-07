#!/bin/sh

find . -name "*.vcxproj" -exec rm -rf {} \;
find . -name "*.sln" -exec rm -rf {} \;
find . -name ".vs" -exec rm -rf {} \;
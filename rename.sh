#!/bin/bash

for file in $(find . -iname "*.cpp"); do
    new_file="${file%.cpp}.hpp"
    echo "$file"
    echo "$new_file"
    git mv "$file" "$new_file"
done

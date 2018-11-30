#!/bin/bash
echo "Installing dependencies on Fedora:"
bash ./install_fedora_dependencies.sh

echo "Building project in coverage mode:"
bash ./buildrelease.sh

echo "Cppcheck analysis:"
bash ./cppcheck.sh
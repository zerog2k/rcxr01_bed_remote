#!/bin/bash
# Simple script to check for compilation errors

echo "Checking for compilation errors..."

# List all source files
echo "Source files:"
find src -name "*.cpp" -o -name "*.h" | sort

# Check for missing includes
echo -e "\nChecking for missing includes..."
grep -n "#include" src/*.cpp src/*.h

# Check for potential issues
echo -e "\nChecking for potential issues..."
grep -n "extern" src/*.cpp src/*.h

echo -e "\nDone checking."
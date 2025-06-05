#!/bin/bash

echo "=== VERIFYING DIJKSTRA PROJECT FIXES ==="
echo "Date: $(date)"
echo

# Change to project root directory
cd /home/vitorette/Documentos/3semestre/EAD2/DijkstrasProject

# Check if project builds
echo "1. Testing compilation..."
if ninja -C builddir > /tmp/build.log 2>&1; then
    echo "✅ Project builds successfully"
else
    echo "❌ Build failed"
    cat /tmp/build.log
    exit 1
fi

# Check if executable exists
if [ -f "./builddir/DijikstrasProject" ]; then
    echo "✅ Executable created successfully"
else
    echo "❌ Executable not found"
    exit 1
fi

# Check test files
echo
echo "2. Testing OSM file..."
if [ -f "test_data/simple_test.osm" ]; then
    echo "✅ Test OSM file available"
    echo "   - Contains $(grep -c '<node' test_data/simple_test.osm) nodes"
    echo "   - Contains $(grep -c '<way' test_data/simple_test.osm) ways"
else
    echo "❌ Test OSM file missing"
fi

# Check test programs
echo
echo "3. Testing debug programs..."
if [ -f "./test/test_simple_selection" ]; then
    echo "✅ Point selection test compiled"
else
    echo "❌ Point selection test missing"
fi

if [ -f "./test/test_events" ]; then
    echo "✅ Event detection test compiled"
else
    echo "❌ Event detection test missing"
fi

# Verify key fixes in main.c
echo
echo "4. Verifying code fixes..."
if grep -q "DEBUG: Button press" src/main.c; then
    echo "✅ Debug output added to button press handler"
else
    echo "❌ Debug output missing from button press handler"
fi

if grep -q "DEBUG: Ctrl+click detected" src/main.c; then
    echo "✅ Ctrl+click detection logic implemented"
else
    echo "❌ Ctrl+click detection logic missing"
fi

if grep -q "find_closest_point" src/main.c; then
    echo "✅ Point finding function integrated"
else
    echo "❌ Point finding function missing"
fi

echo
echo "=== VERIFICATION COMPLETE ==="
echo
echo "MANUAL TESTING INSTRUCTIONS:"
echo "1. Run: ./builddir/DijikstrasProject"
echo "2. Load OSM file: test_data/simple_test.osm"
echo "3. Test Ctrl+click on points in the visualization"
echo "4. Check console output for debug messages"
echo "5. Verify points turn green (start) and red (end)"
echo
echo "Expected behavior:"
echo "- First Ctrl+click selects start point (green)"
echo "- Second Ctrl+click selects end point (red)"  
echo "- Third Ctrl+click resets and starts over"
echo "- Console shows detailed event detection logs"

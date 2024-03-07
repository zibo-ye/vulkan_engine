#!/bin/zsh

# Get the full path of the directory where the script is located
SCRIPT_DIR=$(dirname "$(realpath "$0")")

source ~/.zshrc

# Change the working directory to the parent of the script's directory
cd "$SCRIPT_DIR"/..

# Execute the command
xmake run Main -scene ../s72/examples/sphereflake_animated.s72 -headless "script/example.events"

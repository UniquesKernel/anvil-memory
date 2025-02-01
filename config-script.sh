#!/bin/bash

# Create main project directories
mkdir -p {src/{core,asm},tests/python,include}

# Create empty main.c file
cat >src/main.c <<'EOL'
#include <stdio.h>

int main(void) {
    return 0;
}
EOL

# Create empty __init__.py for Python package
touch tests/python/__init__.py

# Create empty placeholder files to ensure git tracks directories
touch src/core/.gitkeep
touch src/asm/.gitkeep
touch include/.gitkeep

mkdir -p ./docs

# Create example header with Doxygen documentation
cat >include/example.h <<'EOL'
/**
 * @file example.h
 * @brief Example header file with Doxygen documentation
 *
 * This is a detailed description of the header file.
 * It demonstrates proper Doxygen documentation style.
 */

#ifndef EXAMPLE_H
#define EXAMPLE_H

/**
 * @brief Example function that demonstrates documentation
 *
 * This is a more detailed description of the function.
 * It can span multiple lines and include detailed information.
 *
 * @param x First parameter description
 * @param y Second parameter description
 * @return Description of return value
 *
 * @note Any special notes about the function
 * @warning Any warnings about the function
 */
int example_function(int x, int y);

#endif /* EXAMPLE_H */
EOL

echo "Project structure created successfully"

cat >src/core/example.c <<'EOL'
#include "example.h"

int example_function(int x, int y) {
  return x + y;
}
EOL

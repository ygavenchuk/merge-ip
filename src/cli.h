/*
 * Copyright 2024 Yurii Havenchuk.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdbool.h>

typedef struct {
    bool help;
    bool debug;
    const char *file;
} CommandLineOptions;


/**
 * @brief Prints the usage message for the program.
 *
 * This function prints out the usage instructions, detailing the options
 * available for running the program and what each option does.
 *
 * @param program_name A string containing the name of the program, typically
 * provided by argv[0].
 */
void print_usage(const char *program_name);

/**
 * @brief Parses command line options passed to the program.
 *
 * This function processes arguments passed to the program and sets the
 * corresponding options in the CommandLineOptions structure. The function
 * handles three options:
 * -h or --help: Displays the usage information and exits the program.
 * -d or --debug: Enables debug mode.
 * -f filename or --file=filename: Specifies the input file for the program.
 *
 * If an unknown option or incorrect usage is detected, the function will
 * print an error message, display usage information, and exit the program.
 *
 * @param argc The count of command line arguments including the program name.
 * @param argv The array of command line arguments where argv[0] is the
 *             program name.
 * @return CommandLineOptions structure containing parsed options.
 */
CommandLineOptions parse_command_line_options(int argc, char *argv[]);

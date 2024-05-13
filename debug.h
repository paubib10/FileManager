//
// Created by Brouse on 09/04/2024.
//

#ifndef FILEMANAGER_DEBUG_H
#define FILEMANAGER_DEBUG_H

#include <stdio.h>

// Colors
#define BLACK   "\x1B[30m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1B[37m"
#define ORANGE  "\x1B[38;2;255;128;0m"
#define ROSE    "\x1B[38;2;255;151;203m"
#define LBLUE   "\x1B[38;2;53;149;240m"
#define LGREEN  "\x1B[38;2;17;245;120m"
#define GRAY    "\x1B[38;2;176;174;174m"
#define RESET   "\x1b[0m"

#define NEGRITA "\x1b[1m"

// Debugs levels
#define DEBUG7 0
#define DEBUG9 1

// Debug methods
#define DEBUG(method, msg, ...) ( \
    fprintf(stderr, GRAY method"()-> "msg RESET, ## __VA_ARGS__) \
);

#define ERR(method, msg, ...) ( \
        printf(RED method"()-> "msg RESET, ## __VA_ARGS__) \
);

#define PERR(method) ( \
    perror(RED method "()" RESET) \
);

#endif //FILEMANAGER_DEBUG_H
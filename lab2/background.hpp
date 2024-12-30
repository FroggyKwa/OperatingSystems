#pragma once

int start_background(const char *program_path);

int start_background(const char *program_path, int &status);

int wait_program(const int pid, int* exit_code = nullptr);
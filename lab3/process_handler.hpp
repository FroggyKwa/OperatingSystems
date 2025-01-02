#ifndef PROCESS_HANDLER_HPP
#define PROCESS_HANDLER_HPP

#include <string>

void logToFile(const std::string &message);
void spawnChild(int mode);

#endif // PROCESS_HANDLER_HPP


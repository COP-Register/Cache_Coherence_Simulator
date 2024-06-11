#ifndef SELECTION_H
#define SELECTION_H

#include <string>
#include <filesystem>

std::string selection_data();
bool selection_protocol();
int selection_cache_line_size();
std::string selection_output();
int check(const std::string&);

#endif
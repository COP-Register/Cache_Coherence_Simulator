#ifndef CPU_H
#define CPU_H

#include "Processor.h"
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class Processor;
class Cache_Controller;

class CPU
{
public:
    static CPU& get_instance(std::string data_input, int cache_line_size);
    CPU(const CPU&) = delete;
    CPU& operator=(const CPU&) = delete;
    [[nodiscard]] std::vector<std::shared_ptr<Processor>> get_processors() const;
    void start_simulation(Cache_Controller &controller) const;
    [[nodiscard]] int get_cache_line_size() const;
private:
    static CPU* instance;
    CPU(std::string&& data_input, int cache_line_size);
    std::string data_input;
    int cache_line_size;
    std::vector<std::shared_ptr<Processor>> processors;
};

#endif //CPU_H

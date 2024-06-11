#include "CPU.h"
#include <iostream>
#include <fstream>
#include <mutex>

namespace fs = std::filesystem;

CPU* CPU::instance = nullptr;

CPU::CPU(std::string&& data_input, const int cache_line_size)
    : data_input(std::move(data_input)), cache_line_size(cache_line_size)
{
    processors.emplace_back(Processor::create(cache_line_size,"P0"));
    processors.emplace_back(Processor::create(cache_line_size,"P1"));
    processors.emplace_back(Processor::create(cache_line_size,"P2"));
    processors.emplace_back(Processor::create(cache_line_size,"P3"));
}

CPU& CPU::get_instance(std::string data_input, const int cache_line_size)
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    if (!instance)
    {
        instance = new CPU(std::move(data_input), cache_line_size);
    }
    return *instance;
}

std::vector<std::shared_ptr<Processor>> CPU::get_processors() const
{
    return processors;
}

int CPU::get_cache_line_size() const
{
    return cache_line_size;
}


void CPU::start_simulation(Cache_Controller &controller) const
{
    std::ifstream file(data_input);
    if (!file.is_open())
    {
        std::cerr << "Datei konnte nicht geöffnet werden: " << data_input << std::endl;
        return;
    }
    std::string line;
    while (getline(file, line))
    {
        if(line.substr(0,2) == "P0")
        {
            processors[0]->commit_command_processor(line, controller);
        }
        else if(line.substr(0,2) == "P1")
        {
            processors[1]->commit_command_processor(line, controller);
        }
        else if(line.substr(0,2) == "P2")
        {
            processors[2]->commit_command_processor(line, controller);
        }
        else if(line.substr(0,2) == "P3")
        {
            processors[3]->commit_command_processor(line, controller);
        }
        else
        {
            controller.commit_command_cache_controller(line);
        }
    }
    file.close();
    controller.print_results();
}


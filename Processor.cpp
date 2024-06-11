#include "Processor.h"

Processor::Processor(const int cache_line_size, std::string new_name_processor)
{
    cache = Cache::create(cache_line_size);
    name_processor = std::move(new_name_processor);
}

std::shared_ptr<Processor> Processor::create(const int cache_line_size, std::string new_name_processor)
{
    return std::shared_ptr<Processor>(new Processor(cache_line_size, std::move(new_name_processor)));
}

void Processor::commit_command_processor(const std::string &line, Cache_Controller &controller)
{
    controller.commit_command_cache_controller(line, this);
}

std::shared_ptr<Cache> Processor::get_cache() const
{
    return cache;
}

std::string Processor::get_name_processor() const
{
    return name_processor;
}

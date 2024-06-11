#include "Cache_Line.h"

Cache_Line::Cache_Line(const int cache_line_size, const int new_index)
    : index(new_index)
{
    for(int i = 0; i<(2048/cache_line_size); i++)
    {
        words.emplace_back(Word::create(i));
    }
}

std::shared_ptr<Cache_Line> Cache_Line::create(const int cache_line_size, const int new_index)
{
    return std::shared_ptr<Cache_Line>(new Cache_Line(cache_line_size, new_index));
}

Cache_Line::Status Cache_Line::get_status() const
{
    return status;
}

int Cache_Line::get_index() const
{
    return index;
}

int Cache_Line::get_tag() const
{
    return tag;
}

void Cache_Line::set_status(const Status new_status)
{
    status = new_status;
}

void Cache_Line::set_index(const int new_index)
{
    index = new_index;
}

void Cache_Line::set_tag(const int new_tag)
{
    tag = new_tag;
}

std::string Cache_Line::status_to_string(const Status status)
{
    switch (status)
    {
        case Invalid:
            return "Invalid";
        case Exclusive:
            return "Exclusive";
        case Shared:
            return "Shared";
        case Modified:
            return "Modified";
        default:
            return "Unknown";
    }
}
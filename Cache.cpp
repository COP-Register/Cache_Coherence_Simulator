#include "Cache.h"

Cache::Cache(const int cache_line_size)
{
    for(int i = 0; i<cache_line_size; i++)
    {
        const int new_index = (i/(2048/cache_line_size)) % cache_line_size;
        cache_lines.emplace_back(Cache_Line::create(cache_line_size, new_index));
    }
}

std::shared_ptr<Cache> Cache::create(const int cache_line_size)
{
    return std::shared_ptr<Cache>(new Cache(cache_line_size));
}

std::vector<std::shared_ptr<Cache_Line> > Cache::get_cache_line() const
{
    return cache_lines;
}

#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <memory>
#include "Cache_Line.h"

class Cache
{
public:
    static std::shared_ptr<Cache> create (int cache_line_size);
    [[nodiscard]] std::vector<std::shared_ptr<Cache_Line>> get_cache_line() const;
private:
    std::vector<std::shared_ptr<Cache_Line>> cache_lines;
    explicit Cache(int cache_line_size);
};

#endif //CACHE_H

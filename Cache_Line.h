#ifndef CACHE_LINE_H
#define CACHE_LINE_H

#include <vector>
#include <memory>
#include <string>
#include "Word.h"

class Cache_Line
{
public:
    static std::shared_ptr<Cache_Line> create (int cache_line_size, int new_index);
    enum Status
    {
        Invalid,
        Exclusive,
        Shared,
        Modified
    };
    [[nodiscard]]Status get_status() const;
    [[nodiscard]] int get_index() const;
    [[nodiscard]] int get_tag() const;
    void set_status(Status new_status);
    void set_index(int new_index);
    void set_tag(int new_tag);
    static std::string status_to_string(Status status);
private:
    std::vector <std::shared_ptr<Word>> words ;
    int tag {-1};
    int index;
    Status status {Invalid};
    explicit Cache_Line(int cache_line_size, int new_index);
};

#endif //CACHE_LINE_H

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <memory>
#include <string>
#include"Cache.h"
#include "Cache_Controller.h"

class Cache_Controller;

class Processor
{
public:
   static std::shared_ptr<Processor> create (int cache_line_size, std::string new_name_processor);
   void commit_command_processor(const std::string &line, Cache_Controller &controller);
   [[nodiscard]] std::shared_ptr<Cache> get_cache() const;
   [[nodiscard]] std::string get_name_processor() const;
private:
   std::shared_ptr<Cache> cache;
   explicit Processor(int cache_line_size, std::string new_name_processor);
   std::string name_processor;
};

#endif //PROCESSOR_H

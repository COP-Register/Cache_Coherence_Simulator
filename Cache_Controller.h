#ifndef CACHE_CONTROLLER_H
#define CACHE_CONTROLLER_H

#include <string>
#include "Cache_Line.h"
#include "Processor.h"
#include "CPU.h"
#include <filesystem>

class CPU;
class Processor;

namespace fs = std::filesystem;

class Cache_Controller
{
public:
    static Cache_Controller& get_instance(bool protocol, const std::string &output, CPU& cpu);
    Cache_Controller(const Cache_Controller&) = delete;
    Cache_Controller& operator=(const Cache_Controller&) = delete;
    void commit_command_cache_controller(const std::string &line);
    void commit_command_cache_controller(const std::string &line, Processor *processor);
    void set_data_output(const fs::path& file_path);
    [[nodiscard]] std::string get_data_output() const;
    [[nodiscard]] bool get_write_mode() const;
    void swap_write_mode();
    void write_all_caches() const;
    void write_all_invalid_caches() const;
    void write_hit_rate() const;
    void write_cache(const std::shared_ptr<Processor> &processor,int address);
    void read_cache(const std::shared_ptr<Processor> &processor,int address);
    void log_event(const std::string& event, const std::shared_ptr<Processor>& processor, int cache_line_position, int tag, Cache_Line::Status status, const std::string& reason = "None") const;
    [[nodiscard]] bool check_other_caches_read(const std::shared_ptr<Processor>& processor, int cache_line_position) const;
    void check_other_caches_write(const std::shared_ptr<Processor>& processor, int cache_line_position);
    void increase_hit_counter();
    void increase_miss_counter();
    void increase_broadcast_counter();
    void increase_lines_counter();
    void increase_private_counter();
    void increase_public_counter();
    void add_to_invalid_list(const std::stringstream &new_string);
    void print_results() const;
private:
    static Cache_Controller* instance;
    Cache_Controller(bool new_protocol, const std::string &output, CPU& cpu);
    bool protocol; // false == MSI; true == MESI
    bool write_mode{false};
    std::string data_output;
    CPU& cpu;
    double hit_rate{0};
    double miss_rate{0};
    std::vector <std::string> invalid_list;
    int invalidation_broadcast_count{0};
    int invalidated_lines_count{0};
    int private_access_count{0};
    int public_access_count{0};
};

#endif //CACHE_CONTROLLER_H

#include "Cache_Controller.h"
#include <iostream>
#include <stdexcept>
#include <mutex>
#include <fstream>

Cache_Controller* Cache_Controller::instance = nullptr;

Cache_Controller::Cache_Controller(const bool new_protocol, const std::string &output, CPU& cpu)
    : protocol(new_protocol), cpu(cpu)
{
    try
    {
        const fs::path output_dir = "Output";
        fs::create_directories(output_dir);

        const fs::path file_path = output_dir / (output + ".txt");
        std::ofstream file(file_path);

        if (!file)
        {
            throw std::runtime_error("Datei konnte nicht zum Schreiben geöffnet werden: " + file_path.string());
        }

        this->set_data_output(file_path);
        file.close();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fehler im Konstruktor von Protocoll: " << e.what() << std::endl;
    }
}

Cache_Controller& Cache_Controller::get_instance(const bool new_protocol, const std::string &output, CPU& cpu)
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    if (!instance)
    {
        instance = new Cache_Controller(new_protocol, output, cpu);
    }
    return *instance;
}

void Cache_Controller::set_data_output(const fs::path& file_path)
{
    data_output = file_path.string();
}

std::string Cache_Controller::get_data_output() const
{
    return data_output;
}

void Cache_Controller::increase_hit_counter()
{
    hit_rate++;
}

void Cache_Controller::increase_miss_counter()
{
    miss_rate++;
}

void Cache_Controller::increase_broadcast_counter()
{
    invalidation_broadcast_count++;
}

void Cache_Controller::increase_lines_counter()
{
    invalidated_lines_count++;
}

void Cache_Controller::increase_private_counter()
{
    private_access_count++;
}

void Cache_Controller::increase_public_counter()
{
    public_access_count++;
}


void Cache_Controller::commit_command_cache_controller(const std::string &line)
{
    std::istringstream stream(line);
    std::string command;

    while (stream >> command)
    {
        if (command.empty())
        {
            continue;
        }
        if (command == "v" ||  command == "V")
        {
            this->swap_write_mode();
        }
        else if (command == "c" ||  command == "C")
        {
            write_all_caches();
        }
        else if (command == "h" ||  command == "H")
        {
            write_hit_rate();
        }
        else if (command == "i" ||  command == "I")
        {
            write_all_invalid_caches();
        }
        else
        {
            std::cerr << "Unknown command: " << command << std::endl;
        }
    }
}

void Cache_Controller::commit_command_cache_controller(const std::string& line, Processor *processor)
{
    std::istringstream iss(line);
    std::string processor_name;
    char modus;
    int address;

    if (!(iss >> processor_name >> modus >> address))
    {
        return;
    }

    std::string extra;
    if (iss >> extra)
    {
        return;
    }

    std::ofstream file(get_data_output(), std::ios::app);
    if (!file.is_open())
    {
        std::cerr << "Fehler beim Öffnen der Logdatei!" << std::endl;
        return;
    }

    if(modus=='W' || modus=='w')
    {
        if (processor_name == "P0")
        {
            write_cache(cpu.get_processors()[0], address);
        }
        else if (processor_name == "P1")
        {
            write_cache(cpu.get_processors()[1], address);
        }
        else if (processor_name == "P2")
        {
            write_cache(cpu.get_processors()[2], address);
        }
        else if (processor_name == "P3")
        {
            write_cache(cpu.get_processors()[3], address);
        }
    }

    else if (modus=='R' || modus=='r')
    {
        if (processor_name == "P0")
        {
            read_cache(cpu.get_processors()[0], address);
        }
        else if (processor_name == "P1")
        {
            read_cache(cpu.get_processors()[1], address);
        }
        else if (processor_name == "P2")
        {
            read_cache(cpu.get_processors()[2], address);
        }
        else if (processor_name == "P3")
        {
            read_cache(cpu.get_processors()[3], address);
        }
    }
    else
    {
        return;
    }
    file.close();
}

bool Cache_Controller::get_write_mode() const
{
    return write_mode;
}

void Cache_Controller::swap_write_mode()
{
    write_mode = !write_mode;
}

void Cache_Controller::write_all_caches() const
{
    const fs::path output_dir {get_data_output()};
    std::ofstream file(output_dir, std::ios::app);
    if (!file.is_open())
    {
        std::cerr << "Datei konnte nicht geöffnet werden: " << output_dir << std::endl;
        return;
    }

    const std::vector<std::shared_ptr<Processor>> processors  = cpu.get_processors();
    for (const auto& processor : processors)
    {
        for(int i{0}; i < cpu.get_cache_line_size(); i++)
        {
            const std::shared_ptr<Cache> cache = processor->get_cache();
            std::vector<std::shared_ptr<Cache_Line>> cache_line = cache->get_cache_line();
            const Cache_Line::Status status = cache_line[i]->get_status();
            const int tag = cache_line[i]->get_tag();

            file << "Processor: " << processor->get_name_processor() << " Cache-Line-Number: " << i << " Tag: " << tag << " Status: " << cache_line[i]->status_to_string(status) << std::endl;
        }
    }
    file.close();
}

void Cache_Controller::write_all_invalid_caches() const
{
    const fs::path output_dir {get_data_output()};
    std::ofstream file(output_dir, std::ios::app);
    if (!file.is_open())
    {
        std::cerr << "Datei konnte nicht geöffnet werden: " << output_dir << std::endl;
        return;
    }

    for (const auto& line : invalid_list)
    {
        file << line << std::endl;
    }

    file.close();
}

void Cache_Controller::write_hit_rate() const
{
    const fs::path output_dir {get_data_output()};
    std::ofstream file(output_dir, std::ios::app);
    if (!file.is_open())
    {
        std::cerr << "Datei konnte nicht geöffnet werden: " << output_dir << std::endl;
        return;
    }

    file << "Hit_Rate: " << (hit_rate/(hit_rate+miss_rate))*100 << "%" << std::endl;

    file.close();
}

void Cache_Controller::read_cache(const std::shared_ptr<Processor> &processor, const int address)
{
    const int cache_line_position = (address/4) % cpu.get_cache_line_size();
    const int new_tag = address/2048;

    const std::shared_ptr<Cache > cache = processor->get_cache();
    const std::vector<std::shared_ptr<Cache_Line>> cache_line = cache->get_cache_line();
    const Cache_Line::Status status = cache_line[cache_line_position]->get_status();
    const int tag = cache_line[cache_line_position]->get_tag();

    if(protocol==false) //MSI
    {
        if(tag == new_tag)
        {
            if(status == Cache_Line::Status::Modified || status == Cache_Line::Status::Shared)
            {
                increase_hit_counter();
                if(write_mode == true)
                {
                    log_event("Cache Hit - Read", processor, cache_line_position, tag, status);
                }
                if(status == Cache_Line::Status::Modified)
                {
                    increase_private_counter();
                }
                else
                {
                    increase_public_counter();
                }
                return;
            }
            else if (status == Cache_Line::Status::Invalid)
            {
                increase_miss_counter();
                if (write_mode == true)
                {
                    log_event("Cache Miss - Read", processor, cache_line_position, tag, status, "Invalid");
                }
                std::stringstream invalidation_message;
                invalidation_message << "A read operation by processor " << processor->get_name_processor() << " for tag " << tag << " in cache line " << cache_line_position << "was found in this cache in the Invalid state (cache miss)" << std::endl;
                add_to_invalid_list(invalidation_message);
                [[maybe_unused]] bool change =check_other_caches_read(processor, cache_line_position);
                cache_line[cache_line_position]->set_status(Cache_Line::Status::Shared);
                cache_line[cache_line_position]->set_tag(new_tag);
                if (write_mode == true)
                {
                    log_event("New Values", processor, cache_line_position, cache_line[cache_line_position]->get_tag(), cache_line[cache_line_position]->get_status(), "Cache Miss");
                }
                return;
            }
        }
        else
        {
            increase_miss_counter();
            if (write_mode == true)
            {
                log_event("Cache Miss - Read", processor, cache_line_position, tag, status, "Tag Mismatch");
            }
            std::stringstream invalidation_message;
            invalidation_message << "A read operation by processor " << processor->get_name_processor() << " for tag " << tag << " in cache line " << cache_line_position << "was not found in this cache (cache miss)" << std::endl;
            add_to_invalid_list(invalidation_message);
            [[maybe_unused]] bool change = check_other_caches_read(processor, cache_line_position);
            cache_line[cache_line_position]->set_status(Cache_Line::Status::Shared);
            cache_line[cache_line_position]->set_tag(new_tag);
            if (write_mode == true)
            {
                log_event("New Values", processor, cache_line_position, cache_line[cache_line_position]->get_tag(), cache_line[cache_line_position]->get_status(), "Cache Miss");
            }
            return;
        }
    }

    if (protocol == true) // MESI
    {
        if (tag == new_tag)
        {
            if (status == Cache_Line::Status::Modified || status == Cache_Line::Status::Exclusive || status == Cache_Line::Status::Shared)
            {
                increase_hit_counter();
                if (write_mode == true)
                {
                    log_event("Cache Hit - Read", processor, cache_line_position, tag, status);
                }
                if(status == Cache_Line::Status::Modified || status == Cache_Line::Status::Exclusive)
                {
                    increase_private_counter();
                }
                else
                {
                    increase_public_counter();
                }
                return;
            }
            else if (status == Cache_Line::Status::Invalid)
            {
                increase_miss_counter();
                if (write_mode == true)
                {
                    log_event("Cache Miss - Read", processor, cache_line_position, tag, status, "Invalid");
                }
                if(check_other_caches_read(processor, cache_line_position) == true)
                {
                    cache_line[cache_line_position]->set_status(Cache_Line::Status::Shared);
                }
                else
                {
                    cache_line[cache_line_position]->set_status(Cache_Line::Status::Exclusive);
                }
                std::stringstream invalidation_message;
                invalidation_message << "A read operation by processor " << processor->get_name_processor() << " for tag " << tag << " in cache line " << cache_line_position << "was found in this cache in the Invalid state (cache miss)" << std::endl;
                add_to_invalid_list(invalidation_message);
                cache_line[cache_line_position]->set_tag(new_tag);
                if (write_mode == true)
                {
                    log_event("New Values", processor, cache_line_position, cache_line[cache_line_position]->get_tag(), cache_line[cache_line_position]->get_status(), "Cache Miss");
                }
                return;
            }
        }
        else
        {
            increase_miss_counter();
            if (write_mode == true)
            {
                log_event("Cache Miss - Read", processor, cache_line_position, tag, status, "Tag Mismatch");
            }
            if(check_other_caches_read(processor, cache_line_position) == true)
            {
                cache_line[cache_line_position]->set_status(Cache_Line::Status::Shared);
            }
            else
            {
                cache_line[cache_line_position]->set_status(Cache_Line::Status::Exclusive);
            }
            std::stringstream invalidation_message;
            invalidation_message << "A read operation by processor " << processor->get_name_processor() << " for tag " << tag << " in cache line " << cache_line_position << "was not found in this cache (cache miss)" << std::endl;
            add_to_invalid_list(invalidation_message);
            cache_line[cache_line_position]->set_tag(new_tag);
            if (write_mode == true)
            {
                log_event("New Values", processor, cache_line_position, cache_line[cache_line_position]->get_tag(), cache_line[cache_line_position]->get_status(), "Cache Miss");
            }
            return;
        }
    }
}

void Cache_Controller::write_cache(const std::shared_ptr<Processor> &processor, int address)
{
    const int cache_line_position = (address/4) % cpu.get_cache_line_size();
    const int new_tag = address/2048;

    const std::shared_ptr<Cache > cache = processor->get_cache();
    const std::vector<std::shared_ptr<Cache_Line>> cache_line = cache->get_cache_line();
    const Cache_Line::Status status = cache_line[cache_line_position]->get_status();
    const int tag = cache_line[cache_line_position]->get_tag();

    if(protocol==false) //MSI
    {
        if(tag == new_tag)
        {
            if(status == Cache_Line::Status::Modified)
            {
                increase_hit_counter();
                if(write_mode == true)
                {
                    log_event("Cache Hit - Write", processor, cache_line_position, tag, status);
                }
                return;
            }
            else if (status == Cache_Line::Status::Shared || status == Cache_Line::Status::Invalid)
            {
                increase_miss_counter();
                if (write_mode == true)
                {
                    log_event("Cache Miss - Write", processor, cache_line_position, tag, status, "Invalid");
                }
                std::stringstream invalidation_message;
                invalidation_message << "A write operation by processor " << processor->get_name_processor() << " for tag " << tag << " in cache line " << cache_line_position << "was found in this cache in the Invalid state (cache miss)" << std::endl;
                add_to_invalid_list(invalidation_message);
                check_other_caches_write(processor, cache_line_position);
                cache_line[cache_line_position]->set_status(Cache_Line::Status::Modified);
                cache_line[cache_line_position]->set_tag(new_tag);
                if (write_mode == true)
                {
                    log_event("New Values", processor, cache_line_position, cache_line[cache_line_position]->get_tag(), cache_line[cache_line_position]->get_status(), "Cache Miss - Write");
                }
                return;
            }
        }
        else
        {
            increase_miss_counter();
            if (write_mode == true)
            {
                log_event("Cache Miss - Write", processor, cache_line_position, tag, status, "Tag Mismatch");
            }
            std::stringstream invalidation_message;
            invalidation_message << "A write operation by processor " << processor->get_name_processor() << " for tag " << tag << " in cache line " << cache_line_position << "was not found in this cache (cache miss)" << std::endl;
            add_to_invalid_list(invalidation_message);
            check_other_caches_write(processor, cache_line_position);
            cache_line[cache_line_position]->set_status(Cache_Line::Status::Modified);
            cache_line[cache_line_position]->set_tag(new_tag);
            if (write_mode == true)
            {
                log_event("New Values", processor, cache_line_position, cache_line[cache_line_position]->get_tag(), cache_line[cache_line_position]->get_status(), "Cache Miss - Write");
            }
            return;
        }
    }

    if (protocol == true) // MESI
    {
        if (tag == new_tag)
        {
            if (status == Cache_Line::Status::Modified)
            {
                increase_hit_counter();
                if (write_mode == true)
                {
                    log_event("Cache Hit - Write", processor, cache_line_position, tag, status);
                }
                return;
            }
            else if(status == Cache_Line::Status::Exclusive)
            {
                increase_hit_counter();
                if (write_mode == true)
                {
                    log_event("Cache Hit - Write", processor, cache_line_position, tag, status);
                }
                cache_line[cache_line_position]->set_status(Cache_Line::Status::Modified);
                if (write_mode == true)
                {
                    log_event("New Values", processor, cache_line_position, cache_line[cache_line_position]->get_tag(), cache_line[cache_line_position]->get_status(), "Change Exclusiv to Modified");
                }
                return;
            }
            else if (status == Cache_Line::Status::Invalid || status == Cache_Line::Status::Shared)
            {
                increase_miss_counter();
                if (write_mode == true)
                {
                    log_event("Cache Miss - Write", processor, cache_line_position, tag, status, "Wrong Status");
                }
                std::stringstream invalidation_message;
                invalidation_message << "A write operation by processor " << processor->get_name_processor() << " for tag " << tag << " in cache line " << cache_line_position << "was found in this cache in the Invalid state (cache miss)" << std::endl;
                add_to_invalid_list(invalidation_message);
                check_other_caches_write(processor, cache_line_position);
                cache_line[cache_line_position]->set_status(Cache_Line::Status::Modified);
                cache_line[cache_line_position]->set_tag(new_tag);
                if (write_mode == true)
                {
                    log_event("New Values", processor, cache_line_position, cache_line[cache_line_position]->get_tag(), cache_line[cache_line_position]->get_status(), "Cache Miss");
                }
                return;
            }
        }
        else
        {
            increase_miss_counter();
            if (write_mode == true)
            {
                log_event("Cache Miss - Write", processor, cache_line_position, tag, status, "Tag Mismatch");
            }
            std::stringstream invalidation_message;
            invalidation_message << "A write operation by processor " << processor->get_name_processor() << " for tag " << tag << " in cache line " << cache_line_position << "was not found in this cache (cache miss)" << std::endl;
            add_to_invalid_list(invalidation_message);
            check_other_caches_write(processor, cache_line_position);
            cache_line[cache_line_position]->set_status(Cache_Line::Status::Modified);
            cache_line[cache_line_position]->set_tag(new_tag);
            if (write_mode == true)
            {
                log_event("New Values", processor, cache_line_position, cache_line[cache_line_position]->get_tag(), cache_line[cache_line_position]->get_status(), "Cache Miss");
            }
            return;
        }
    }
}

void Cache_Controller::log_event(const std::string& event, const std::shared_ptr<Processor>& processor, const int cache_line_position, const int tag, const Cache_Line::Status status, const std::string& reason) const
{
    if (std::ofstream file(get_data_output(), std::ios::app); file.is_open())
    {
        file << event << ": Processor = " << processor->get_name_processor()
             << ", Cache-Line Number = " << cache_line_position
             << ", Tag = " << tag
             << ", Status = " << Cache_Line::status_to_string(status);
        if(reason != "None")
        {
            file << ", Reason = " << reason;
        }
        file << std::endl;
        file.close();
    }
    else
    {
        std::cerr << "Error opening the log file!" << std::endl;
    }
}

bool Cache_Controller::check_other_caches_read(const std::shared_ptr<Processor>& processor, const int cache_line_position) const
{
    const auto processors = cpu.get_processors();
    bool changed = false;

    for(const auto& other_processor : processors)
    {
        if (other_processor == processor)
        {
            continue;
        }
        const auto cache = other_processor->get_cache();
        const std::vector<std::shared_ptr<Cache_Line>> cache_line = cache->get_cache_line();
        const Cache_Line::Status status = cache_line[cache_line_position]->get_status();
        const int tag = cache_line[cache_line_position]->get_tag();

        if(status == Cache_Line::Status::Modified)
        {
            if (write_mode == true)
            {
                log_event("Modified Status in other Cache", other_processor, cache_line_position, tag, status);
            }
            cache_line[cache_line_position]->set_status(Cache_Line::Shared);
            if (write_mode == true)
            {
                log_event("Set Status to Shared", other_processor, cache_line_position, tag, cache_line[cache_line_position]->get_status());
            }
            changed = true;
        }
        if(status == Cache_Line::Status::Exclusive)
        {
            if (write_mode == true)
            {
                log_event("Exclusive Status in other Cache", other_processor, cache_line_position, tag, status);
            }
            cache_line[cache_line_position]->set_status(Cache_Line::Shared);
            if (write_mode == true)
            {
                log_event("Set Status to Shared", other_processor, cache_line_position, tag, cache_line[cache_line_position]->get_status());
            }
            changed = true;
        }
        if(status == Cache_Line::Status::Shared)
        {
            changed = true;
        }
    }
    return changed;
}

void Cache_Controller::check_other_caches_write(const std::shared_ptr<Processor> &processor, const int cache_line_position)
{
    const auto processors = cpu.get_processors();
    bool broadcast {false};

    for(const auto& other_processor : processors)
    {
        if (other_processor == processor)
        {
            continue;
        }
        const auto cache = other_processor->get_cache();
        const std::vector<std::shared_ptr<Cache_Line>> cache_line = cache->get_cache_line();
        const Cache_Line::Status status = cache_line[cache_line_position]->get_status();
        const int tag = cache_line[cache_line_position]->get_tag();

        if(status == Cache_Line::Status::Modified)
        {
            if (write_mode == true)
            {
                log_event("Modified Status in other Cache", other_processor, cache_line_position, tag, status, "Write Modified Data to RAM");
            }
            cache_line[cache_line_position]->set_status(Cache_Line::Invalid);
            if (write_mode == true)
            {
                log_event("Set Status to Invalid", other_processor, cache_line_position, tag, cache_line[cache_line_position]->get_status());
            }
            increase_lines_counter();
            broadcast = true;
        }
        if(status == Cache_Line::Status::Exclusive)
        {
            if (write_mode == true)
            {
                log_event("Exclusive Status in other Cache", other_processor, cache_line_position, tag, status);
            }
            cache_line[cache_line_position]->set_status(Cache_Line::Invalid);
            if (write_mode == true)
            {
                log_event("Set Status to Invalid", other_processor, cache_line_position, tag, cache_line[cache_line_position]->get_status());
            }
            increase_lines_counter();
            broadcast = true;
        }
        if(status == Cache_Line::Status::Shared)
        {
            if (write_mode == true)
            {
                log_event("Shared Status in other Cache", other_processor, cache_line_position, tag, status);
            }
            cache_line[cache_line_position]->set_status(Cache_Line::Invalid);
            if (write_mode == true)
            {
                log_event("Set Status to Invalid", other_processor, cache_line_position, tag, cache_line[cache_line_position]->get_status());
            }
            increase_lines_counter();
            broadcast = true;
        }
    }
    if(broadcast == true)
    {
        increase_broadcast_counter();
    }
}

void Cache_Controller::add_to_invalid_list(const std::stringstream &new_string)
{
    invalid_list.emplace_back(new_string.str());
}

void Cache_Controller::print_results() const
{
    const fs::path output_dir {get_data_output()};
    std::ofstream file(output_dir, std::ios::app);
    if (!file.is_open())
    {
        std::cerr << "Datei konnte nicht geöffnet werden: " << output_dir << std::endl;
        return;
    }
    file << "End results:" << std::endl
         << "Hit-Count: " << hit_rate << std::endl
         << "Miss-Count: " << miss_rate << std::endl
         << "Hit-Rate: " << (hit_rate/(hit_rate+miss_rate))*100 << "%" << std::endl
         << "Calls of invalidation broadcast: " << invalidation_broadcast_count << std::endl
         << "Invalidations: " << invalidated_lines_count << std::endl
         << "Accesses to private data: " << private_access_count << std::endl
         << "Accesses to public data: " << public_access_count << std::endl;
    file.close();
}

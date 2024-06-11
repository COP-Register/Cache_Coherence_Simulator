#include "Selection.h"
#include "CPU.h"

int main()
{
    const std::string data_input{selection_data()};
    const bool new_protocol{selection_protocol()};
    const int cache_line_size{selection_cache_line_size()};
    const std::string output{selection_output()};
    CPU &test = CPU::get_instance(data_input, cache_line_size);
    Cache_Controller &controller = Cache_Controller::get_instance(new_protocol, output, test);
    test.start_simulation(controller);
}

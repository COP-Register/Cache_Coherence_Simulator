#include "selection.h"
#include <vector>
#include <iostream>

namespace fs = std::filesystem;

std::string selection_data()
{
    std::string path = "Trace-Data";
    std::vector<std::string> filenames;

    try
    {
        if (!fs::is_directory(path))
        {
            std::cerr << "The specified path is not a directory!" << std::endl;
        }

        int index = 1;
        for (const auto& entry : fs::directory_iterator(path))
        {
            if (entry.is_regular_file())
            {
                filenames.push_back(entry.path().filename().string());
                std::cout << index++ << ": " << filenames.back() << std::endl;
            }
        }
    }
    catch (const fs::filesystem_error& e)
    {
        std::cerr <<  "Error accessing the file system: " << e.what() << std::endl;
    }

    while (true)
    {
        int choice {check("Select a file: ")};

        if (choice < 1 || static_cast<size_t>(choice) > filenames.size())
        {
            std::cerr << "Invalid selection." << std::endl;
            continue;
        }

        fs::path filepath = fs::path(path) / filenames[choice - 1];

        std::string file = filepath.string();
        return file;
    }
}

bool selection_protocol()
{
    std::cout << "1. MSI\n" << "2. MESI" << std::endl;

    while (true)
    {
        int choice {check("Select a cache coherence protocol: ")};

        if (choice!=1 && choice!=2)
        {
            std::cerr << "Invalid selection." << std::endl;
            continue;
        }
        return (choice == 2);
    }
}

int selection_cache_line_size()
{
    std::cout << "1. 2\n" << "2. 4\n" << "3. 8\n" << "4. 16" << std::endl;

    while (true)
    {
        int choice {check("Select a size for the cache line: ")};

        if (choice!=1 && choice!=2 && choice!=3 && choice!=4)
        {
            std::cerr << "Invalid selection." << std::endl;
            continue;
        }

        switch (choice)
        {
            case 1:
                return 1024;
            case 2:
                return 512;
            case 3:
                return 256;
            default:
                return 128;
        }
    }
}

std::string selection_output()
{
    std::string name{};
    std::cout <<  "Name of the new file where the output should be saved?" << std::endl;

    while (true)
    {
        std::cin >> name;
        fs::path output_dir{"Output"};

        if (!fs::exists(output_dir))
        {
            fs::create_directories(output_dir);
        }

        fs::path file_path = output_dir / (name + ".txt");

        if (fs::exists(file_path))
        {
            std::cout <<  "A file with this name already exists. Please choose a different name." << std::endl;
        }
        else
        {
            break;
        }
    }
    return name;
}

int check(const std::string& question)
{
    while (true)
    {
        std::cout << question << std::endl;
        int answer;
        if (std::cin >> answer)
        {
            return answer;
        }
        else
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "Invalid selection." << std::endl;
        }
    }
}
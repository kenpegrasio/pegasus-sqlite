#include <cstring>
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cerr << "Logs from your program will appear here" << std::endl;

    if (argc != 3) {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }

    std::string database_file_path = argv[1];
    std::string command = argv[2];

    if (command == ".dbinfo") {
        std::ifstream database_file(database_file_path, std::ios::binary);
        if (!database_file) {
            std::cerr << "Failed to open the database file" << std::endl;
            return 1;
        }

        database_file.seekg(16); // moves the pointer to byte 16
        
        char buffer[2];
        database_file.read(buffer, 2); // get byte 17 and 18
        
        unsigned short page_size = (static_cast<unsigned char>(buffer[1]) | (static_cast<unsigned char>(buffer[0]) << 8));

        database_file.seekg(103); // moves the pointer to byte 103
        database_file.read(buffer, 2); // get byte 104 and 105
        unsigned short table_numbers = (static_cast<unsigned char>(buffer[1]) | static_cast<unsigned char>(buffer[0]) << 8);
        
        std::cout << "database page size: " << page_size << std::endl;
        std::cout << "number of tables: " << table_numbers << std::endl;
    }

    return 0;
}

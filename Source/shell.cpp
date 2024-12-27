#include "shell.h"
#include "vero.h"

int main() {
    initialize_global_symbol_table();
    std::string statement;
    while (true) {
        std::cout << ">>> ";
        std::getline(std::cin, statement);
        if (statement=="exit") {
            break;
        }
        if (statement.empty()) {
            continue;
        }
        std::pair result = run("<stdin>", statement);
        Value* value = result.first;
        Error* error = result.second;

        if (error->err_name != "" && error->err_name != "EOF") {
            std::cout << error->as_string() << std::endl;
        } else {
            std::string out_value = value->repr();
            // Split output by newlines
            size_t pos = 0;
            size_t last_pos = 0;
            // Only print lines before the last one (which contains the return value)
            while ((pos = out_value.find('\n', last_pos)) != std::string::npos) {
                std::string line = out_value.substr(last_pos, pos - last_pos);
                std::cout << line << std::endl;
                last_pos = pos + 1;
            }
        } 
    }
    return 0;
}
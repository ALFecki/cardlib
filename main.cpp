#include <limits>

#include "bpace.h"

int main() {
    Bpace bpace = Bpace("334780", Pwd::CAN);
    std::cout << bpace.authorize() << std::endl;
    bpace.getName();
    // CardSecure card = CardSecure();
    // card.initSecure(bpace.getKey().data());

    // while (true) {
    //     std::cout << "Enter operation code (1 - data groups, 2 - sign): ";
    //     int operation;
    //     while (true) {
    //         if (std::cin >> operation) {
    //             std::cin.clear();
    //             std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    //             break;
    //         } else {
    //             std::cout << "Please enter a valid integer" << std::endl;
    //             std::cin.clear();
    //             std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    //         }
    //     }

    //     switch (operation) {
    //         case 1: {
    //             std::string password;
    //             std::cout << "Enter CAN code: ";

    //             std::getline(std::cin, password);
    //             if (password.size() < 6) {
    //                 continue;
    //             }
    //             Bpace bpace = Bpace(password, Pwd::CAN);
    //             bpace.authorize();
    //             CardSecure card = CardSecure();
    //             card.initSecure(bpace.getKey().data());
    //         }
    //     }
    // }
}
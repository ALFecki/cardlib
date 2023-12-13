#include <json/json.h>
#include <table.h>

#include <boost/algorithm/string.hpp>
#include <limits>

#include "bpace.h"

int main() {
    // Bpace bpace = Bpace("334780", pwd_t::CAN);
    // std::cout << bpace.authorize() << std::endl;

    // CardSecure card = CardSecure();
    // card.initSecure(bpace.getKey().data());

    while (true) {
        std::cout << "Enter operation code (1 - data groups, 2 - identity number): ";
        int operation;
        while (true) {
            if (std::cin >> operation) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                break;
            } else {
                std::cout << "Please enter a valid integer" << std::endl;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
        }

        switch (operation) {
            case 1: {
                std::string password;
                std::cout << "Enter CAN code: ";

                std::getline(std::cin, password);
                if (password.size() < 6) {
                    continue;
                }
                Bpace bpace = Bpace(password, pwd_t::CAN);
                if (!bpace.authorize()) {
                    std::cout << "Error in authorization, try again." << std::endl;
                    continue;
                }

                std::string json_dg = bpace.getName();

                auto t = TextTable('-', '|', '+');
                t.add("Data group");
                t.add("Value");
                t.endOfRow();

                Json::Value root;
                Json::Reader reader;
                reader.parse(json_dg.c_str(), root);

                auto fio = root["be"];

                t.add("family_name_be");
                t.add(fio.get("family_name", "None").asString());
                t.endOfRow();

                t.add("given_name_be");
                t.add(fio.get("given_name", "None").asString());
                t.endOfRow();

                t.add("middle_name_be");
                t.add(fio.get("middle_name", "None").asString());
                t.endOfRow();

                fio = root["ru"];

                t.add("family_name_ru");
                t.add(fio.get("family_name", "None").asString());
                t.endOfRow();

                t.add("given_name_ru");
                t.add(fio.get("given_name", "None").asString());
                t.endOfRow();

                t.add("middle_name_ru");
                t.add(fio.get("middle_name", "None").asString());
                t.endOfRow();

                fio = root["la"];

                t.add("family_name_la");
                t.add(fio.get("family_name", "None").asString());
                t.endOfRow();

                t.add("given_name_la");
                t.add(fio.get("given_name", "None").asString());
                t.endOfRow();

                json_dg = bpace.getBirthDate();
                root.clear();
                reader.parse(json_dg.c_str(), root);
                t.add("birth_date");
                t.add(root.get("date_of_birth", "None").asString());
                t.endOfRow();

                json_dg = bpace.getSex();
                root.clear();
                reader.parse(json_dg.c_str(), root);
                t.add("sex");
                t.add(root.get("sex", "None").asString());
                t.endOfRow();

                t.setAlignment(2, TextTable::Alignment::RIGHT);
                std::cout << t;
                break;
            }
            case 2: {
                std::string password;
                std::cout << "Enter PIN1 code: ";

                std::getline(std::cin, password);
                if (password.size() < 6) {
                    continue;
                }
                Bpace bpace = Bpace(password, pwd_t::PIN);
                if (!bpace.authorize()) {
                    std::cout << "Error in authorization, try again." << std::endl;
                    continue;
                }
                

                password.clear();
                std::cout << "Enter PIN2 code: ";
                std::getline(std::cin, password);

                if (password.size() < 6) {
                    continue;
                }

                std::string json_dg = bpace.getIdentityNumber(password);

                auto t = TextTable('-', '|', '+');
                t.add("Data group");
                t.add("Value");
                t.endOfRow();

                Json::Value root;
                Json::Reader reader;
                reader.parse(json_dg.c_str(), root);
                t.add("identity_number");
                t.add(root.get("personal_number", "None").asString());
                t.endOfRow();
                t.setAlignment(2, TextTable::Alignment::RIGHT);
                std::cout << t;
                break;
            }
        }
    }
}
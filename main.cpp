#include "bpace.h"

int main() {
    Bpace bpace = Bpace("334780", Pwd::CAN);
    std::cout << bpace.authorize() << std::endl;
}
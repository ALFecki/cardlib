#include "bpace.h"

int main() {
    std::cout << sizeof(unsigned char) << std::endl;
    auto bpace = Bpace("543524");
    std::cout << bpace.authorize() << std::endl;
}
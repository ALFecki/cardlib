#include "bpace.h"

int main() {
    Bpace bpace = Bpace("334780");
    std::cout << bpace.authorize() << std::endl;
}
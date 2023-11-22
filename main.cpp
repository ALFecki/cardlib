#include "bpace.h"

int main() {
    Bpace bpace = Bpace("543524");
    std::cout << bpace.authorize() << std::endl;
}
#include <iostream>
#include "../Core/Math/Vector3.h"

int main()
{
    auto* vec = new Math::Vector3();
    std::cout << (*vec).x;
    return 1;
}

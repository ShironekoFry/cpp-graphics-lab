/*
Write a short program where the user is asked to enter a single character.
Print the value of the character and its ASCII code, using static_cast.
*/

#include <iostream>

int main()
{
    std::cout << "Enter a single character: ";
    char x{};
    std::cin >> x;

    std::cout << "You entered '" << x << "', which has ASCII code " << static_cast<int>(x) << ".\n";

    return 0;
}
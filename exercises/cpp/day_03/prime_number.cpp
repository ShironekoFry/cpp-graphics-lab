/*
Asks the user to enter a number 0 through 9 (inclusive).
If the user enters a number within this range that is prime (2, 3, 5, or 7), print “The digit is prime”.
Otherwise, print “The digit is not prime”.
*/

#include <iostream>

bool isPrime(int x)     // Type name is needed for the input of the function.
{
    return x == 2 || x == 3 || x == 5 || x == 7;

    // Formats like (x == 2, 3, 4) are not allowed.
    // Using logical operators or switch statements can make the function less verbose.
    
}

int main()
{
    std::cout << "Enter a number 0 through 9 (inclusive): ";
    int x{};
    std::cin >> x;

    if ( isPrime(x) )
        std::cout << "The digit is prime.\n";
    else 
        std::cout << "The digit is not prime.\n";
    
    return 0;
}
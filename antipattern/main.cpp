#include <exception>
#include <iostream>
#include <string>

void throwInt()
{
    throw 42;
}

void throwString()
{
    throw std::string("error as std::string");
}

int main()
{
    std::cout << "--- catch (const std::exception&) with throw 42 ---\n";
    try
    {
        try
        {
            throwInt();
        }
        catch (const std::exception& ex)
        {
            std::cout << "std::exception handler: " << ex.what() << '\n';
        }
    }
    catch (...)
    {
        std::cout << "std::exception did NOT catch int, caught by catch (...)\n";
    }

    std::cout << "--- catch (const std::exception&) with throw std::string ---\n";
    try
    {
        try
        {
            throwString();
        }
        catch (const std::exception& ex)
        {
            std::cout << "std::exception handler: " << ex.what() << '\n';
        }
    }
    catch (...)
    {
        std::cout << "std::exception did NOT catch std::string, caught by catch (...)\n";
    }

    std::cout << "--- catch (...) loses the information about the error ---\n";
    try
    {
        throwInt();
    }
    catch (...)
    {
        std::cout << "caught something, but the type and value are unknown\n";
    }
    return 0;
}

#include <iostream>
#include "smart_pointer.hpp"
#include "inheritance_ship.hpp"


typedef e7::utils::object object_t;

class may : public object_t
{
public:
    may(void) : __data__(0)
    {std::cout << "may cstct:" << this << std::endl;}
    may(intptr_t x) : __data__(x)
    {std::cout << "may cstct:" << this << std::endl;}
    may(may const &other) : __data__(other.__data__)
    {std::cout << "may copy cstct:" << this << std::endl;}
    ~may(void)
    {std::cout << "may dstct:" << this << std::endl;}

    void print_data(void)
    {
        std::cout << __data__ << std::endl;
    }
    void set_data(intptr_t x)
    {
        __data__ = x;
    }

private:
    intptr_t __data__;
};

int main(int argc, char *argv[])
{
    e7::utils::smart_pointer<may> p1(new may(2));
    e7::utils::smart_pointer<may> p2(new may(7));
    e7::utils::smart_pointer<may> null_pointer;

    p2.copy(p1);
    p2->print_data();
    p1 = null_pointer;
    p2 = null_pointer;
    std::cout << "end" << std::endl;

    return 0;
}

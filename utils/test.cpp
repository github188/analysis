#include <iostream>
#include "smart_pointer.hpp"
#include "inheritance_ship.hpp"


typedef ::e7::utils::object object_t;

class may : public object_t
{
public:
    may(intptr_t x) : __data__(x)
    {std::cout << "may cstct:" << this << std::endl;}
    ~may(void)
    {std::cout << "may dstct:" << this << std::endl;}

    void print_data(void)
    {
        std::cout << __data__ << std::endl;
    }

private:
    intptr_t __data__;
};

int main(int argc, char *argv[])
{
    ::e7::utils::smart_pointer<may> p1(new may(13));
    ::e7::utils::smart_pointer<may> p2(p1);
    ::e7::utils::smart_pointer<may> p3;

    p1->print_data();
    p1 = NULL;
    p3 = new may(23);
    p3->print_data();
    (*p3).print_data();

    return 0;
}

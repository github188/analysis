#include <iostream>
#include <ctime>
#include <cstdlib>


int main(void)
{
    int *p_rslt = NULL;
    int count = 0;

    srand(static_cast<unsigned int>(time(NULL)));
    while (true) {
        std::cout << "input count: ";
        std::cin.clear();
        std::cin.sync();
        std::cin >> count;
        if (std::cin.fail()) {
            continue;
        }
        if (!(count & 1)) {
            break;
        }
    }

    p_rslt = new int[count]();
    for (int i = 0; i < count; ++i) {
        p_rslt[i] = i + 1;
    }
    for (int i = 0; i < 6; ++i) {
        int left = rand() % count;
        int right = rand() % count;
    #if 1
        int tmp = 0;
        
        tmp = p_rslt[right];
        p_rslt[right] = p_rslt[left];
        p_rslt[left] = tmp;
    #else
        if (left == right) { // 没有这个会出错
            continue;
        }
        p_rslt[left] ^= p_rslt[right];
        p_rslt[right] ^= p_rslt[left];
        p_rslt[left] ^= p_rslt[right];
    #endif
    }
    for (int i = 0; i < count; ++++i) {
        std::cout << p_rslt[i] << " vs " << p_rslt[i + 1] << std::endl;
    }
    delete[] p_rslt;

    std::cout << "press any key to exit..." << std::endl;
    std::cin.clear();
    std::cin.sync();
    static_cast<void>(std::cin.get());

    return 0;
}

#include <map>
#include <list>
#include <string>

namespace std {

class string_ex
{
public:
    explicit string_ex(char const *s)
        : __str__(s)
    {
    }

public:
    operator std::string &(void)
    {
        return __str__;
    }

    std::list<std::string> to_list(std::list<std::string> &l,
                                   std::string const &dlmt) const
    {
        size_t iter_id = 0;

        while (1) {
            size_t iter_id_tmp = __str__.find(iter_id);

            if (string::npos == iter_id_tmp) {
                break;
            }

        }

        return l;
    }

private:
    std::string __str__;
};

}

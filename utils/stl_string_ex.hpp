#include <map>
#include <list>
#include <string>

namespace std {

class string_ex
{
public:
    explicit string_ex(char const *s)
        : __str__(s)
    {}

public:
    operator std::string &(void)
    {
        return __str__;
    }

    void to_list(std::list<std::string> &l, std::string const &dlmt) const
    {
        std::string::size_type iter_id_l = 0;
        std::string::size_type str_size = __str__.size();
        std::string::size_type dlmt_size = dlmt.size();

        while (iter_id_l < str_size) {
            std::string::size_type iter_id_r = __str__.find(dlmt, iter_id_l);

            if (string::npos == iter_id_r) {
                l.push_back(__str__.substr(iter_id_l));
                break;
            }

            std::string elmt(__str__.substr(iter_id_l, iter_id_r - iter_id_l));
            if (!elmt.empty()) {
                l.push_back(__str__.substr(iter_id_l, iter_id_r - iter_id_l));
            }
            iter_id_l = iter_id_r + dlmt_size;
        }

        return;
    }

private:
    std::string __str__;
};

}

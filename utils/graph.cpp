#include <cstdint>
#include <cassert>
#include <iostream>
#include <list>
#include <map>
#include <string>


typedef ::std::string StdString;
// c++11
// template <typename T> using StdList = ::std::list<T>;
// template <typename KEY, typename VALUE>
    // using StdMap = ::std::map<KEY, VALUE>;
template <typename T>
class StdList : public ::std::list<T>
{};
template <typename KEY, typename VALUE>
class StdMap : public ::std::map<KEY, VALUE>
{};

namespace uchiha
{
    namespace ryuuzaki
    {
        class Graph;
    }
}

class uchiha::ryuuzaki::Graph
{
private:
    class Edge
    {
    public:
        explicit Edge(void) : __weight__(0)
        {}
        explicit Edge(intptr_t weight) : __weight__(weight)
        {}

    public:
        intptr_t GetWeight(void) const
        {
            return __weight__;
        }

    private:
        intptr_t __weight__;
    };

    typedef StdMap<StdString, Edge> VertexOutMap;

    class Vertex
    {
    public:
        void AddEdge(StdString const &name, Edge &edge)
        {
            __out_map__[name] = edge;

            return;
        }

        intptr_t FindEdge(StdString const &name)
        {
            return (__out_map__.end() != __out_map__.find(name));
        }

        VertexOutMap const &GetOutMap(void) const
        {
            return __out_map__;
        }

        intptr_t EdgeCount(void) const
        {
            return __out_map__.size();
        }

        intptr_t DelEdge(StdString const &name)
        {
            if (__out_map__.end() == __out_map__.find(name)) {
                return -1;
            }

            __out_map__.erase(name);

            return 0;
        }

        void Print(void) const
        {
            for (StdMap<StdString, Edge>::const_iterator it = __out_map__.begin();
                 __out_map__.end() != it;
                 ++it)
            {
                ::std::cout << it->first
                            << "(" << it->second.GetWeight() << "),";
            }
        }

    private:
        VertexOutMap __out_map__;
    };

public:
    explicit Graph(void);
    intptr_t Init();
    ~Graph(void); // inheritance denied

public:
    intptr_t VertexCount(void) const
    {
        return __vertexes__.size();
    }

    intptr_t EdgeCount(void) const
    {
        return __edge_count__;
    }

    void AddVertex(StdString name);
    intptr_t DelVertex(StdString name);
    void AddEdge(StdString from, StdString to, intptr_t weight = 0);
    intptr_t DelEdge(StdString from, StdString to);
    void Print(void) const;

private:
    typedef StdMap<StdString, Vertex> VertexMap;
    typedef StdMap<StdString, intptr_t> VertexIndegreeMap;

    intptr_t __edge_count__;
    VertexMap __vertexes__;
    VertexIndegreeMap __indegree_of_vertexes__;
};

::uchiha::ryuuzaki::Graph::Graph(void)
    : __edge_count__(0)
{}

intptr_t ::uchiha::ryuuzaki::Graph::Init(void)
{
    return 0;
}

::uchiha::ryuuzaki::Graph::~Graph(void)
{}

void ::uchiha::ryuuzaki::Graph::AddVertex(StdString name)
{
    __vertexes__[name];
    __indegree_of_vertexes__[name];

    return;
}

intptr_t ::uchiha::ryuuzaki::Graph::DelVertex(StdString name)
{
    if (__vertexes__.end() == __vertexes__.find(name)) {
        return -1;
    }

    for (VertexMap::iterator it = __vertexes__.begin();
         __vertexes__.end() != it;
         ++it)
    {
        if (0 == it->second.DelEdge(name)) {
            --__edge_count__;
        }
    }
    __edge_count__ -= __vertexes__[name].EdgeCount();

    VertexOutMap const &out_map = __vertexes__[name].GetOutMap();
    for (VertexOutMap::const_iterator it = out_map.begin();
         out_map.end() != it;
         ++it)
    {
        --__indegree_of_vertexes__[it->first];
    }

    __indegree_of_vertexes__.erase(name);
    __vertexes__.erase(name);

    return 0;
}

void ::uchiha::ryuuzaki::Graph::AddEdge(StdString from,
                                        StdString to,
                                        intptr_t weight)
{
    Edge e(weight);

    AddVertex(from);
    AddVertex(to);

    __vertexes__[from].AddEdge(to, e);
    ++__indegree_of_vertexes__[to];

    ++__edge_count__;

    return;
}

intptr_t ::uchiha::ryuuzaki::Graph::DelEdge(StdString from, StdString to)
{
    if (__vertexes__.end() == __vertexes__.find(from)) {
        return -1;
    }

    if (-1 == __vertexes__[from].DelEdge(to)) {
        return -1;
    }

    --__indegree_of_vertexes__[to];

    --__edge_count__;

    return 0;
}

void ::uchiha::ryuuzaki::Graph::Print(void) const
{
    ::std::cout << "total vertexes: " << VertexCount()  << ::std::endl;
    ::std::cout << "total edges: " << EdgeCount()  << ::std::endl;
    for (VertexMap::const_iterator it = __vertexes__.begin();
         __vertexes__.end() != it;
         ++it)
    {
        ::std::cout << it->first
                    << "(" << __indegree_of_vertexes__.at(it->first) << "): ";
        it->second.Print();
        ::std::cout << ::std::endl;
    }

    return;
}

int main(int argc, char *argv[])
{
    ::uchiha::ryuuzaki::Graph g;

#define TEST 0
#if 0 == TEST

    g.AddEdge("home", "school", 12);
    g.AddEdge("home", "company");
    g.AddEdge("guangzhou", "shenzhen", 100);
    g.DelVertex("home");
    /*assert(0 == g.DelEdge("guangzhou", "shenzhen"));
    assert(0 == g.DelEdge("home", "school"));
    assert(0 == g.DelEdge("home", "company"));
    assert(-1 == g.DelEdge("home", "home"));
    assert(-1 == g.DelEdge("shenzhen", "guangzhou"));
    assert(-1 == g.DelEdge("guangzhou", "shenzhen"));*/
    g.Print();

#elif 1 == TEST

    g.AddEdge("v1", "v2");
    g.AddEdge("v1", "v3");
    g.AddEdge("v1", "v4");
    g.AddEdge("v2", "v4");
    g.AddEdge("v2", "v5");
    g.AddEdge("v3", "v6");
    g.AddEdge("v4", "v3");
    g.AddEdge("v4", "v6");
    g.AddEdge("v4", "v7");
    g.AddEdge("v5", "v4");
    g.AddEdge("v5", "v7");
    g.AddEdge("v7", "v6");

    g.Print();

#endif

    return 0;
}

#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <cmath>
#include <map>
#include <omp.h>
#include "../file_list_loader.h"
#include "config_distance_table_conv.h"

using namespace std;
using namespace std::chrono;

const double INF_DISTANCE = 1000000000;

typedef int hash_t;
typedef map<hash_t, double> Frequences;

double norm(Frequences &f)
{
    double s = 0;
    for (auto &it : f)
        s += it.second * it.second;

    return std::sqrt(s);
}

void normalize(Frequences &f)
{
    auto n = norm(f);

    if (n == 0)
        throw std::runtime_error("norm is 0");

    for (auto &it : f)
        it.second /= n;
}

double mul(Frequences &a, Frequences &b)
{
    double s = 0;
    for (auto &it : a)
    {
//        cout << it.second << " " << b[it.first] << endl;
        s += it.second * b[it.first]; // todo: dont create new elements
    }

    return s;
}

void add(Frequences &a, const Frequences &b)
{
    for (auto &it : b)
        a[it.first] += it.second;
}

Frequences merge_frequences(Frequences &a, Frequences &b)
{
    Frequences m = a;
    add(m, b);
//    for (auto &f : m)
//        cout << f.first << " " << f.second << endl;

    normalize(m);

 //   for (auto &f : m)
 //       cout << f.first << " " << f.second << endl;

    return m;
}

struct Node
{
    string filename;
    struct Merge
    {
        int a, b;
        double distance;
        Merge(int a, int b, double distance) : a(a), b(b), distance(distance) {}
    } merge;

    bool removed;
    Frequences freqs;

    Node(const string &filename, const Frequences &_freqs) : filename(filename), freqs(_freqs), merge(-1, -1, INF_DISTANCE), removed(false){ normalize(freqs); }
    Node(int a, int b, double distance, const Frequences &_freqs) : freqs(_freqs), merge(a, b, distance), removed(false){ normalize(freqs); }
};

double calculate_distance(Node &a, Node &b)
{
    auto sim = mul(a.freqs, b.freqs); // dont need to norm
    if (sim > 1.0001 || sim < 0)
    {
        cerr << "sim is " << sim << endl;
        throw std::runtime_error("calculate_distance sim > 1.0001 || sim < 0");
    }

    return 1.0 - sim;
}

typedef vector<Node> Nodes;

struct MergePair
{
    int a, b;
    double distance;
    MergePair(int a, int b, double distance = INF_DISTANCE) : a(a), b(b), distance(distance){}     

    bool valid() const { return distance < INF_DISTANCE; }
};

struct DistanceTable
{
    Nodes &nodes;

    DistanceTable(Nodes &nodes) : nodes(nodes){}

#if 0
    map<std::pair<int, int>, double> distance_to_cache;
    double distance_to(int a, int b) 
    {
        if (!valid_node(a) || !valid_node(b))
            throw std::runtime_error("distance to invalid node");

        if (a == b)
            return 0;

//        auto p = std::pair<int, int>(std::min(a, b), std::max(a, b));
        auto p = std::make_pair(std::min(a, b), std::max(a, b));

        if (distance_to_cache.find(p) == distance_to_cache.end())
            distance_to_cache[p] = calculate_distance(nodes[a], nodes[b]);

        return distance_to_cache[p];
    }
#else
    vector< vector<double> > distance_to_cache;

    double distance_to(int a, int b) 
    {
        if (!valid_node(a) || !valid_node(b))
            throw std::runtime_error("distance to invalid node");

        if (a == b)
            return 0;

//        auto p = std::pair<int, int>(std::min(a, b), std::max(a, b));
        auto p = std::make_pair(std::min(a, b), std::max(a, b));

    //    if (distance_to_cache.find(p) == distance_to_cache.end())
      //      distance_to_cache[p] = calculate_distance(nodes[a], nodes[b]);
        if (get_cache(p.first, p.second) < 0)
            set_cache(p.first, p.second, calculate_distance(nodes[a], nodes[b]));

        return get_cache(p.first, p.second);
    }

    double get_cache(int a, int b) const
    {
        if ( a >= distance_to_cache.size() )
            return -1;

        auto &a_v = distance_to_cache[a];
        if (b >= a_v.size())
            return -1;

        return a_v[b];
    }

    void set_cache(int a, int b, double x)
    {
        while ( a >= distance_to_cache.size() )
            distance_to_cache.push_back(vector<double>());

        auto &a_v = distance_to_cache[a];
        while (b >= a_v.size())
            a_v.push_back(-1);
            
        a_v[b] = x;
    }

#endif


    void add_node(const MergePair &m)
    {
        if (!valid_node(m.a) || !valid_node(m.b))
            throw std::runtime_error("add_node !valid_node(m.a) || !valid_node(m.b)");

        auto freqs = merge_frequences(nodes[m.a].freqs, nodes[m.b].freqs);
        nodes.push_back(Node(m.a, m.b, m.distance, freqs));
        nodes[m.a].removed = true;
        nodes[m.b].removed = true;
    }

private:
    bool valid_node(int x) const
    {
        return x >=0 && x < int(nodes.size());
    }
};

void print_tree(const DistanceTable &t, int node, int indent = 0)
{
    if (!t.nodes[node].filename.empty())
    {
        for (int i=0; i<indent; i++)
            cout << ' ';
        cout << t.nodes[node].filename << endl;
    }

//    cout << "node " << node << " a " << t.nodes[node].merge.a << " b " << t.nodes[node].merge.b << endl;
    if (t.nodes[node].merge.a >= 0)
    {
        print_tree(t, t.nodes[node].merge.a, indent + 1);
        print_tree(t, t.nodes[node].merge.b, indent + 1);
    }
}

void print(DistanceTable &t)
{
    return;
    for (int a = 0; a < int(t.nodes.size()); a++)
    {
        if (t.nodes[a].removed)
            cout << "x  ";
        else
            cout << "   ";

        for (int b = 0; b < int(t.nodes.size()); b++)
            cout << t.distance_to(a, b) << '\t';
        cout << endl;
    }
}

MergePair find_closest(DistanceTable &table, int to_node)
{
    MergePair best_pair(-1, -1);
   
    for (int b = 0; b < table.nodes.size(); b++)
        if (b != to_node && !table.nodes[b].removed)
        {
            auto dist = table.distance_to(to_node, b); //std::max(table.distance(to_node, b), table.distance(b, to_node));

            if (dist < best_pair.distance)
                best_pair = MergePair(to_node, b, dist);
        }

    return best_pair;
}

MergePair find_merge_pair(DistanceTable &table)
{
    MergePair best_pair(-1, -1);
    for (int a = 0; a < table.nodes.size(); a++)
        if (!table.nodes[a].removed)
        {
            auto pair = find_closest(table, a);
//            cout << a << " closest " << pair.a << " " << pair.b << " " << pair.distance << endl;
            if (pair.valid() && pair.distance < best_pair.distance)
                best_pair = pair;
        }

    return best_pair;
}

void merge_table(DistanceTable &table)
{
    while (true)
    {
        auto merge_pair = find_merge_pair(table);
        if (!merge_pair.valid())
            break;

        cout << "merge: " << merge_pair.a << " " << merge_pair.b << " " << merge_pair.distance << endl;
        print_tree(table, merge_pair.a);
        print_tree(table, merge_pair.b);
        table.add_node(merge_pair);
        print(table);

        //static int count = 0;
        //count ++;
        //if (count >=2)
        //    return;
    }
}

Frequences load_frequences(const string &filename)
{
    Frequences d;

    ifstream f(filename);

    int count = 0;
    f >> count;
    for (int i = 0; i < count; i++)
    {
        hash_t hash = 0;
        double freq = 0;
        f >> hash >> freq;
        if (d[hash] != 0)
            throw std::runtime_error("duplicate hash_t in load_frequences");
        d[hash] = freq;
    }

	if (f.fail() || f.bad())
        throw std::runtime_error("failed to load frequences");

    return d;
}

void load_nodes(const string &file_list_name, Nodes &nodes, int kmer_len)
{
	FileListLoader file_list(file_list_name);
    int file_number = 0;
    nodes.reserve(file_list.files.size());

    for (int file_number = 0; file_number < int(file_list.files.size()); file_number ++)
    {
        auto &file = file_list.files[file_number];
        auto freqs = load_frequences(file.filename + ".conv." + std::to_string(kmer_len) + ".mer");
        nodes.push_back(Node(file.filename, freqs));
    }
}

int main(int argc, char const *argv[])
{
    try
    {
		Config config(argc, argv);

		auto before = high_resolution_clock::now();
        Nodes nodes;
        load_nodes(config.file_list, nodes, config.kmer_len);

        DistanceTable table(nodes);
        print(table);
        merge_table(table);

		cerr << "total time (sec) " << std::chrono::duration_cast<std::chrono::seconds>( high_resolution_clock::now() - before ).count() << endl;
        return 0;
    }
    catch ( exception & x )
    {
        cerr << x.what() << endl;
		cerr << "exit 3" << endl;
		return 3;
    }
  //  catch ( string & x )
  //  {
  //      cerr << x << endl;
		//cerr << "exit 4" << endl;
		//return 4;
  //  }
    catch ( const char * x )
    {
        cerr << x << endl;
		cerr << "exit 5" << endl;
		return 5;
    }
    catch ( char const * x )
    {
        cerr << x << endl;
		cerr << "exit 5.1" << endl;
		return 5;
    }
    catch ( ... )
    {
        cerr << "unknown exception" << endl;
//		cerr << "exit 6" << endl;
		return 6;
    }
}
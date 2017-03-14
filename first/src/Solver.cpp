#include "Solver.h"

namespace Valkovic
{
    class MyException
    {
    };

    class Edge
    {
    public:
        unsigned int from;
        unsigned int to;
        unsigned int realFrom;

        Edge() : from(UINT_MAX), to(UINT_MAX), realFrom(UINT_MAX)
        {}

        Edge(unsigned int from, unsigned int to) : Edge()
        {
            this->from = from;
            this->to = to;
        }

        unsigned int To(unsigned int from)
        {
            return this->from == from ? this->to : this->from;
        }

        bool Can(unsigned int from)
        {
#ifndef __PROGTEST__
            if (this->from != from && this->to != from) //node not used
                throw new MyException();
#endif

            return this->from == UINT_MAX || this->from != from;

            /*
			if( this->from == UINT_MAX ) //link is not in flow
			{
				return true; //can be used in all situtations
			}
			else //link alredy in flow
			{
				if( this->from == from ) //already used in same direction
					return false;
				else //alredy used in oposite directon
					return true;
			}*/
        }

        void Use(unsigned int from)
        {
#ifndef __PROGTEST__
            if (!this->Can(from))
                throw new MyException();
#endif

            this->from = from;
        }
    };

    /*
    bool operator<( const Edge& first, const Edge& sec )
    {
        return first.link->m_From < sec.link->m_From && first.link->m_To < sec.link->m_To;
    }*/

    class Vertex
    {
    public:
        unsigned int index;
        map<int, vector<Edge*>> edges;

        Vertex() : index(UINT_MAX)
        {}

        Vertex(unsigned int index) : index(index)
        {}

        Edge* getEdge(int to)
        {
            for (Edge* e : this->edges[to])
                if (e->from == to || e->to == to)
                    if (e->Can(this->index))
                        return e;
            throw new MyException();
        }
    };

    bool operator<(const Vertex& first, const Vertex& sec)
    {
        return first.index < sec.index;
    }


    uint64_t FordFuklerson(const string& start, const string& end, unordered_map<string, Vertex>& vertexes)
    {
        Vertex begin = vertexes[start];
        Vertex finish = vertexes[end];

        while (true)
        {
            stack<Vertex> toProccess;
            set<Vertex> proccessed;
            map<string, string> toFrom;
            //DFS
            toProccess.push(begin);
            while (!toProccess.empty())
            {
                Vertex cur = toProccess.top();

                if (cur.name == finish.name)
                    break;

                toProccess.pop();
                if (proccessed.find(cur) != proccessed.end())
                    continue;
                proccessed.insert(cur);

                for (Edge* edge : cur.edges)
                    if (edge->can(cur.name))
                    {
                        string to = edge->to(cur.name);
                        toProccess.push(vertexes[to]);
                        toFrom.insert(pair<string, string>(to, cur.name));
                    }
            }
            //reconstruct path
            if (toFrom.find(finish.name) == toFrom.end())
                break;
            else
            {
                Vertex to = finish;
                do
                {
                    Vertex from = vertexes[toFrom[to.name]];
                    Edge* e = from.getEdge(to.name);
                    e->use(from.name);
                    to = from;
                } while (to.name != start);
            }
        }
#ifndef __PROGTEST__
        cout << "End of FordFuklerson" << endl;
#endif

        uint64_t paths = 0;
        for (Edge* e : begin.edges)
            if (e->isUsed(start))
                paths++;

        return paths;

    }

    void FloydWarshal(vector<CLink>& links, string& node, map<string, double>& latencies, double& maxLatency)
    {
        uint64_t countOfIndexes = 0;
        map<string, uint64_t> indexes;
        vector<vector<double>> latency;


        //fill 2d array
        for (size_t b = 0, e = links.size(); b < e; b++)
        {
            CLink& link = links[b];
            //add vector if required
            if (indexes.find(link.m_From) == indexes.end())
            {
                indexes.insert(pair<string, uint64_t>(link.m_From, countOfIndexes));
                latency.push_back(vector<double>());
                countOfIndexes++;
            }
            if (indexes.find(link.m_To) == indexes.end())
            {
                indexes.insert(pair<string, uint64_t>(link.m_To, countOfIndexes));
                latency.push_back(vector<double>());
                countOfIndexes++;
            }
            //get indexes
            uint64_t indexFrom = indexes[link.m_From];
            uint64_t indexTo = indexes[link.m_To];
            //resize vectors if required
            if (latency[indexFrom].size() <= indexTo)
            {
                size_t oldSize = latency[indexFrom].size();
                size_t inserted = countOfIndexes - oldSize;
                latency[indexFrom].resize(countOfIndexes);
                for (size_t i = 0; i < inserted; i++)
                    latency[indexFrom][oldSize + i] = numeric_limits<double>::max();
            }
            if (latency[indexTo].size() <= indexFrom)
            {
                size_t oldSize = latency[indexTo].size();
                size_t inserted = countOfIndexes - oldSize;
                latency[indexTo].resize(countOfIndexes);
                for (size_t i = 0; i < inserted; i++)
                    latency[indexTo][oldSize + i] = numeric_limits<double>::max();
            }
            if (latency[indexFrom][indexTo] > link.m_Delay)
                latency[indexFrom][indexTo] = link.m_Delay;
            if (latency[indexTo][indexFrom] > link.m_Delay)
                latency[indexTo][indexFrom] = link.m_Delay;
        }

        //resize vectors
        for (uint64_t i = 0; i < countOfIndexes; i++)
        {
            size_t oldSize = latency[i].size();
            latency[i].resize(countOfIndexes);
            for (uint64_t j = 0, set = countOfIndexes - oldSize; j < set; j++)
                latency[i][oldSize + j] = numeric_limits<double>::max();
            latency[i][i] = 0;
        }

#ifndef __PROGTEST__
        cout << "Vertex" << endl;
        for (auto v : indexes)
            cout << v.first << " : " << v.second << endl;
        cout << endl << "Before floyd" << endl;

        for (uint64_t i = 0; i < countOfIndexes; i++, cout << endl)
            for (uint64_t j = 0; j < countOfIndexes; j++)
                cout << latency[i][j] << " ";
#endif


        //FloydWarshal
        for (uint64_t i = 0; i < countOfIndexes; i++)
            for (uint64_t j = 0; j < countOfIndexes; j++)
                for (uint64_t k = 0; k < countOfIndexes; k++)
                    if (latency[j][k] > latency[j][i] + latency[i][k])
                        latency[j][k] = latency[j][i] + latency[i][k];

#ifndef __PROGTEST__
        cout << endl << "After floyd" << endl;
        for (uint64_t i = 0; i < countOfIndexes; i++, cout << endl)
            for (uint64_t j = 0; j < countOfIndexes; j++)
                cout << latency[i][j] << " ";
#endif

        //find lowest
        double lowestNumber = numeric_limits<double>::max();
        uint64_t index = -1;
        for (uint64_t i = 0; i < countOfIndexes; i++)
        {
            double max = numeric_limits<double>::min();
            for (uint64_t j = 0; j < countOfIndexes; j++)
                if (latency[i][j] > max)
                    max = latency[i][j];
            if (max < lowestNumber)
            {
                lowestNumber = max;
                index = i;
            }
        }

#ifndef __PROGTEST__
        cout << "Nejmensi index " << index << endl;
#endif

        //fil info
        for (auto c : indexes)
            if (c.second == index)
            {
                node = c.first;
            }
            else
            {
                latencies.insert(pair<string, double>(c.first, latency[index][c.second]));
            }
        maxLatency = lowestNumber;

#ifndef __PROGTEST__
        cout << "Nejmensi node " << node << " s delay " << maxLatency << endl << endl << endl << endl;
#endif

        return;
    }

}


void CSolver::Solve(shared_ptr<CCenter> x)
{
    Valkovic::FloydWarshal(x->m_Links, x->m_Center, x->m_Delays, x->m_MaxDelay);
}


void CSolver::Solve(shared_ptr<CRedundancy> param)
{
    using Valkovic::Vertex;
    using Valkovic::Edge;

    //get center
    string centerName = param->m_Center;

    //prepare structures
    vector<Edge*> edges;
    map<unsigned int, string> indexToName;
    map<string, unsigned int> nameToIndex;
    vector<Vertex*> vertexes;

    //fill structures
    for (CLink& x : param->m_Links)
    {

        if (nameToIndex.find(x.m_From) == nameToIndex.end())
        {
            indexToName.insert(pair<unsigned int, string>(vertexes.size(), x.m_From));
            nameToIndex.insert(pair<string, unsigned int>(x.m_From, vertexes.size()));
            vertexes.push_back(new Vertex((unsigned int) vertexes.size()));
        }
        if (nameToIndex.find(x.m_To) == nameToIndex.end())
        {
            indexToName.insert(pair<unsigned int, string>((unsigned int) vertexes.size(), x.m_To));
            nameToIndex.insert(pair<string, unsigned int>(x.m_To, (unsigned int) vertexes.size()));
            vertexes.push_back(new Vertex((unsigned int) vertexes.size()));
        }


        Edge* cur = new Edge(nameToIndex[x.m_From],nameToIndex[x.m_To]);
        edges.push_back(cur);
        //TODO insert into edges
    }

#ifndef __PROGTEST__
    cout << "Links" << endl;
    for (
        auto v :
            vertexes)
    {
        cout << v.first <<
             endl;
        for (
            auto e :
                v.second.edges)
            cout << "\t - " << e->link->m_From << ":" << e->link->m_To <<
                 endl;
    }
#endif

//for each vertex except center apply FordFulkerson
    for (
        auto v :
            vertexes)
    {
        if (v.first == centerName)
            continue;

//reset capacity
        for (
            Edge* e :
                edges)
            e->
                    from = nullptr;

#ifndef __PROGTEST__
        cout << "Running FordFuklerson between " << centerName << " and " << v.first <<
             endl;
#endif

//FordFuklerson
        uint64_t result = Valkovic::FordFuklerson(centerName, v.first, vertexes);
        param->m_Redundancy[v.first] =
                result;

#ifndef __PROGTEST__
        cout << "Founded " << result << " paths to " + v.first <<
             endl;
#endif
        param->m_Redundancy.
                insert(pair<string, int>(v.first, (int) result));

    }

    for (
        Edge* e :
            edges)
        delete
                e;
}


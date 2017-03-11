#ifndef __PROGTEST__
#include "devs.h"
#endif

class CSolver
{
public:
    static void Solve(shared_ptr<CCenter> x)
    {
        set<string> vertex;
        for(CLink link : x->m_Links)
        {
            vertex.insert(link.m_From);
            vertex.insert(link.m_To);
        }
        cout << "Count of vertex: " << vertex.size() << endl;
    }

    static void Solve(shared_ptr<CRedundancy> x)
    {}

    CSolver(void)
    {}

    ~CSolver(void)
    {}

    void Start(int thrCnt)
    {}

    void Stop(void)
    {}

    void AddCustomer(ACustomer c)
    {}

private:
    // todo
};

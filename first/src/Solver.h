#ifndef __PROGTEST__
#include "devs.h"
#endif

class CSolver
{
public:
	static void Solve( shared_ptr<CCenter> x );

	static void Solve( shared_ptr<CRedundancy> x );

	CSolver( void )
	{}

	~CSolver( void )
	{}

	void Start( int thrCnt )
	{}

	void Stop( void )
	{}

	void AddCustomer( ACustomer c )
	{}

private:
	static void FloydWarshal( vector<CLink>& links, string & node, map<string, double> &latencies, double &maxLatency );
};

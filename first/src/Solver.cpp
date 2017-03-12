#include "Solver.h"

namespace Valkovic
{
	class Edge
	{
	public:
		CLink* link;
		unsigned char capacity;

		Edge( CLink &l ) : link( &l ), capacity( 1 )
		{}

		string to( string from )
		{
			return link->m_From == from ? link->m_To : link->m_From;
		}
	};

	class Vertex
	{
	public:
		string* name;
		set<Edge> edges;

		Vertex( string& name ) : name( &name )
		{}
	};
}


void CSolver::Solve( shared_ptr<CCenter> x )
{
	FloydWarshal( x->m_Links, x->m_Center, x->m_Delays, x->m_MaxDelay );
}


void CSolver::Solve( shared_ptr<CRedundancy> param )
{
	string nodeName;
	map<string, double> latencies;
	double maxLatency;
	FloydWarshal( param->m_Links, nodeName, latencies, maxLatency );

	//prepare links
	unordered_map<string, set<CLink*>> links( latencies.size() + 1 );
	links.insert( pair<string, set<CLink*>>( nodeName, set<CLink*>() ) );
	for( auto x : latencies )
		links.insert( pair<string, set<CLink*>>( x.first, set<CLink*>() ) );

	//fill map
	for( size_t b = 0, e = param->m_Links.size(); b < e; b++ )
	{
		links[param->m_Links[b].m_From].insert( &param->m_Links[b] );
		links[param->m_Links[b].m_To].insert( &param->m_Links[b] );
	}

#ifndef __PROGTEST__
	cout << "Links";
	for( auto v : links )
	{
		cout << v.first << endl;
		for( auto e : v.second )
			cout << "\t - " << e->m_From << ":" << e->m_To << "  ?  " << e->m_Delay << endl;
	}
#endif
}

void CSolver::FloydWarshal( vector<CLink>& links, string & node, map<string, double> &latencies, double &maxLatency )
{
	uint64_t countOfIndexes = 0;
	map<string, uint64_t> indexes;
	vector<vector<double>> latency;


	//fill 2d array
	for( auto link : links )
	{
		//add vector if required
		if( indexes.find( link.m_From ) == indexes.end() )
		{
			indexes.insert( pair<string, uint64_t>( link.m_From, countOfIndexes ) );
			latency.push_back( vector<double>() );
			countOfIndexes++;
		}
		if( indexes.find( link.m_To ) == indexes.end() )
		{
			indexes.insert( pair<string, uint64_t>( link.m_To, countOfIndexes ) );
			latency.push_back( vector<double>() );
			countOfIndexes++;
		}
		//get indexes
		uint64_t indexFrom = indexes[link.m_From];
		uint64_t indexTo = indexes[link.m_To];
		//resize vectors if required
		if( latency[indexFrom].size() <= indexTo )
		{
			size_t oldSize = latency[indexFrom].size();
			size_t inserted = countOfIndexes - oldSize;
			latency[indexFrom].resize( countOfIndexes );
			for( size_t i = 0; i < inserted; i++ )
				latency[indexFrom][oldSize + i] = numeric_limits<double>::max();
		}
		if( latency[indexTo].size() <= indexFrom )
		{
			size_t oldSize = latency[indexTo].size();
			size_t inserted = countOfIndexes - oldSize;
			latency[indexTo].resize( countOfIndexes );
			for( size_t i = 0; i < inserted; i++ )
				latency[indexTo][oldSize + i] = numeric_limits<double>::max();
		}
		if( latency[indexFrom][indexTo] > link.m_Delay )
			latency[indexFrom][indexTo] = link.m_Delay;
		if( latency[indexTo][indexFrom] > link.m_Delay )
			latency[indexTo][indexFrom] = link.m_Delay;
	}

	//resize vectors
	for( uint64_t i = 0; i < countOfIndexes; i++ )
	{
		size_t oldSize = latency[i].size();
		latency[i].resize( countOfIndexes );
		for( uint64_t j = 0, set = countOfIndexes - oldSize; j < set; j++ )
			latency[i][oldSize + j] = numeric_limits<double>::max();
		latency[i][i] = 0;
	}

#ifndef __PROGTEST__
	cout << "Vertex" << endl;
	for( auto v : indexes )
		cout << v.first << " : " << v.second << endl;
	cout << endl << "Before floyd" << endl;

	for( uint64_t i = 0; i < countOfIndexes; i++, cout << endl )
		for( uint64_t j = 0; j < countOfIndexes; j++ )
			cout << latency[i][j] << " ";
#endif


	//FloydWarshal
	for( uint64_t i = 0; i < countOfIndexes; i++ )
		for( uint64_t j = 0; j < countOfIndexes; j++ )
			for( uint64_t k = 0; k < countOfIndexes; k++ )
				if( latency[j][k] > latency[j][i] + latency[i][k] )
					latency[j][k] = latency[j][i] + latency[i][k];

#ifndef __PROGTEST__
	cout << endl << "After floyd" << endl;
	for( uint64_t i = 0; i < countOfIndexes; i++, cout << endl )
		for( uint64_t j = 0; j < countOfIndexes; j++ )
			cout << latency[i][j] << " ";
#endif

	//find lowest
	double lowestNumber = numeric_limits<double>::max();
	uint64_t index = -1;
	for( uint64_t i = 0; i < countOfIndexes; i++ )
	{
		double max = numeric_limits<double>::min();
		for( uint64_t j = 0; j < countOfIndexes; j++ )
			if( latency[i][j] > max )
				max = latency[i][j];
		if( max < lowestNumber )
		{
			lowestNumber = max;
			index = i;
		}
	}

#ifndef __PROGTEST__
	cout << "Nejmensi index " << index << endl;
#endif

	//fil info
	for( auto c : indexes )
		if( c.second == index )
		{
			node = c.first;
		}
		else
		{
			latencies.insert( pair<string, double>( c.first, latency[index][c.second] ) );
		}
	maxLatency = lowestNumber;

#ifndef __PROGTEST__
	cout << "Nejmensi node " << node << " s delay " << maxLatency << endl << endl << endl << endl;
#endif

	return;
}

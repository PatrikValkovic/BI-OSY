#include "Solver.h"

namespace Valkovic
{
	class Edge
	{
	public:
		CLink* link;
		unsigned char capacity;

		Edge() : link( nullptr ), capacity( 1 )
		{}

		Edge( CLink &l ) : Edge()
		{
			this->link = &l;
		}

		string to( string from )
		{
			return link->m_From == from ? link->m_To : link->m_From;
		}
	};

	bool operator<( const Edge& first, const Edge& sec )
	{
		return first.link->m_From < sec.link->m_From;
	}

	class Vertex
	{
	public:
		string name;
		set<Edge> edges;

		Vertex()
		{}

		Vertex( string name ) : name( name )
		{}
	};

	bool operator<( const Vertex& first, const Vertex& sec )
	{
		return first.name < sec.name;
	}


	uint64_t FordFuklerson( const string& start, const string& end, unordered_map<string, Vertex>& vertexes )
	{
		Vertex begin = vertexes[start];
		Vertex finish = vertexes[end];

		list<list<Vertex>> paths;

		while( true )
		{
			queue<Vertex> toProccess;
			set<Vertex> proccessed;
			map<string, string> toFrom;
			//BFS
			toProccess.push( begin );
			while( !toProccess.empty() )
			{
				Vertex cur = toProccess.front();

				if( cur.name == finish.name )
					break;

				toProccess.pop();
				if( proccessed.find( cur ) != proccessed.end() )
					continue;
				proccessed.insert( cur );

				for( auto edge : cur.edges )
					if( edge.capacity == 1 )
					{
						string to = edge.to(cur.name);
						toProccess.push( vertexes[to] );
						toFrom.insert(pair<string,string>(to,cur.name));
					}
			}

			if( toFrom.find(finish.name) == toFrom.end())
				break;
			else
			{
				list<Vertex> newPath;
				newPath.push_back( finish.name );
				while( newPath.back().name != start )
					newPath.push_back(toFrom[newPath.back().name]);
				paths.push_back( newPath );
			}
		}
		return 0;
	}

	void FloydWarshal( vector<CLink>& links, string & node, map<string, double> &latencies, double &maxLatency )
	{
		uint64_t countOfIndexes = 0;
		map<string, uint64_t> indexes;
		vector<vector<double>> latency;


		//fill 2d array
		for( size_t b = 0, e = links.size(); b < e; b++ )
		{
			CLink& link = links[b];
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

}


void CSolver::Solve( shared_ptr<CCenter> x )
{
	Valkovic::FloydWarshal( x->m_Links, x->m_Center, x->m_Delays, x->m_MaxDelay );
}


void CSolver::Solve( shared_ptr<CRedundancy> param )
{
	using Valkovic::Vertex;
	using Valkovic::Edge;

	//get center
	string centerName;
	map<string, double> latencies;
	double maxLatency;
	Valkovic::FloydWarshal( param->m_Links, centerName, latencies, maxLatency );

	//prepare vertexes
	vector<Edge> edges( param->m_Links.size() );
	unordered_map<string, Vertex> vertexes( latencies.size() + 1 );
	vertexes.insert( pair<string, Vertex>( centerName, Vertex( centerName ) ) );
	for( auto x : latencies )
		vertexes.insert( pair<string, Vertex>( x.first, Vertex( x.first ) ) );

	//fill map
	for( size_t b = 0, e = param->m_Links.size(); b < e; b++ )
	{
		Edge cur = Edge( param->m_Links[b] );
		edges[b] = cur;
		vertexes[param->m_Links[b].m_From].edges.insert( cur );
		vertexes[param->m_Links[b].m_To].edges.insert( cur );
	}

#ifndef __PROGTEST__
	cout << "Links";
	for( auto v : vertexes )
	{
		cout << v.first << endl;
		for( auto e : v.second.edges )
			cout << "\t - " << e.link->m_From << ":" << e.link->m_To << "  ?  " << e.link->m_Delay << endl;
	}
#endif

	//for each vertex except center apply FordFulkerson
	for( auto v : vertexes )
	{
		if( v.first == centerName )
			continue;

		//reset capacity
		for( Edge& e : edges )
			e.capacity = 1;

#ifndef __PROGTEST__
		cout << "Running FordFuklerson between " << centerName << " and " << v.first << endl;
#endif

		//FordFuklerson
		uint64_t result = Valkovic::FordFuklerson( centerName, v.first, vertexes );
		param->m_Redundancy[v.first] = result;

#ifndef __PROGTEST__
		cout << "Founded " << result << " paths" << endl;
#endif
	}
}


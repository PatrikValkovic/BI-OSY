#include "Solver.h"

namespace Valkovic
{
	class MyException {};

	class Edge
	{
	public:
		CLink* link;
		const char* from;

		Edge() : link( nullptr ), from( nullptr )
		{}

		Edge( CLink &l ) : Edge()
		{
			this->link = &l;
		}

		string to( string from )
		{
			return link->m_From == from ? link->m_To : link->m_From;
		}

		bool can( string from )
		{
			if( link->m_From != from && link->m_To != from ) //node not used
				return false;

			if( this->from == nullptr ) //link is not in flow
			{
				return true; //can be used in all situtations
			}
			else //link alredy in flow
			{
				if( strcmp( this->from, from.c_str() ) == 0 ) //already used in same direction
					return false;
				else //alredy used in oposite directon
					return true;
			}
		}

		void use( string from )
		{
			if( !can( from ) )
				throw new MyException();

			this->from = this->link->m_From == from ? this->link->m_From.c_str() : this->link->m_To.c_str();
		}

		bool isUsed( string from )
		{
			return this->from != nullptr && strcmp( this->from, from.c_str() ) == 0;
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
		string name;
		vector<Edge*> edges;

		Vertex()
		{}

		Vertex( string name ) : name( name )
		{}

		Edge* getEdge( string to )
		{
			for( Edge* e : this->edges )
				if( e->link->m_From == to || e->link->m_To == to )
					if( e->can( this->name ) )
						return e;
			throw new MyException();
		}
	};

	bool operator<( const Vertex& first, const Vertex& sec )
	{
		return first.name < sec.name;
	}


	uint64_t FordFuklerson( const string& start, const string& end, unordered_map<string, Vertex>& vertexes )
	{
		Vertex begin = vertexes[start];
		Vertex finish = vertexes[end];

		while( true )
		{
			stack<Vertex> toProccess;
			set<Vertex> proccessed;
			map<string, string> toFrom;
			//DFS
			toProccess.push( begin );
			while( !toProccess.empty() )
			{
				Vertex cur = toProccess.top();

				if( cur.name == finish.name )
					break;

				toProccess.pop();
				if( proccessed.find( cur ) != proccessed.end() )
					continue;
				proccessed.insert( cur );

				for( Edge* edge : cur.edges )
					if( edge->can( cur.name ) )
					{
						string to = edge->to( cur.name );
						toProccess.push( vertexes[to] );
						toFrom.insert( pair<string, string>( to, cur.name ) );
					}
			}
			//reconstruct path
			if( toFrom.find( finish.name ) == toFrom.end() )
				break;
			else
			{
				Vertex to = finish;
				do
				{
					Vertex from = vertexes[toFrom[to.name]];
					Edge* e = from.getEdge( to.name );
					e->use( from.name );
					to = from;
				} while( to.name != start );
			}
		}
#ifndef __PROGTEST__
		cout << "End of FordFuklerson" << endl;
#endif

		uint64_t paths = 0;
		for( Edge* e : begin.edges )
			if( e->isUsed( start ) )
				paths++;

		return paths;

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
	string centerName = param->m_Center;

	//prepare vertexes
	vector<Edge*> edges;
	unordered_map<string, Vertex> vertexes;
	for( CLink& x : param->m_Links )
	{
		if( vertexes.find( x.m_From ) == vertexes.end() )
			vertexes.insert( pair<string, Vertex>( x.m_From, Vertex( x.m_From ) ) );
		if( vertexes.find( x.m_To ) == vertexes.end() )
			vertexes.insert( pair<string, Vertex>( x.m_To, Vertex( x.m_To ) ) );

		Edge* cur = new Edge( x );
		edges.push_back( cur );
		vertexes[x.m_From].edges.push_back( cur );
		vertexes[x.m_To].edges.push_back( cur );
	}

#ifndef __PROGTEST__
	cout << "Links" << endl;
	for( auto v : vertexes )
	{
		cout << v.first << endl;
		for( auto e : v.second.edges )
			cout << "\t - " << e->link->m_From << ":" << e->link->m_To << endl;
	}
#endif

	//for each vertex except center apply FordFulkerson
	for( auto v : vertexes )
	{
		if( v.first == centerName )
			continue;

		//reset capacity
		for( Edge* e : edges )
			e->from = nullptr;

#ifndef __PROGTEST__
		cout << "Running FordFuklerson between " << centerName << " and " << v.first << endl;
#endif

		//FordFuklerson
		uint64_t result = Valkovic::FordFuklerson( centerName, v.first, vertexes );
		param->m_Redundancy[v.first] = result;

#ifndef __PROGTEST__
		cout << "Founded " << result << " paths to " + v.first << endl;
#endif
		param->m_Redundancy.insert( pair<string, int>( v.first, (int)result ) );

	}

	for( Edge* e : edges )
		delete e;
}


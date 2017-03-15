#ifndef __PROGTEST__
#include "devs.h"
#endif

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
		bool used;

		Edge() : from( UINT_MAX ), to( UINT_MAX ), used( false )
		{}

		Edge( unsigned int from, unsigned int to ) : Edge()
		{
			this->from = from;
			this->to = to;
		}

		unsigned int To( unsigned int from )
		{
			return this->from == from ? this->to : this->from;
		}

		bool Can( unsigned int from )
		{
#ifdef __VALKOVIC__
			if( this->from != from && this->to != from ) //node not used
				throw new MyException();
#endif

			return !this->used;
		}

		void Use( unsigned int from )
		{
#ifdef __VALKOVIC__
			if( !this->Can( from ) )
				throw new MyException();
#endif

			this->used = true;
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

		Vertex() : index( UINT_MAX )
		{}

		Vertex( unsigned int index ) : index( index )
		{}

		Edge* getEdge( unsigned int to )
		{
			for( Edge* e : this->edges[to] )
				if( e->from == to || e->to == to )
					if( e->Can( this->index ) )
						return e;
			throw new MyException();
		}
	};

	bool operator<( const Vertex& first, const Vertex& sec )
	{
		return first.index < sec.index;
	}

	//SOURCE: http://www.geeksforgeeks.org/find-edge-disjoint-paths-two-vertices/
	bool bfs( const vector<vector<int>> &graph, int s, int t, vector<int> &parent )
	{
		// Create a visited array and mark all vertices as not visited
		size_t size = graph.size();
		vector<bool> visited( size, false );

		// Create a queue, enqueue source vertex and mark source vertex
		// as visited
		queue <int> q;
		q.push( s );
		visited[s] = true;
		parent[s] = -1;

		// Standard BFS Loop
		while( !q.empty() )
		{
			int u = q.front();
			q.pop();

			for( size_t v = 0; v < size; v++ )
			{
				if( visited[v] == false && graph[u][v] > 0 )
				{
					q.push( v );
					parent[v] = u;
					visited[v] = true;
				}
			}
		}

		// If we reached sink in BFS starting from source, then return
		// true, else false
		return ( visited[t] == true );
	}

	unsigned int FordFuklerson( vector<vector<int>> graph, int start, int end )
	{
		int u, v;

		vector<int> parent( graph.size() );  // This array is filled by BFS and to store path

		int max_flow = 0;  // There is no flow initially

						   // Augment the flow while tere is path from source to sink
		while( bfs( graph, start, end, parent ) )
		{
			// Find minimum residual capacity of the edges along the
			// path filled by BFS. Or we can say find the maximum flow
			// through the path found.
			int path_flow = INT_MAX;

			for( v = end; v != start; v = parent[v] )
			{
				u = parent[v];
				path_flow = min( path_flow, graph[u][v] );
			}

			// update residual capacities of the edges and reverse edges
			// along the path
			for( v = end; v != start; v = parent[v] )
			{
				u = parent[v];
				graph[u][v] -= path_flow;
				graph[v][u] += path_flow;
			}

			// Add path flow to overall flow
			max_flow += path_flow;
		}

		// Return the overall flow (max_flow is equal to maximum
		// number of edge-disjoint paths)
		return max_flow;
	}

	void FloydWarshal( vector<CLink>& links, string& node, map<string, double>& latencies, double& maxLatency )
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

#ifdef __VALKOVIC__
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

#ifdef __VALKOVIC__
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

#ifdef __VALKOVIC__
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

#ifdef __VALKOVIC__
		cout << "Nejmensi node " << node << " s delay " << maxLatency << endl << endl << endl << endl;
#endif

		return;
	}

	class RedundancyData
	{
	public:
		shared_ptr<CRedundancy> problem;
		shared_ptr<CCustomer> customer;

		RedundancyData() {}
		RedundancyData( shared_ptr<CRedundancy> problem, shared_ptr<CCustomer> customer )
			: problem( problem ), customer( customer ) {}
	};

	class CenterData
	{
	public:
		shared_ptr<CCenter> problem;
		shared_ptr<CCustomer> customer;

		CenterData() {}
		CenterData( shared_ptr<CCenter> problem, shared_ptr<CCustomer> customer )
			: problem( problem ), customer( customer ) {}
	};

}

class CSolver
{
public:
	static void Solve( shared_ptr<CCenter> x );

	static void Solve( shared_ptr<CRedundancy> x );

	CSolver( void );

	~CSolver( void );

	void Start( int thrCnt );

	void Stop( void );

	void AddCustomer( shared_ptr<CCustomer> c );

private:
	int maxProblemsInQueue;

	bool stoped;
	vector<thread*>* threads = nullptr;
	vector<thread*> clientsThreads;

	static void WorkingThreadFn( CSolver* data );

	static void ClientCenterFn( CSolver* data, shared_ptr<CCustomer> client );
	static void ClientRedundancyFn( CSolver* data, shared_ptr<CCustomer> client );

	mutex redundancyMutex;
	queue<Valkovic::RedundancyData> redundancy;
	mutex centerMutex;
	queue<Valkovic::CenterData> center;

	atomic_bool ended;
	atomic_int clients;
};




void CSolver::Solve( shared_ptr<CCenter> x )
{
	Valkovic::FloydWarshal( x->m_Links, x->m_Center, x->m_Delays, x->m_MaxDelay );
}


void CSolver::Solve( shared_ptr<CRedundancy> param )
{
	unsigned int countOfIndexes = 0;
	map<string, unsigned int> indexes;
	vector<vector<int>> linksMap;


	//fill 2d array
	for( size_t b = 0, e = param->m_Links.size(); b < e; b++ )
	{
		CLink& link = param->m_Links[b];
		//add vector if required
		if( indexes.find( link.m_From ) == indexes.end() )
		{
			indexes.insert( pair<string, unsigned int>( link.m_From, countOfIndexes ) );
			linksMap.push_back( vector<int>() );
			countOfIndexes++;
		}
		if( indexes.find( link.m_To ) == indexes.end() )
		{
			indexes.insert( pair<string, unsigned int>( link.m_To, countOfIndexes ) );
			linksMap.push_back( vector<int>() );
			countOfIndexes++;
		}
		//get indexes
		uint64_t indexFrom = indexes[link.m_From];
		uint64_t indexTo = indexes[link.m_To];
		//resize vectors if required
		if( linksMap[indexFrom].size() <= indexTo )
		{
			size_t oldSize = linksMap[indexFrom].size();
			size_t inserted = countOfIndexes - oldSize;
			linksMap[indexFrom].resize( countOfIndexes );
			for( size_t i = 0; i < inserted; i++ )
				linksMap[indexFrom][oldSize + i] = 0;
		}
		if( linksMap[indexTo].size() <= indexFrom )
		{
			size_t oldSize = linksMap[indexTo].size();
			size_t inserted = countOfIndexes - oldSize;
			linksMap[indexTo].resize( countOfIndexes );
			for( size_t i = 0; i < inserted; i++ )
				linksMap[indexTo][oldSize + i] = 0;
		}
		linksMap[indexFrom][indexTo]++;
		linksMap[indexTo][indexFrom]++;
	}

	//resize vectors
	for( uint64_t i = 0; i < countOfIndexes; i++ )
	{
		size_t oldSize = linksMap[i].size();
		linksMap[i].resize( countOfIndexes );
		for( uint64_t j = 0, set = countOfIndexes - oldSize; j < set; j++ )
			linksMap[i][oldSize + j] = 0;
		linksMap[i][i] = 0;
	}

#ifdef __VALKOVIC__
	cout << "Map: " << endl;
	unsigned int len = linksMap.size();
	for( unsigned int i = 0; i < len; i++, cout << endl )
		for( unsigned int j = 0; j < len; j++ )
			cout << linksMap[i][j] << '\t';
#endif
	int centerIndex = indexes[param->m_Center];
	for( auto v : indexes )
	{
		if( v.first == param->m_Center )
			continue;

#ifdef __VALKOVIC__
		cout << "Running FordFuklerson between " << param->m_Center << " and " << v.first << endl;
#endif
		unsigned int result = Valkovic::FordFuklerson( linksMap, centerIndex, v.second );
		param->m_Redundancy.insert( pair<string, int>( v.first, (int)result ) );
	}
#ifdef __VALKOVIC__
	cout << "End of Redundancy problem" << endl;
#endif
}


void CSolver::Start( int threadCount )
{
#ifdef __VALKOVIC__
	cout << "Starting with " << threadCount << " threads" << endl;
#endif
	this->stoped = false;
	this->maxProblemsInQueue = threadCount;

	this->threads = new vector<thread*>( threadCount );
	for( int i = 0; i < threadCount; i++ )
		( *this->threads )[i] = new thread( WorkingThreadFn, this );
}

void CSolver::Stop( void )
{
	this->ended = true;

#ifdef __VALKOVIC__
	cout << "Stoping" << endl;
#endif 
	for( thread* t : this->clientsThreads )
		t->join();
	for( thread* t : this->clientsThreads )
		delete t;
	for( thread* t : *threads )
		t->join();
	for( thread* t : *threads )
		delete t;
	delete this->threads;
	stoped = true;
}

void CSolver::AddCustomer( shared_ptr<CCustomer> c )
{
	if( this->stoped )
		return;

#ifdef __VALKOVIC__
	cout << "New client arrive" << endl;
#endif

	clientsThreads.push_back( new thread( ClientCenterFn, this, c ) );
	clientsThreads.push_back( new thread( ClientRedundancyFn, this, c ) );

	this->clients.fetch_add( 2 );
}

void CSolver::WorkingThreadFn( CSolver* data )
{
	using Valkovic::RedundancyData;
	using Valkovic::CenterData;

	RedundancyData redProblem;
	CenterData centerProblem;
	bool loadedRedundancy;
	bool loadedCenter;
	bool obtainedTask = false;

	while( data->ended.load() == false || obtainedTask || data->clients.load() > 0 )
	{
		obtainedTask = false;
		//try to get redundancy problem
		loadedRedundancy = false;
		data->redundancyMutex.lock();
		if( (int)data->redundancy.size() > 0 )
		{
			redProblem = data->redundancy.front();
			data->redundancy.pop();
			loadedRedundancy = true;
		}
		data->redundancyMutex.unlock();
		if( loadedRedundancy )
		{
#ifdef __VALKOVIC__
			cout << "Going to solve redundancy problem" << endl;
#endif
			obtainedTask = true;
			Solve( redProblem.problem );
			redProblem.customer->Solved( redProblem.problem );
		}
		else
		{
			loadedCenter = false;
			//try to gen center problem
			data->centerMutex.lock();
			if( (int)data->center.size() > 0 )
			{
				centerProblem = data->center.front();
				data->center.pop();
				loadedCenter = true;
			}
			data->centerMutex.unlock();
			if( loadedCenter )
			{
#ifdef __VALKOVIC__
				cout << "Going to solve center problem" << endl;
#endif
				obtainedTask = true;
				Solve( centerProblem.problem );
				centerProblem.customer->Solved( centerProblem.problem );
			}
			else
				this_thread::yield();
		}
	}

#ifdef __VALKOVIC__
	cout << "Working thread ended" << endl;
#endif
}

void CSolver::ClientCenterFn( CSolver * data, shared_ptr<CCustomer> client )
{
	using Valkovic::CenterData;
	shared_ptr<CCenter> instance;
	bool added = false;
	while( (instance = client->GenCenter()) != NULL )
	{
#ifdef __VALKOVIC__
		cout << "Next problem of center arrive" << endl;
#endif
		added = false;
		while( !added )
		{
			data->centerMutex.lock();
			if( (int)data->center.size() < data->maxProblemsInQueue )
			{
				data->center.push( CenterData( instance, client ) );
				data->centerMutex.unlock();
				added = true;
			}
			else
			{
				data->centerMutex.unlock();
				this_thread::yield();
			}
		}
	}
#ifdef __VALKOVIC__
	cout << "Client ended with center problems" << endl;
#endif
	data->clients.fetch_sub( 1 );
}

void CSolver::ClientRedundancyFn( CSolver * data, shared_ptr<CCustomer> client )
{
	using Valkovic::RedundancyData;
	shared_ptr<CRedundancy> instance;
	bool added = false;
	while( (instance = client->GenRedundancy()) != NULL )
	{
#ifdef __VALKOVIC__
		cout << "Next problem of redundancy arrive" << endl;
#endif
		added = false;
		while( !added )
		{
			data->redundancyMutex.lock();
			if( (int)data->center.size() < data->maxProblemsInQueue )
			{
				data->redundancy.push( RedundancyData( instance, client ) );
				data->redundancyMutex.unlock();
				added = true;
			}
			else
			{
				data->redundancyMutex.unlock();
				this_thread::yield();
			}
		}
	}
#ifdef __VALKOVIC__
	cout << "Client ended with redundancy problems" << endl;
#endif
	data->clients.fetch_sub( 1 );
}

CSolver::CSolver( void ) : maxProblemsInQueue( -1 ), stoped( false ), ended( false ), clients(0)
{}

CSolver::~CSolver( void )
{
	if( !this->stoped )
		this->Stop();
}
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

	enum Problems
	{
		redundancy,
		center,
		quit
	};

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

	class ProblemData
	{
	public:
		Problems type;

		CenterData center;
		RedundancyData redundancy;

		ProblemData( shared_ptr<CCenter> problem, shared_ptr<CCustomer> customer )
			: type( Problems::center ), center( problem, customer ) {}

		ProblemData( shared_ptr<CRedundancy> problem, shared_ptr<CCustomer> customer )
			: type( Problems::redundancy ), redundancy( problem, customer ) {}

		ProblemData() = default;

		ProblemData( Problems p ) : type( p )
		{}
	};

	//SOURCE https://juanchopanzacpp.wordpress.com/2013/02/26/concurrent-queue-c11/
	class BlockingQueue
	{
	private:
		std::queue<ProblemData> queue_;
		std::mutex mutex_;
		std::condition_variable cond_;
	public:
		ProblemData pop()
		{
			std::unique_lock<std::mutex> mlock( mutex_ );
			while( queue_.empty() )
			{
				cond_.wait( mlock );
			}
			auto item = queue_.front();
			queue_.pop();
			return item;
		}

		void push( const ProblemData item )
		{
			std::unique_lock<std::mutex> mlock( mutex_ );
			queue_.push( item );
			mlock.unlock();
			cond_.notify_one();
		}
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
	vector<thread*> threads;
	vector<thread*> clientsThreads;

	static void WorkingThreadFn( CSolver* data );

	static void ClientCenterFn( CSolver* data, shared_ptr<CCustomer> client );
	static void ClientRedundancyFn( CSolver* data, shared_ptr<CCustomer> client );

	Valkovic::BlockingQueue queue;

	mutex clientsMutex;
	int clients;
	condition_variable clientsEnded;
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
	this->threads.resize( threadCount );
	this->clientsThreads.resize( 0 );

	for( int i = 0; i < threadCount; i++ )
		this->threads[i] = new thread( WorkingThreadFn, this );
}

void CSolver::Stop( void )
{
	//TODO wait to finish users
	unique_lock<mutex> clientsLock( this->clientsMutex );
	while( this->clients != 0 )
		clientsEnded.wait( clientsLock );
	clientsLock.unlock();
	//fill end command
	for( size_t i = 0, l = this->threads.size(); i < l; i++ )
		this->queue.push( Valkovic::ProblemData( Valkovic::Problems::quit ) );

#ifdef __VALKOVIC__
	cout << "Stoping" << endl;
#endif 
	for( thread* t : this->clientsThreads )
		t->join();
	for( thread* t : this->clientsThreads )
		delete t;
	for( thread* t : this->threads )
		t->join();
	for( thread* t : this->threads )
		delete t;
}

void CSolver::AddCustomer( shared_ptr<CCustomer> c )
{
#ifdef __VALKOVIC__
	cout << "New client arrive" << endl;
#endif
	this->clientsMutex.lock();
	this->clients += 2;
	this->clientsMutex.unlock();

	clientsThreads.push_back( new thread( ClientCenterFn, this, c ) );
	clientsThreads.push_back( new thread( ClientRedundancyFn, this, c ) );
}

void CSolver::WorkingThreadFn( CSolver* data )
{
	using Valkovic::ProblemData;
	using Valkovic::Problems;

	while( true )
	{
		ProblemData d = data->queue.pop();
		if( d.type == Problems::quit )
			break;
		else if( d.type == Problems::center )
		{
			Solve( d.center.problem );
			d.center.customer->Solved( d.center.problem );
		}
		else if( d.type == Problems::redundancy )
		{
			Solve( d.redundancy.problem );
			d.redundancy.customer->Solved( d.redundancy.problem );
		}
	}

#ifdef __VALKOVIC__
	cout << "Working thread ended" << endl;
#endif
}

void CSolver::ClientCenterFn( CSolver * data, shared_ptr<CCustomer> client )
{
	using Valkovic::ProblemData;
	shared_ptr<CCenter> instance;
	while( ( instance = client->GenCenter() ) != NULL )
	{
#ifdef __VALKOVIC__
		cout << "Next problem of center arrive" << endl;
#endif
		data->queue.push( ProblemData( instance, client ) );
	}
#ifdef __VALKOVIC__
	cout << "Client ended with center problems" << endl;
#endif
	data->clientsMutex.lock();
	data->clients--;
	if( data->clients == 0 )
		data->clientsEnded.notify_all();
	data->clientsMutex.unlock();
}

void CSolver::ClientRedundancyFn( CSolver * data, shared_ptr<CCustomer> client )
{
	using Valkovic::ProblemData;
	shared_ptr<CRedundancy> instance;
	while( ( instance = client->GenRedundancy() ) != NULL )
	{
#ifdef __VALKOVIC__
		cout << "Next problem of redundancy arrive" << endl;
#endif
		data->queue.push( ProblemData( instance, client ) );
	}
#ifdef __VALKOVIC__
	cout << "Client ended with redundancy problems" << endl;
#endif
	data->clientsMutex.lock();
	data->clients--;
	if( data->clients == 0 )
		data->clientsEnded.notify_all();
	data->clientsMutex.unlock();

}

CSolver::CSolver( void ) : clients( 0 )
{}

CSolver::~CSolver( void )
{
}
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
		unsigned int realFrom;

		Edge() : from( UINT_MAX ), to( UINT_MAX ), realFrom( UINT_MAX )
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

			return this->realFrom == UINT_MAX || this->realFrom != from;

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

		void Use( unsigned int from )
		{
#ifdef __VALKOVIC__
			if( !this->Can( from ) )
				throw new MyException();
#endif

			this->realFrom = from;
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


	uint64_t FordFuklerson( Vertex* begin, Vertex* end, vector<Vertex*> &allVertex )
	{
		while( true )
		{
			stack<unsigned int> toProccess;
			vector<bool> proccessed( allVertex.size(), false );
			vector<unsigned int> fromTo( allVertex.size(), UINT_MAX );
			//DFS
			toProccess.push( begin->index );
			while( !toProccess.empty() )
			{
				Vertex* cur = allVertex[toProccess.top()];

				if( cur->index == end->index )
					break;

				toProccess.pop();

				if( proccessed[cur->index] )
					continue;
				proccessed[cur->index] = true;

				for( pair<unsigned int, vector<Edge*>> edge : cur->edges )
				{
					if( proccessed[edge.first] )
						continue;
					for( Edge* e : edge.second )
					{
						if( e->Can( cur->index ) )
						{
							unsigned int to = e->To( cur->index );
							toProccess.push( to );
							fromTo[to] = cur->index;
						}
					}
				}
			}
			//reconstruct path
			if( fromTo[end->index] == UINT_MAX )
				break;
			else
			{
				unsigned int to = end->index;
				while( to != begin->index )
				{
					unsigned int from = fromTo[to];
					Edge* e = allVertex[from]->getEdge( to );
					e->Use( from );
					to = from;
				}
			}
		}
#ifdef __VALKOVIC__
		cout << "End of FordFuklerson" << endl;
#endif

		uint64_t paths = 0;
		for( pair<int, vector<Edge*>> edges : begin->edges )
			for( Edge* edge : edges.second )
				if( edge->realFrom == begin->index )
					paths++;

		return paths;

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
};




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

	//prepare structures
	vector<Edge*> edges;
	map<unsigned int, string> indexToName;
	map<string, unsigned int> nameToIndex;
	vector<Vertex*> vertexes;

	//fill structures
	for( CLink& x : param->m_Links )
	{

		if( nameToIndex.find( x.m_From ) == nameToIndex.end() )
		{
			indexToName.insert( pair<unsigned int, string>( vertexes.size(), x.m_From ) );
			nameToIndex.insert( pair<string, unsigned int>( x.m_From, vertexes.size() ) );
			vertexes.push_back( new Vertex( (unsigned int)vertexes.size() ) );
		}
		if( nameToIndex.find( x.m_To ) == nameToIndex.end() )
		{
			indexToName.insert( pair<unsigned int, string>( (unsigned int)vertexes.size(), x.m_To ) );
			nameToIndex.insert( pair<string, unsigned int>( x.m_To, (unsigned int)vertexes.size() ) );
			vertexes.push_back( new Vertex( (unsigned int)vertexes.size() ) );
		}


		Vertex* vertexFrom = vertexes[nameToIndex[x.m_From]];
		Vertex* vertexTo = vertexes[nameToIndex[x.m_To]];
		Edge* cur = new Edge( vertexFrom->index, vertexTo->index );
		edges.push_back( cur );
		if( vertexFrom->edges.find( vertexTo->index ) == vertexFrom->edges.end() )
			vertexFrom->edges.insert( pair<int, vector<Edge*>>( vertexTo->index, vector<Edge*>() ) );
		if( vertexTo->edges.find( vertexFrom->index ) == vertexTo->edges.end() )
			vertexTo->edges.insert( pair<int, vector<Edge*>>( vertexFrom->index, vector<Edge*>() ) );

		vertexFrom->edges[vertexTo->index].push_back( cur );
		vertexTo->edges[vertexFrom->index].push_back( cur );
	}

#ifdef __VALKOVIC__
	cout << "Links" << endl;
	for( Vertex* v : vertexes )
	{
		cout << v->index << endl;
		for( pair<int, vector<Edge*>> e : v->edges )
			cout << "\t - " << e.first << ":" << e.second.size() << endl;
	}
#endif

	//find center index
	unsigned int centerIndex = nameToIndex[centerName];
	Vertex* centerVertex = vertexes[centerIndex];

	//for each vertex except center apply FordFulkerson
	for( Vertex* v : vertexes )
	{
		if( v->index == centerIndex )
			continue;

		//reset capacity
		for( Edge* e : edges )
			e->realFrom = UINT_MAX;

#ifdef __VALKOVIC__
		cout << "Running FordFuklerson between " << centerName << " and " << indexToName[v->index] << endl;
#endif

		//FordFuklerson
		uint64_t result = Valkovic::FordFuklerson( centerVertex, v, vertexes );

#ifdef __VALKOVIC__
		cout << "Founded " << result << " paths to " << indexToName[v->index] << endl;
#endif
		param->m_Redundancy.insert( pair<string, int>( indexToName[v->index], (int)result ) );

	}

	for( Edge* e : edges )
		delete e;
	for( Vertex* v : vertexes )
		delete v;
}


void CSolver::Start( int threadCount )
{
	this->stoped = false;
	this->maxProblemsInQueue = threadCount;

	this->threads = new vector<thread*>( threadCount );
	for( int i = 0; i < threadCount; i++ )
		( *this->threads )[i] = new thread( WorkingThreadFn, this );
}

void CSolver::Stop( void )
{
	for( thread* t : this->clientsThreads )
		t->join();
	for( thread* t : *threads )
		t->join();
	for( thread* t : *threads )
		delete t;
	stoped = true;
}

void CSolver::AddCustomer( shared_ptr<CCustomer> c )
{
	if( this->stoped )
		return;

	clientsThreads.push_back( new thread( ClientCenterFn, this, c ) );
	clientsThreads.push_back( new thread( ClientRedundancyFn, this, c ) );
}

void CSolver::WorkingThreadFn( CSolver* data )
{
	using Valkovic::RedundancyData;
	using Valkovic::CenterData;

	RedundancyData redProblem;
	CenterData centerProblem;
	bool loadedRedundancy;
	bool loadedCenter;

	shared_ptr<CCenter> centerProblem;
	while( true ) //TODO add end condition
	{
		//try to get redundancy problem
		loadedRedundancy = false;
		data->redundancyMutex.lock();
		if( data->redundancy.size() > 0 )
		{
			redProblem = data->redundancy.front();
			data->redundancy.pop();
			loadedRedundancy = true;
		}
		data->redundancyMutex.unlock();
		if( loadedRedundancy )
		{
			Solve( redProblem.problem );
			redProblem.customer->Solved( redProblem.problem );
		}
		else
		{
			loadedCenter = false;
			//try to gen center problem
			data->centerMutex.lock();
			if( data->center.size() > 0 )
			{
				centerProblem = data->center.front();
				data->center.pop();
				loadedCenter = true;
			}
			data->centerMutex.unlock();
			if( loadedCenter )
			{
				Solve( centerProblem.problem );
				centerProblem.customer->Solved( centerProblem.problem );
			}
			else
				this_thread::yield();
		}
	}
}

void CSolver::ClientCenterFn( CSolver * data, shared_ptr<CCustomer> client )
{
	using Valkovic::CenterData;
	shared_ptr<CCenter> instance;
	bool added = false;
	while( instance = client->GenCenter() )
	{
#ifdef __VALKOVIC__
		cout << "Next problem of center arrive" << endl;
#endif
		added = false;
		while( !added )
		{
			data->centerMutex.lock();
			if( data->center.size() < data->maxProblemsInQueue )
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
}

void CSolver::ClientRedundancyFn( CSolver * data, shared_ptr<CCustomer> client )
{
	using Valkovic::RedundancyData;
	shared_ptr<CRedundancy> instance;
	bool added = false;
	while( instance = client->GenRedundancy() )
	{
#ifdef __VALKOVIC__
		cout << "Next problem of redundancy arrive" << endl;
#endif
		added = false;
		while( !added )
		{
			data->redundancyMutex.lock();
			if( data->center.size() < data->maxProblemsInQueue )
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
}

CSolver::CSolver( void )
{}

CSolver::~CSolver( void )
{
	if( !this->stoped )
		this->Stop();
}
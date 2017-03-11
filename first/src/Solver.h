#ifndef __PROGTEST__
#include "devs.h"
#endif

class CSolver
{
public:
	static void Solve( shared_ptr<CCenter> x )
	{
		int countOfIndexes = 0;
		map<string, int> indexes;
		vector<vector<double>> latency;


		//fill 2d array
		for( CLink link : x->m_Links )
		{
			//add vector if required
			if( indexes.find( link.m_From ) == indexes.end() )
			{
				indexes.insert( pair<string, int>( link.m_From, countOfIndexes ) );
				latency.push_back( vector<double>() );
				countOfIndexes++;
			}
			if( indexes.find( link.m_To ) == indexes.end() )
			{
				indexes.insert( pair<string, int>( link.m_To, countOfIndexes ) );
				latency.push_back( vector<double>() );
				countOfIndexes++;
			}
			//get indexes
			int indexFrom = indexes[link.m_From];
			int indexTo = indexes[link.m_To];
			//resize vectors if required
			if( (int)latency[indexFrom].size() <= indexTo )
			{
				int oldSize = latency[indexFrom].size();
				int inserted = countOfIndexes - oldSize;
				latency[indexFrom].resize( countOfIndexes );
				for( int i = 0; i < inserted; i++ )
					latency[indexFrom][oldSize + i] = numeric_limits<double>::max();
			}
			if( (int)latency[indexTo].size() <= indexFrom )
			{
				int oldSize = latency[indexTo].size();
				int inserted = countOfIndexes - oldSize;
				latency[indexTo].resize( countOfIndexes );
				for( int i = 0; i < inserted; i++ )
					latency[indexTo][oldSize + i] = numeric_limits<double>::max();
			}
			if( latency[indexFrom][indexTo] > link.m_Delay )
				latency[indexFrom][indexTo] = link.m_Delay;
			if( latency[indexTo][indexFrom] > link.m_Delay )
				latency[indexTo][indexFrom] = link.m_Delay;
		}

		//resize vectors
		for( int i = 0; i < countOfIndexes; i++ )
		{
			unsigned int oldSize = latency[i].size();
			latency[i].resize( countOfIndexes - 1 );
			for(int j=0,set=countOfIndexes - oldSize;j < set; j++ )
				latency[i][oldSize + j] = numeric_limits<double>::max();
			latency[i][i] = 0;
		}

#ifndef __PROGTEST__
		cout << "Vertex" << endl;
		for( auto v : indexes )
			cout << v.first << " : " << v.second << endl;
		cout << endl << "Before floyd" << endl;

		for( int i = 0; i < countOfIndexes; i++, cout << endl )
			for( int j = 0; j < countOfIndexes; j++ )
				cout << latency[i][j] << " ";
#endif


		//FloydWarshal
		for( int i = 0; i < countOfIndexes; i++ )
			for( int j = 0; j < countOfIndexes; j++ )
				for( int k = 0; k < countOfIndexes; k++ )
					if( latency[j][k] > latency[j][i] + latency[i][k] )
						latency[j][k] = latency[j][i] + latency[i][k];

#ifndef __PROGTEST__
		cout << endl << "After floyd" << endl;
		for( int i = 0; i < countOfIndexes; i++, cout << endl )
			for( int j = 0; j < countOfIndexes; j++ )
				cout << latency[i][j] << " ";
#endif

		//find lowest
		double lowestNumber = numeric_limits<double>::max();
		int index = -1;
		for( int i = 0; i < countOfIndexes; i++ )
		{
			double max = numeric_limits<double>::min();
			for( int j = 0; j < countOfIndexes; j++ )
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
				x->m_Center = c.first;
			}
			else
			{
				x->m_Delays.insert(pair<string,double>(c.first,latency[index][c.second]));
			}
		x->m_MaxDelay = lowestNumber;

#ifndef __PROGTEST__
		cout << "Nejmensi node " << x->m_Center << " s delay " << lowestNumber << endl << endl << endl << endl;
#endif

		return;
	}

	static void Solve( shared_ptr<CRedundancy> x )
	{}

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
	// todo
};

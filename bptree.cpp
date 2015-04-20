#include<bits/stdc++.h>
using namespace std;
#define pb push_back
#define iter make_move_iterator
string FOLDER ="temp/";
string QUERY_FILE="querysample.txt";
string INITIAL_FILE="assgn2_bplus_data.txt";
int MAX_KEYS =-1, nodeNoGlobalCounter = 1, objectNoGlobal = 1,Root = 1;
long long int pointQuery=0,insertQuery=0,rangeQuery=0,invalidQuery=0, diskReads=0,diskWrites=0,diskReadsQueries=0, diskWritesQueries=0;
bool isFirst=true;
map<int,int> parent;
vector<double> statsInsert,statsPoint,statsange;
clock_t startTime,endTime;

class BPTree
{
	public:

		int leaf;
		int nodeNo;
		vector <double> keys;
		vector <int> children;	
		int previous;
		int next;


		BPTree(int tempNodeNo,int mleaf=1)
		{
			nodeNo = tempNodeNo;
			leaf = mleaf;
			previous = -1;
			next = -1;

		}

		BPTree (int tempNodeNo, vector<double> &tempKeys, vector<int> &tempChildren)
		{
			leaf = 1;
			nodeNo = tempNodeNo;
			keys = tempKeys;
			children = tempChildren;
			previous = -1;
			next = -1;
		}		

		BPTree(int tempNodeNo,int mleaf,vector <double> &tempKeys, vector <int> &tempChildren, int tempPrevious, int tempNext)
		{
			nodeNo = tempNodeNo;
			leaf = mleaf;
			keys = tempKeys;
			children = tempChildren;
			previous = tempPrevious;
			next = tempNext;
		}

};

void Statistics(string header,vector<double> &timeVector)
{
	auto sum = accumulate(timeVector.begin(), timeVector.end(), 0.0);
	auto mean = sum / timeVector.size();
	auto squaredSum = inner_product(timeVector.begin(), timeVector.end(),timeVector.begin(), 0.0);
	auto standardDev = sqrt(squaredSum / timeVector.size() - mean * mean);
	cout<<header<<" : "<<"\n";
	cout<<"\t\t(1). Minimum :\t"<<*min_element(timeVector.begin(),timeVector.end())<<"\n";
	cout<<"\t\t(2). Maximum Time :\t "<<*max_element(timeVector.begin(),timeVector.end())<<"\n";
	cout<<"\t\t(3). Average Time :\t"<<mean<<"\n";
	cout<<"\t\t(4). Standard Deviation :\t "<<standardDev<<"\n";
}

void pointRead(int fname)
	{
		string line;
		ifstream in(FOLDER+"data_"+to_string(fname));

		while(getline(in,line))
			cout<<line<<"\n";
		in.close();
	}

void rangeRead(vector<int> &files)
		{

			for(int i=0;i<files.size();i++)
				pointRead(files[i]);
		}

int NewObjectFile(string data)
	{			
		int fname=objectNoGlobal;
		objectNoGlobal++;
		string inPath=FOLDER+"data_"+to_string(fname);
		ofstream out(inPath);
		out<<data;
		out.close();
		return fname;
	}

void Write(BPTree *tree)
	{
		
		/* File Structure
		Info if its a leaf
		totalSize
		followed by the keys
		No of children
		Followed by children
		if leaf previous
		if leaf next
		*/
		ofstream out(FOLDER+to_string(tree->nodeNo));	
		out.write((char*) &(tree->leaf),sizeof(tree->leaf));
		int tempSize = tree->keys.size(),i=0;
		out.write((char*) &tempSize,sizeof(tempSize));
		
		for(i=0;i<tempSize;i++)
			out.write((char *) &(tree->keys[i]),sizeof(tree->keys[i]));

		out.write((char*) &(tree->previous),sizeof(tree->previous));	
		out.write((char*) &(tree->next),sizeof(tree->next));	
		
		tempSize = tree->children.size();			
		out.write((char*) &tempSize,sizeof(tempSize));
		
		for(i=0;i<tempSize;i++)
			out.write((char *) &(tree->children[i]),sizeof(tree->children[i]));

		out.close();
		diskWrites++;
		diskWritesQueries++;
	}



BPTree* Read(string inPath, int fName)
	{	
		/* File Structure
		Info if its a leaf
		KeySize
		followed by the keys
		if leaf previous
		if leaf next		
		No of children
		Followed by children

		*/
		
		vector<double> keys;
		vector<int> children;
		int leaf,previous,next,child,a;
		double key=0;

		ifstream look(inPath);
		int tempSize,i;
		look.read((char*) &(leaf),sizeof(leaf));
		look.read((char*) &tempSize,sizeof(tempSize));
		for(i=0 ; i < tempSize ; i++ )
		{
			look.read((char *) &key,sizeof(key));
			keys.pb(key);
		}
		look.read((char*) &(previous),sizeof(previous));	
		look.read((char*) &(next),sizeof(next));
		
		look.read((char*) &tempSize,sizeof(tempSize));
		for(i=0 ; i < tempSize ; i++ )
		{
			look.read((char *) &child,sizeof(child));
			children.pb(child);
		}

		look.close();
		auto node = new BPTree(fName,leaf,keys,children,previous,next);	
		diskReads++;
		diskReadsQueries++;
		return node;

	}

BPTree* Find(double key)	
	{
		
		auto current = Read(FOLDER+to_string(Root), Root);
		int nodeName=0,i=0;
		while(!current->leaf)
		{
			
			for(i=0;i < current->keys.size();i++)
				if(current->keys[i] > key) 
					break;
			
			nodeName = current->children[i];
			parent[nodeName] = current->nodeNo;
			delete current;
			current = Read(FOLDER+to_string(nodeName),nodeName);
		}
		
		return current;
		
	}

void insertLeaf(BPTree* leafNode,double Value,int obj)
	{
		auto insertKey=leafNode->keys.begin();
		auto insertChildren = leafNode->children.begin();
		if(Value >= leafNode->keys[0])
		{
			int vSize = leafNode->keys.size(),index=0;
			for(index=index;index<vSize-1;index++)
				if(leafNode->keys[index+1] > Value)	
					break;
			insertKey+=index+1;
			insertChildren+=index+1;

		}
			leafNode->keys.insert(insertKey,Value);	
			leafNode->children.insert(insertChildren,obj);	
	}

void insertParent(BPTree *current,double Value, int obj)
	{
		if(current->nodeNo == Root)
		{
			auto *tempNode = new BPTree(nodeNoGlobalCounter,0);
			Root = nodeNoGlobalCounter ;
			nodeNoGlobalCounter++;
			tempNode->children.pb(current->nodeNo);
			tempNode->children.pb(obj);
			tempNode->keys.pb(Value);
			Write(tempNode);
			delete tempNode;
			delete current;
		}
		else
		{

			auto tempParent= parent[current->nodeNo];
			auto nodeNo= current->nodeNo;
			delete current;
			current = Read(FOLDER+to_string(tempParent),tempParent);
			bool done=false;
			for(int i=0;i<current->children.size();i++)
			{
				if(current->children[i] == nodeNo)
				{
					current->children.insert(current->children.begin() + i + 1, obj);
					current->keys.insert(current->keys.begin() + i , Value);
					done = true;
					break;
				}
			}
			
			if(current->keys.size() <= MAX_KEYS)
			{
				Write(current);
				delete current;
			}
			else
			{
				
				auto tempNode = new BPTree(nodeNoGlobalCounter,0);
				nodeNoGlobalCounter++;

				vector<int> children(iter(current->children.begin() + (current->children.size()+1)/2 ), iter(current->children.end()));
				current->children.erase(current->children.begin() + (current->children.size()+1)/2 , current->children.end());
				tempNode->children = children;
				children.clear();

				auto nKey = current->keys[current->keys.size()/2];

				vector<double> keys(iter(current->keys.begin() + current->keys.size()/2+1 ), iter(current->keys.end()));
				current->keys.erase(current->keys.begin() + (current->keys.size())/2 , current->keys.end());
				tempNode->keys = keys;
				keys.clear();
				Write(tempNode);
				Write(current);
				auto nObj = tempNode->nodeNo;
				delete tempNode;
				insertParent(current,nKey,nObj);
			}
		}
	}

void Insert(double Value,int obj)
	{	
		parent.clear();
		
		if(isFirst)
		{
			auto current = new BPTree(nodeNoGlobalCounter);
			Root = nodeNoGlobalCounter;
			nodeNoGlobalCounter++;						
			current->children.pb(obj);	
			current->keys.pb(Value);					
			Write(current);
			delete current;
			isFirst=false;
		}
		else
		{
			auto current = Find(Value);		
			if(current->keys.size() >= MAX_KEYS)
			{
				
				insertLeaf(current,Value,obj);
				auto tempNode = new BPTree(nodeNoGlobalCounter,1);
				nodeNoGlobalCounter++;
				
				tempNode->previous = current->nodeNo;

				if(current->next != -1)
				{
					auto temp = Read(FOLDER+to_string(current->next),current->next);
					temp->previous = tempNode->nodeNo;
					Write(temp);
					delete temp;
				}


				vector<int> children(iter(current->children.begin() + (current->children.size()+1)/2 ), iter(current->children.end()));
				current->children.erase(current->children.begin() + (current->children.size()+1)/2 , current->children.end());
				tempNode->children = children;
				children.clear();

				vector<double> keys(iter(current->keys.begin() + (current->keys.size()+1)/2 ), iter(current->keys.end()));
				current->keys.erase(current->keys.begin() + (current->keys.size()+1)/2 , current->keys.end());
				tempNode->keys = keys;

				tempNode->next = current->next;
				current->next = tempNode->nodeNo;				keys.clear();


				Write(tempNode);
				auto newObject = tempNode->nodeNo;
				auto newValue = tempNode->keys[0];
				delete tempNode;
				Write(current);
				insertParent(current,newValue,newObject);

			}
			else
			{	

				insertLeaf(current,Value,obj);
				Write(current);
				delete current;

			}		
		}
	}

int PointQuery(double key)
	{
		auto current = Find(key);
		bool found = false;
		int i=0;
		for(i=0;i < current->keys.size();i++)
		{
			if(current->keys[i] == key)
			{
				found = true;
				break;
			}
		}
		if(found)
		{
			auto res = current->children[i];
			delete current;
			return res;
		}
		delete current;
		return 0;
	}

void RangeQuery(double point,double range, vector<int> &result)
	{

		auto current = Find(point+range);
		BPTree *next=NULL;
		bool done = false;
		auto lower=point-range;
		auto higher=point+range;
		while(!done)
		{	
			for(auto temp=current->keys.rbegin();temp!=current->keys.rend();temp++)
			{
				if( *temp < lower)
					{
						done=true;
						break;
					}
				if(*temp <= higher)
					result.pb(current->children[(current->keys.rend()-temp-1)]);
			}
			if(!done)
			{
				if(current->previous == -1) 	break;
				next = Read(FOLDER+to_string(current->previous),current->previous);
				delete current;
				current = next;
			}
			else 
				break;
		}
		delete current;
		return;
	}


int main()
 {	
	ifstream config("bplustree.config");
	config>>MAX_KEYS;
	config.close();


	printf("Max_Keys=%d\n",MAX_KEYS);

	ifstream iFile(INITIAL_FILE);
	if(!iFile.is_open())
	{
		cout<<"Initialization of the tree should be done from "<<INITIAL_FILE<<" : file \n";
		exit(0);
	}

	int qCode,inserted=0;
	double key;
	string data;

	cout<<"Tree Creation : In Process .....................\n";
	while (true) 
	{
		    iFile >> key;
		    if( iFile.eof() ) break;
		    iFile >> data;
			startTime = clock();
			Insert(key,NewObjectFile(to_string(key)+"\t"+data));
			endTime = clock();
			statsInsert.pb( double(endTime-startTime) / CLOCKS_PER_SEC );
			inserted++;
	}
	iFile.close();
	cout<<"Tree Creation : Success ..........................\n";
	cout<<"Queries processing from file : querysample.txt \n";

	ifstream queries(QUERY_FILE);
	if(!queries.is_open())
	{
		cout<<"Queries should be read from : "<<QUERY_FILE<<" : file \n";
		exit(0);
	}
	diskWritesQueries=0;
	diskReadsQueries=0;
	while(true){
		queries >> qCode;
		double pt1;
		if(queries.eof()) break;
		queries >> pt1;
			if(qCode == 0)
				{
					queries>>data;
					startTime = clock();
					Insert(pt1,NewObjectFile(to_string(pt1)+"\t"+data));
					endTime = clock();
					statsInsert.pb( double(endTime-startTime) / CLOCKS_PER_SEC );
					insertQuery++;
					cout<<"Inserted"<<"\n";
				}
				else if(qCode == 1)
				{
					startTime = clock();
					auto temp = PointQuery(pt1);
					endTime = clock();
					statsPoint.pb( double(endTime-startTime) / CLOCKS_PER_SEC );
					if(temp)
					{
						cout<<pt1<<" : Found"<<"\n";
						pointRead(temp);
					}
					else
						{	
						cout<<"Not Found"<<"\n";
						printf("%d\t%.6lf\n",qCode,pt1);
					}
					pointQuery++;
				}
				else if(qCode == 2)
				{
					double Range;
					queries >> Range;
					cout<<"Range Query:"<<"\n";
					printf("Point : %.6f Range : %.6f\n",pt1,Range);
					vector<int> result;
					startTime = clock();

					RangeQuery(pt1,Range,result);
					rangeRead(result);
					endTime = clock();
					statsange.pb( double(endTime-startTime) / CLOCKS_PER_SEC );
					rangeQuery++;
				}
				else 
				{
					printf("Query Code %d not supported\n",qCode);
					invalidQuery++;
				}
				cout<<"\n";


	}

	

	cout<<"STATISTICS"<<"\n";
	
	Statistics("\tInsertion Queries",statsInsert);
	cout<<"\t\t\t\tNumber of insertion Queries : "<<insertQuery<<"\n";
	Statistics("\tPoint Queries",statsPoint);
	cout<<"\t\t\t\tNumber of point Queries : "<<pointQuery<<"\n";
	Statistics("\tRange Queries",statsange);
	cout<<"\t\t\t\tNumber of range Queries : "<<rangeQuery<<"\n";
	cout<<"\n\tNumber of invalid Queries : "<<invalidQuery<<"\n";
	cout<<"\n\n\t\t\tTotal Number of disks Reads : "<<diskReads<<"\n";
	cout<<"\t\t\tTotal Number of disks Writes : "<<diskWrites<<"\n";
	cout<<"\t\t\tTotal Number of disk Access : "<<diskReads+diskWrites<<"\n";
	cout<<"\t\t\tNumber of disks Reads in queries : "<<diskReadsQueries<<"\n";
	cout<<"\t\t\tNumber of disks Writes in queries : "<<diskWritesQueries<<"\n";
	cout<<"\t\t\tNumber of disk Access in queries : "<<diskReadsQueries+diskWritesQueries<<"\n";
    return 0;
}
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>


using namespace std;

stringstream report;
int strtoi(string s){
	int x = atoi(s.c_str());
	return x;
}

string inttostr(int number)
{
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

class LS{
	private: 
		
		int indexof(int *a,int c){
			int res=-1;
			for (int k=0;k<order;k++){
				if (a[k]==c){res=k;break;}
			}
			return res;
		}
		int* lselems;
		int order;
		int &item(int i, int j);	
		int * row (int i);
		int * column (int j);
		int * maindiag();
		int * subdiag();
		bool checkonlyone(int * a);
		void print (int *a){
			cout <<endl;
			for (int i=0;i<order;i++){
				cout <<a[i]<<" ";
			}
			cout<<endl;
		}
		bool checkrow(int i)
		{
			int * t=row(i);
			return checkonlyone(t);
		}
		bool checkcolumn(int j){
			int * t=column(j);
			return checkonlyone(t);
		}
		bool checkmaindiag(){
			int * t=maindiag();
			return checkonlyone(t);
		}
		bool checksubdiag(){
			int * t=subdiag();
			
			return checkonlyone(t);
		}

		void transprows(int i1, int i2);
		void transpcolumns(int j1, int j2);
	public:
		
		LS (string s,int n);
		LS (int *ss,int n);
		int it(int i,int j);
		bool parse(int * ss);
		bool check (bool diag);	
		void normalizefirstrow();		
		void normalizefirstcolumn();	
		void reorder();
		string tostring();

};
LS::LS(int * b, int n){
	lselems = b;
	order=n;
	cout<<endl;
	report<<endl;
	for (int i=0;i<order;i++){
		for (int j=0;j<order;j++){
			cout<<b[i*order+j]<<" ";
			report<<b[i*order+j]<<" ";
		}
		cout<<endl;
		report<<endl;
	}
}
LS::LS(string s,int n){
	order=n;
	int *a=new (int[n*n*n]); 	
	int *b=new (int[n*n]);
	string t;
	int i1=0; 
	int i2=0;
	int k=0;
	int g=0;
	for (int i=0;i<s.length();i++){
		if (s[i]==' ')
		{
			i2=i;
			t=s.substr(i1,i2-i1);
			//cout <<t<<" ";
			a[k]=strtoi(t);
			if (a[k]>0) {/*cout<<k%n<< " ";*/b[g]=k%n;g++;}

			k++;
			//if ((k>0)&&((k%(n*n)==0))) cout<<endl;
			//if (k==n*n*n) cout <<endl<<endl;
			i1=i2;
		}		
	}	
	lselems=b;
}
string LS::tostring(){
	string a;
	for (int i=0;i<order;i++){
		for (int j=0;j<order;j++){
			a=a+inttostr(item(i,j))+' ';
		}
		a=a+'\n';
	}
	return a;
}
int * LS::row(int i)
{
	int * a;
	a=new (int[order]);
	for (int k=0;k<order;k++){
		a[k]=lselems[i*order+k];
	}
	return a;
}

int * LS::column(int j)
{
	int * a;
	a=new (int[order]);
	for (int k=0;k<order;k++){
		a[k]=lselems[k*order+j];
	}
	return a;
}
int * LS::maindiag()
{
	int * a;
	a=new (int[order]);
	for (int k=0;k<order;k++){
		a[k]=lselems[k*order+k];
	}
	return a;
}
int * LS::subdiag()
{
	int * a;
	a=new (int[order]);
	for (int k=0;k<order;k++){
		a[k]=lselems[k*order+order-k-1];
	}

	return a;
}

int &LS::item(int i,int j){
	return lselems[i*order+j];
}
int LS::it(int i,int j){
	int r=lselems[i*order+j];
	return r;
}

bool LS::checkonlyone(int * a){
	bool res=true;
	int *cc=new (int[order]);
	for (int k=0;k<order;k++){
		cc[k]=0;
	}
	for (int k=0;k<order;k++){
		cc[a[k]]++;
		if (cc[a[k]]>1) {res=false;}
	}
	return res;
}

void LS::transprows(int i1, int i2){
	int *t=row (i1);
	for (int k=0;k<order;k++){
		item(i1,k)=item(i2,k);
	}
	for (int k=0;k<order;k++){
		item(i2,k)=t[k];
	}
}

void LS::transpcolumns(int j1, int j2){
	int *t=column (j1);
	for (int k=0;k<order;k++){
		item(k,j1)=item(k,j2);
	}
	for (int k=0;k<order;k++){
		item(k,j2)=t[k];
	}
}
bool LS::check (bool diag){
	
	bool a1=true;
	for (int k=0;k<order;k++){
		if (checkrow(k)==false){
			a1=false;
			cout<<"row "<<k<<" fail"<<endl;
			report<<"row "<<k<<" fail"<<endl;
		}
	}
	bool a2=true;
	for (int k=0;k<order;k++){
		if (checkcolumn(k)==false){
			a1=false;
			cout<<"column "<<k<<" fail"<<endl;
			report<<"column "<<k<<" fail"<<endl;
		}
	}
	bool a3=true;
	if (diag==true){
		{
			if (checkmaindiag()==false){
				a3=false;
				cout<<"maindiag fail"<<endl;
				report<<"maindiag fail"<<endl;
			}
			if (checksubdiag()==false){
				a3=false;
				cout<<"subdiag fail"<<endl;
				report<<"subdiag fail"<<endl;
			}
		}
	}
	bool res=a1&&a2&&a3;
	return res;
}
void LS::normalizefirstrow(){
	int * r1=row(1);
	for (int k=0;k<order;k++){
			if (r1[k]!=k){
				int tt1=indexof(r1,k);
				transpcolumns(tt1,k);
				r1[tt1]=r1[k];
				r1[k]=k;
			}
		}
}
void LS::normalizefirstcolumn(){
	int * c1=column(0);
	for (int k=0;k<order;k++){
			if (c1[k]!=k){
				int tt1=indexof(c1,k);
				transprows(tt1,k);
				c1[tt1]=c1[k];
				c1[k]=k;
			}
		}
}
void LS::reorder(){
	int *r1=row(0);
	int *f=new(int[order]);
	for (int k=0;k<order;k++){
		f[r1[k]]=k;
	}
	print (r1);
	print (f);

	int *t=new (int [order*order]);
	for (int k=0;k<order*order;k++){
		t[k]=f[lselems[k]];
	//	cout <<endl<<"("<<lselems[k]<<")->"<<f[lselems[k]]<<endl;
	}	
	for (int k=0;k<order*order;k++){
		lselems[k]=t[k];
	}		
}
class MOLS {
private:
	int order;
	int count;
	bool checkort(int ia, int ib);
	int checkortinc(int ia, int ib);
public:
	
	vector<LS> Squares;
	MOLS();
	~MOLS();
	MOLS(string s,int n, int r,bool wspaces);
	void import(const char *fn, int n, int r);
	void setoutputfile (const char *fn);
	int ortogonalitycheck();
	int check(bool diag);
	void reorder();
	void print(const char * fn);	
};
void MOLS::reorder(){
	for (int i=0;i<count;i++){
		Squares[i].reorder();
	}
}
MOLS::MOLS()
{
	order=0;
	count=0;
}
MOLS::~MOLS(){
	//out.close();
}
void MOLS::setoutputfile (const char *fn){
	//out.open(fn,ios::app);
}

void MOLS::import(const char*fn, int n, int r){
  ifstream myfile (fn); 
  int *b;
  order=n; 
  count=r;
  b=new (int[n*n*r]);
  int sc=0;
  if (myfile.is_open())
  {	
	while (!myfile.eof())//( myfile.good() )
    {
	  string line="";
	  getline (myfile,line);
	  for (int i=0;i<line.length();i++){
		  if ((line[i]>='0')&&(line[i]<'0'+10)){
			  b[sc]=line[i]-'0'; 
			  sc++;
		  }
	  }
	}
	for (int k=0;k<r;k++){
		int *z=new (int [n*n]);
		for (int t=0;t<n*n;t++){
			z[t]=b[k*n*n+t];
		}
		LS h=LS(z,order);
		Squares.push_back(h);
	}
  }

}

bool MOLS::checkort(int ia, int ib){
	bool f=true;
	LS a=Squares[ia];
	LS b=Squares[ib];
	int * r=new (int[order*order]);
	for (int k=0;k<order*order;k++){r[k]=0;}
	for (int i=0;i<order;i++){
		for (int j=0;j<order;j++){
			int h=a.it(i,j)*order+b.it(i,j);
			r[h]++;
			if (r[h]>1){
				f=false; 
				cout<<"orthogonality check failed"<<endl;
				report<<"orthogonality check failed"<<endl;
			}
		}
	}
	return f;
}
int MOLS::checkortinc(int ia, int ib){
	LS a=Squares[ia];
	LS b=Squares[ib];
	int count=0;
	int * r=new (int[order*order]);
	for (int k=0;k<order*order;k++){r[k]=0;}
	for (int i=0;i<order;i++){
		for (int j=0;j<order;j++){
			int h=a.it(i,j)*order+b.it(i,j);
			r[h]++;
			if (r[h]==1){count++;}
		}
	}
	
	if (count<order*order){
		cout<<endl<<"Orthogonality Matrix for squares "<<ia+1<<" and "<<ib+1<<endl;
		report<<endl<<"Orthogonality Matrix for squares "<<ia+1<<" and "<<ib+1<<endl;
		for (int i=0;i<order;i++){
			for (int j=0;j<order;j++){
				cout<<r[i*order+j]<<" ";
				report<<r[i*order+j]<<" ";				
			}
			cout <<endl;
			report<<endl;
		}	
	}
	return count;
}
int MOLS::ortogonalitycheck(){
	int res=0;
	for (int i=0;i<count;i++){
		for(int j=i+1;j<count;j++){
			int r=checkortinc(i,j);
			if (r<order*order){			
				cout<<"Squares "<<i+1<<" and "<<j+1<<" are orthogonal in "<<r<<" entries"<<endl;
				report<<"Squares "<<i+1<<" and "<<j+1<<" are orthogonal in "<<r<<" entries"<<endl;
				if (res<r){res=r;}
			}
		}
	}
	return res;
}
int MOLS::check(bool diag){
	bool f = true;
	int r = 0;
	for (int i=0;i<count;i++){
		LS a=Squares[i];
		bool r=a.check(diag);
		if (r==false){
			f=false; 
			cout<<"square "<<i<<" is wrong"<<endl;
			report<<"square "<<i<<" is wrong"<<endl;
		}
	}
	
	if (f==true){
		r=ortogonalitycheck();
		return r;
	}
	else {
		return 0;
	}
}
MOLS::MOLS(string s, int n,int r, bool wspaces){
	int *a=new (int[n*n*n*r]); 	
	int *b=new (int[n*n*r]);
	string t;
	int i1=0; 
	int i2=0;
	int k=0;
	int g=0;
	if (wspaces==true){
	for (int i=0;i<s.size(), g<=n*n*r;i++){
			if (s[i]==' ')
			{
				i2=i;
				t=s.substr(i1,i2-i1);
				//cout <<t<<" ";
				a[k]=strtoi(t);
				if (a[k]>0) {/*cout<<k%n<< " ";*/b[g]=k%n;g++;
				cout<<endl<<g;}

				k++;
				//if ((k>0)&&((k%(n*n)==0))) cout<<endl;
				//if (k==n*n*n) cout <<endl<<endl;
				i1=i2;
			}		
			if (g>=n*n*r) break;
		}
	}
	else {
		for (int i=0;g<=n*n*r;i++){
			a[i]=s[i]-'0';
			if (a[i]>0) {/*cout<<k%n<< " ";*/b[g]=i%n;g++;}
			}		
		}

	order=n;
	count=r;
	for (int k=0;k<r;k++){
		int *z=new (int [n*n]);
		for (int t=0;t<n*n;t++){
			z[t]=b[k*n*n+t];
		}
		LS h=LS(z,order);
		Squares.push_back(h);
	}
}
void MOLS::print( const char * fn){
	ofstream myfile (fn);
	if (myfile.is_open()){
		for (int k=0;k<count;k++){
			myfile<<Squares[k].tostring()<<'\n';
			myfile<<"reordered version:\n";
			Squares[k].reorder();
			myfile<<Squares[k].tostring()<<'\n';
		}
	}
}
bool eq(LS a, LS b, int order){
	bool res=true;
	for (int i=0;i<order;i++){
		for (int j=0;j<order;j++){
			if (a.it(i,j)!=b.it(i,j)){res=false;}
		}
	}
	return res;
}
struct ijk{
	int sq;
	int i;
	int j;
	int k;
	int phase;
	
};

struct less_than_ijk
{
    inline bool operator() (const ijk& st1, const ijk& st2)
    {
        bool r;
		if (st1.sq<st2.sq){r=true;} 
		else{
			if (st1.sq>st2.sq) {r=false;}
			else { 
				if (st1.i<st2.i){r=true;}
				else if (st1.i>st2.i){r=false;}
				else {
				if (st1.j<st2.j){r=true;}
				else if (st1.j>st2.j){r=false;}
				else{
					if (st1.k<st2.k){r=true;}
					else if (st1.k>=st2.k){r=false;}
				}
			}
		}
	}
		return r;
    }
};

class constraint{
private:
	vector<int*> ivar;
	vector<int*> jvar;
	vector<int*> kvar;
	ijk inttoijk(int a);
	int order;
	vector<ijk> baseconstraint;
	
public:
	vector<vector<ijk>> generated;
	int length();
	void parse (string s, int n);
	void fromvect(vector<ijk> b,int n);
	void construct();
	void replicate(int index);
	string to_string();
	void print();
	void printgenerated();
	string toclause();
};
struct less_than_constraint
{
    inline bool operator() ( constraint& st1,  constraint& st2)
    {
		int l1=st1.length();
		
		int l2=st2.length();
		bool r=(l1<l2);
		return r;
    }
};
void constraint::fromvect(vector<ijk>b, int n){
	order=n;
	for (int j=0;j<n;j++){
		baseconstraint.push_back(b[j]);
	}
}
string constraint::toclause(){
	string s;
	for (int i=0;i<baseconstraint.size();i++){
		ijk a=baseconstraint[i];
		int r=a.sq*order*order*order+a.i*order*order+a.j*order+a.k;
		if (i>0) s=s+" ";
		if (a.phase>0) {s=s+inttostr(r);} else {s=s+inttostr(-r);}
	}
	s=s+"0";
	return s;
}
ijk constraint::inttoijk(int a){
	ijk r;
	if (a<0){r.phase=-1;a=-a;}else{r.phase=+1;}
	if (a>(order*order*order)){r.sq=a/(order*order*order);a=a-r.sq*order*order*order;} else{r.sq=0;}
	r.i=a/(order*order);
	r.j=(a-r.i*order*order)/(order);
	r.k=(a-r.i*order*order-r.j*order);
	return r;
}
void constraint::parse (string s, int n){
	string t;
	order=n;
	int i1=0; 
	int i2=0;
	for (int i=0;i<s.length();i++){
			if ((s[i]==' ')||(s[i]=='\n'))
			{
				i2=i;
				t=s.substr(i1,i2-i1);
				int f=strtoi(t);
				baseconstraint.push_back(inttoijk(f));
				i1=i2;
			}				
		}
	sort(baseconstraint.begin(),baseconstraint.end(),less_than_ijk());			
}

int constraint::length(){
	int r=baseconstraint.size();
	return r;
}
void constraint::print(){
	for (int i=0;i<baseconstraint.size();i++){
		ijk r=baseconstraint[i];
		cout<<endl;
		if (r.phase<0)cout<<"-";
		cout<<"("<<r.i<<", "<<r.j<<", "<<r.k<<", "<<r.sq<<")"<<endl;
	}
}
string constraint::to_string(){
	string s;
	for (int i=0;i<baseconstraint.size();i++){
		ijk r=baseconstraint[i];
		if (r.phase<0){s=s+'-';}
		s=s+"("+inttostr(r.i)+", "+inttostr(r.j)+", "+inttostr(r.k)+", "+inttostr(r.sq)+") ";
	}
	return s;
}
void constraint::replicate(int index)
{
	//(0, 5, 2, 0) (1, 5, 2, 0) (2, 5, 2, 0) (3, 5, 2, 0) (4, 5, 2, 0) (5, 5, 2, 0) -(7, 8, 2, 0) (8, 5, 2, 0) (9, 5, 2, 0) -(6, 5, 1, 1) -(7, 8, 1, 1) 

	ijk cur;
	cur=baseconstraint[index];
	int addind;
	if (index==0) addind=1;
	if (index==1) addind=0;
	if (index==2) addind=3;
	if (index==3) addind=2;
	ijk nb=baseconstraint[addind];
	vector <ijk> nc;		
	for (int i=0;i<order;i++){
		if ((i!=cur.i)&&(i!=nb.i)){
			ijk b;
			b.i=i;
			b.j=cur.j;
			b.k=cur.k;
			b.phase=1;
			b.sq=cur.sq;
			nc.push_back(b);
		}
	}
	for (int i=0;i<baseconstraint.size();i++){
		if (i!=index){nc.push_back(baseconstraint[i]);}
	}
	generated.push_back(nc);
	//column analogue
	vector<ijk> cc;
	for (int j=0;j<order;j++){
		if ((j!=cur.j)&&(j!=nb.j)){
			ijk b;
			b.i=cur.i;
			b.j=j;
			b.k=cur.k;
			b.phase=1;
			b.sq=cur.sq;
			cc.push_back(b);
		}
	}
	for (int i=0;i<baseconstraint.size();i++){
		if (i!=index){cc.push_back(baseconstraint[i]);}
	}
	generated.push_back(cc);
}
void constraint::printgenerated(){
	vector<ijk>a;
	for (int i=0;i<generated.size();i++){
		a=generated[i];
		cout<<endl;
		ijk b;
		for (int j=0;j<a.size();j++){
			b=a[j];
			if (b.phase<0)cout<<"-";
			cout<<"("<<b.i<<", "<<b.j<<", "<<b.k<<", "<<b.sq<<")"<<endl;
		}

	}

}
int ijktov(int i, int j, int k, int n){
	
	return i*n*n+j*n+k+1;
}
int tvi (int t, int i, int j, int mod, int n){

	return n*n*(3*t+mod)+i*n+j+1;
}

int main ( int argc, char **argv ) {
  int n=10;
  int r=3;
#ifdef _DEBUG
	argc = 3;
	argv[1] = "sat_sets.txt";
	argv[2] = "out.txt";
#endif
  if ( argc < 3 ) {
	  cerr << "Usage: program sat_sets_file out_file" << endl;
	  return 1;
  }
  char * myfnin = argv[1];
  ifstream myfile(myfnin);
  if ( !myfile.is_open() ) {
	  cerr << "!myfile.is_open()" << endl;
	  return 1;
  }
  string fntmp = argv[2];
  int sc=0;
  vector <LS> al;
  int * statistics;
  statistics=new (int[n*n]);
  for (int i=0;i<n*n;i++){
	  statistics[i]=0;
  }
  vector<int> timestats;
  while (myfile.good()){
	  string line="";	  
      getline (myfile,line);
	  if (line.size()>0){
		  sc++;
		  report<<"----------------------------------------------------";
		  report<<endl<<"Test No. "<<sc<<endl;
		  MOLS a=MOLS(line,n,r,false);
		  int b=a.check(false);
		  timestats.push_back(b);
		  statistics[b]++;
		  cout <<"b= "<<b<<endl;
		  report<<endl;
	  }		
  }

  cout<<"Time statistics "<<endl;
  for (int i=0;i<timestats.size();i++){
	  if (timestats[i]>10){
		  cout<<"Test No. "<<i+1 <<" "<<timestats[i]<<endl;
		  report<<"Test No. "<<i+1 <<" "<<timestats[i]<<endl;
		}
  }


  cout<<"Final statistics "<<endl;
  for (int i=0;i<n*n;i++){
	  if (statistics[i]>0){
		cout<<"Amount of pairs with "<< i << " common entries is "<< statistics[i]<<endl;
		report<<"Amount of pairs with "<< i << " common entries is "<< statistics[i]<<endl;
	  }
  }
  ofstream out;
  out.open(fntmp.c_str());
  out<<report.rdbuf();
  out.close();
  system("pause");
}

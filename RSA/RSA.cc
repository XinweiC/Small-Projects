// Implementation of the RSA public-key encryption algorithm
// ECE4893/ECE8893, Fall 2012

#include "gmpxx.h"
#include <stdlib.h>
#include<iostream>
using namespace std;
void generateKey();
mpz_class GCD(mpz_class a, mpz_class b);
void breakRsa(int num);
mpz_class d=0, fn=0,n=0, p=0,q=0;
mpz_t e;
int sz=32;
mpz_t cipherText, plainText;

int main()
{
  // Your code here
  long i=0;
  for(sz=32;sz<=1024;sz=sz*2)
  {
	for(int j=0;j<10;j++)
	{
  		generateKey();
                for(int k=0;k<10;k++)
		{
  			mpz_init (cipherText);
  			mpz_init(plainText);
  			gmp_randclass  random(gmp_randinit_default);
  			random.seed(time(NULL)+i++);
  			mpz_class m = random.get_z_bits(sz);

  			mpz_powm(cipherText,m.get_mpz_t(),d.get_mpz_t(),n.get_mpz_t());
  			mpz_powm(plainText,cipherText,e,n.get_mpz_t());
  			if(mpz_cmp(m.get_mpz_t(),plainText)!=0)
			{
				printf("size:%d,keyRound:%d,Round:%d: not equel\n",sz,j,k);
				printf("%ld\n",mpz_get_ui(m.get_mpz_t()));
  				printf("%ld\n",mpz_get_ui(cipherText));
  				printf("%ld\n",mpz_get_ui(plainText));
				return 0;

			}
			if(j==0&&k==0)
			{
			cout << "mpz_class p" << sz << "(\"" << p << "\");" << endl;
			cout << "mpz_class q" << sz << "(\"" << q << "\");" << endl;
			cout << "mpz_class n" << sz << "(\"" << n << "\");" << endl;
			cout << "mpz_class d" << sz << "(\"" << d << "\");" << endl;
			cout << "mpz_class e" << sz << "(\"" << e << "\");" << endl;
			cout << "mpz_class M" << sz << "(\"" << plainText << "\");" << endl;
			cout << "mpz_class C" << sz << "(\"" << cipherText << "\");" << endl;
		
		if(sz==32)
		{
			//printf("begin\n");
			breakRsa(2);

		}
		}
}
		
	}
  }
  //printf("done, no error\n");
  
}
long i=0;

void generateKey()
{
	int size=sz;
	 d=0, fn=0,n=0, p=0,q=0;
		 while( mpz_probab_prime_p(p.get_mpz_t(),100)==0)
	{
         	gmp_randclass  random(gmp_randinit_default);
		random.seed(time(NULL)+i++);
        	p = random.get_z_bits(size);
	}
	 while( mpz_probab_prime_p(q.get_mpz_t(),100)==0)
	{
         	gmp_randclass  random(gmp_randinit_default);
		random.seed(time(NULL)+i++);
        	q = random.get_z_bits(size);
	}
	n=q*p;
	fn=(p-1)*(q-1);
	MP_INT * res = (MP_INT *)malloc(50);
	mpz_init(res);
	while(d>=fn||mpz_get_ui(res)!=1)
	{
		gmp_randclass  random(gmp_randinit_default);
		random.seed(time(NULL)+i++);
		d = random.get_z_bits(size*2);
		mpz_gcd(res,d.get_mpz_t(),fn.get_mpz_t());
	}
	mpz_invert(e,d.get_mpz_t(),fn.get_mpz_t());
	
}
void breakRsa(int num)
{
	mpz_class b,i,B,D,fixed;
	b=2;
	B=1;
	fixed=2;
	D=1;
	while(true)
	{
		B=B*2;
		fixed=b;
		i=1;

	for(;i<B;i++)
	{
		 b=(b*b+1)%n;
		D=b-fixed;
		mpz_gcd(D.get_mpz_t(), D.get_mpz_t(), n.get_mpz_t());

       	        if(D>1)
		break;

	}
       if(D>1)
	{
		mpz_class fn2=(D-1)*(n/D-1);
		mpz_t e2,breakText;
		mpz_init (e2);
       		mpz_init (breakText);
		mpz_invert(e2,d.get_mpz_t(),fn2.get_mpz_t());
		mpz_powm(breakText,cipherText,e2,n.get_mpz_t());
		//printf("%ld\n",(mpz_get_ui(D.get_mpz_t())));
		//printf("%ld\n",(mpz_get_ui(p.get_mpz_t())));
		//printf("%ld\n",(mpz_get_ui(q.get_mpz_t())));
		//printf("planText:%ld\n",mpz_get_ui(plainText));
		cout << "mpz_class p" << sz << "Attack(\"" << p << "\");" << endl;
                        cout << "mpz_class q" << sz << "Attack(\"" << q << "\");" << endl;
                        cout << "mpz_class n" << sz << "Attack(\"" << n << "\");" << endl;
                        cout << "mpz_class d" << sz << "Attack(\"" << d << "\");" << endl;
                        cout << "mpz_class e" << sz << "Attack(\"" << e << "\");" << endl;
                        cout << "mpz_class M" << sz << "Attack(\"" << breakText << "\");" << endl;
		break;
	}

	else
	{
	//printf("continue%ld\n",num++);
	
	}


	}

}



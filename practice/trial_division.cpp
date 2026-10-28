#include <iostream>
using namespace std ;

int main(){
    bool first = 1 ;
    int n = 0 ;
    cin >> n ;
    int p = 2 ;
    while(p * p <= n){
        if (n % p == 0){
            if (first){
                cout << n << "=" << p ;
                first = 0 ;
            }
            else {
                cout << "*" << p ;
            }
            n /= p ;
        }
        else p ++ ;
    }
    cout << "*" << p ;
}

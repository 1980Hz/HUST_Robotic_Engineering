#include <iostream>
#include <cmath>
#include <iomanip>
using namespace std ;

int main () {
    int x ;
    int max = 0 ;
    int sum = 0 ;
    int count = 0 ;
    
    for (cin >> x ; x > 0 ; cin >> x ){
        if (x > max){
            max = x ;
        }
        sum += x ;
        count ++ ;
    }

    if (count == 0){
        cout << 0 << endl ;
        cout << 0 << endl ;
    }

    else {
        cout << max << endl ;
        cout << fixed << setprecision(2) << sum / count << endl ;
    }
    
}
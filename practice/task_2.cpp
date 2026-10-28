#include <iostream>
using namespace std ;

int main () {
    int n , flag = 0;
    cout << "请输入一个正整数" ;
    cin >> n ;
    for (int i = 2 ; i < n ; i ++){
        if (n % i == 0){
            flag = 1 ;
            break;
        }
    }
    if (flag == 1)
    cout << n << "不是质数" ;
    else
    cout << n << "是质数" ;
}
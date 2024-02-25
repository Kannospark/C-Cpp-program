#include "matrix.h"
#include <iostream>
#include <memory>

using namespace std;

int main(){
    //矩阵运算正确性检验
    matrix<double> m1(1,2,3,2.0);
    matrix<double> mt(1,2,3,4);
    matrix m2(m1);
    matrix<float> m3(2,2,3,1);
    m1+=mt;
    cout << "m1: "<< m1 << endl;
    cout << "m2: "<< m2 << endl;
    cout << "m3: "<< m3 << endl;
    cout << "m1+m2:" << (m1+m2) << endl;
    string str;
    (m1 == m2) ? str = "equal" : str = "not equal";
    cout << "m1 == m2 " << str << endl;
    (m1 == m3) ? str = "equal" : str = "not equal";
    cout << "m1 == m3 " << str << endl;
    (m1 == (m1 + m2)) ? str = "equal" : str = "not equal";
    cout << "m1 == m1 + m2 " << str << endl;
    matrix<int> m4;
    m4 = (m1 + 1)*3 - 1 + m2/m3 + 1 ;
    cout << "m4" << m4 << endl;
    m1 = m2;  //检验soft copy
    m1 = m4;  //检验hard copy
    cout << "operator check!" << endl;
    
    // 卷积方法正确性实验
    matrix<unsigned char> m5(3,3,2,1);
    matrix<unsigned char> m7(3,2,2,1);
    matrix<unsigned char> result = m5.conv(m7,1,0);
    matrix<int> m6;
    m6 = result;
    cout << "output"<< m6 << endl;

    matrix<unsigned char> image("test_picture.bmp");
    matrix<unsigned char> kernel(3,1,1,1);
    matrix<unsigned char> output = image.conv(kernel,1,0);
    output.outputImage("output_image.bmp");
    return 0;
}
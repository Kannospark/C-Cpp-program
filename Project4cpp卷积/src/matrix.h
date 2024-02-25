#include <iostream>
#include <memory>
#include <stdio.h>
#include <string.h>

using namespace std;
#define byte unsigned char

//template U代表class里使用的类型
template <typename T>
class matrix{
    public:
    int row;
    int col;
    int channel;
    shared_ptr<T[]> ptr;

    matrix();
    matrix(const char*);
    matrix(int, int, int, T);
    matrix<T>(const matrix<T>&); //softcopy
    //赋值
    template<typename U>
    matrix<T>& operator=(const matrix<U>&); //不同类型hardcopy
    matrix<T>& operator=(const matrix<T>&); //同类型softcopy
    template<typename U>
    matrix<T>& operator=(U);
    //相等
    template<typename U>
    bool operator==(const matrix<U>&);
    //加法
    template <typename U>
    matrix<T> operator+(const matrix<U>&);
    template <typename U>
    matrix<T> operator+(U);
    template <typename U>
    matrix<T>& operator+=(matrix<U>&);
    template <typename U>
    matrix<T>& operator+=(U);
    //减法
    template<typename U>
    matrix<T> operator-(const matrix<U>&);
    template<typename U>
    matrix<T> operator-(U);
    template<typename U>
    matrix<T>& operator-=(matrix<U>&);
    template<typename U>
    matrix<T>& operator-=(U);
    //乘法
    template<typename U>
    matrix<T> operator*(const matrix<U>&);  //矩阵点乘
    template<typename U>
    matrix<T> operator*(U);  //常数乘
    template<typename U>
    matrix<T>& operator*=(matrix<U>&);
    template<typename U>
    matrix<T>& operator*=(U);
    //除法
    template<typename U>
    matrix<T> operator/(const matrix<U>&);
    template<typename U>
    matrix<T> operator/(U);
    template<typename U>
    matrix<T>& operator/=(matrix<U>&);
    template<typename U>
    matrix<T>& operator/=(U);
    //矩阵叉乘
    template<typename U>
    matrix<T> mul(matrix<U>&);  
    //矩阵卷积
    matrix<T> conv(matrix<T>&,int ,int);
    //for pictures
    matrix<T> B();
    matrix<T> G();
    matrix<T> R();
    void outputImage(const char*);
    //输出
    template <typename U>
    friend std::ostream& operator<<(std::ostream&,const matrix<U>&);
};

//具体代码部分
//Constructor（无输入）
template <typename T>
matrix<T>::matrix()
{
    this->row = 0;
    this->col = 0;
    this->channel = 0;
    this->ptr = nullptr;
}

// Constructor（图片输入，结果按通道分开）
template <typename T>
matrix<T>::matrix(const char *filename)
{                                     // 这里也许可以用内存一致连续优化
    FILE *fp = fopen(filename, "rb"); // 打开文件。
    if (fp == NULL)
    { // 打开文件失败
        cout << "Error: open file failed!" << endl;
        this->row = 0; // 若打开失败，则令图像的行数为0，在主程序中判断行数是否为0来判断image初始化是否失败
    }
    // 得到图片长和宽，这里图片是一个像素3byte，分别是BGR
    int row, col;
    fseek(fp, 18, SEEK_CUR);
    fread(&col, sizeof(int), 1, fp);
    fread(&row, sizeof(int), 1, fp);
    this->row = row;
    this->col = col;
    this->channel = 3;
    // 得到图片大小 = 54(定义文件) + 1920x1080x3 = 6220854
    fseek(fp, 0, SEEK_END); // 到文档末尾
    int len = ftell(fp);
    T *data = new T[len - 54];                    // data中包含填充的0数据
    int gap = ((len - col * row * 3 - 54) / row); // 计算图片自动填充0的个数
    fseek(fp, 0, SEEK_SET);                       // 先把指针指回0
    fseek(fp, 54, SEEK_CUR);                      // 再加上54偏移，到达数据位
    fread(data, 1, (len - 54), fp);               // 把图片里的数据全部导入data,len-54的数据量包含补0,3*row*col不包含补0
    // 将填充0前的数据导出
    T *real_data = new T[row * col * this->channel];
    int move = 0;
    for (int i = 0; i < len - 54; i++)
    {
        if ((i - move) % (3 * col) == (3 * col - 1))
        {
            real_data[i - move] = data[i];
            move = move + gap;
            i = i + gap;
            continue;
        }
        real_data[i - move] = data[i];
    }
    shared_ptr<T[]> pointer(new T[row * col * this->channel]);
    int size = this->row * this->col;
    // 将图层分开
    for (int k = 0; k < this->channel; k++)
    {
        for (int i = 0; i < size; i++)
        {
            pointer.get()[i + k * size] = real_data[i * this->channel + k];
        }
    }
    // memcpy(pointer.get(), real_data, row * col * this->channel); // 将数据拷贝到image中
    this->ptr = pointer;
    fclose(fp); // 清除文件指针
    delete[] data;
    delete[] real_data;
}

// Constructor矩阵初始化为常数
template <typename T>
matrix<T>::matrix(int channel, int row, int col, T val)
{
    if (row <= 0 || col <= 0 || channel <= 0)
    {
        cout << "Error: matrix size should be positive!" << endl;
    }
    this->row = row;
    this->col = col;
    this->channel = channel;
    shared_ptr<T[]> pointer(new T[row * col * channel]);
    for (int i = 0; i < row * col * channel; i++)
    {
        pointer.get()[i] = val;
    }
    this->ptr = pointer;
}

// Constructor矩阵初始化
template <typename T>
matrix<T>::matrix(const matrix<T> &m)
{
    this->row = m.row;
    this->col = m.col;
    this->channel = m.channel;
    this->ptr = m.ptr; // soft copy
}

//矩阵赋值,softcopy
template<typename T>
matrix<T> &matrix<T>::operator=(const matrix<T> &m)
{
    this->channel = m.channel;
    this->row = m.row;
    this->col = m.col;
    this->ptr = m.ptr;
    cout << "softcopy" << endl;
    return *this;
}

// 矩阵赋值,hardcopy
template <typename T>
template <typename U>
matrix<T> &matrix<T>::operator=(const matrix<U> &m)
{
    this->channel = m.channel;
    this->row = m.row;
    this->col = m.col;
    shared_ptr<T[]> pointer(new T[m.row * m.col * m.channel]);
        for(int i = 0; i < m.row * m.col * m.channel; i++){
            pointer.get()[i] = m.ptr.get()[i];
        }
    this->ptr = pointer;
    cout << "hardcopy" << endl;
    return *this;
}

// 矩阵赋值常数
template <typename T>
template <typename U>
matrix<T> &matrix<T>::operator=(U val)
{
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        this->ptr.get()[i] = val;
    }
    return *this;
}

//矩阵相等判断
template<typename T>
template<typename U>
bool matrix<T>::operator==(const matrix<U> &m){
    if(typeid(T).name() != typeid(U).name()) {
        // cout << "Equal judge Error: matrix type not match!" << endl;
        return false;
    }
    else if(this->row != m.row || this->col != m.col || this->channel != m.channel){
        return false;
    }
    for(int i = 0; i < this->row * this->col * this->channel; i++){
        if(this->ptr.get()[i] != m.ptr.get()[i]){
            return false;
        }
    }
    return true;
}

// 矩阵加矩阵
template <typename T>
template <typename U>
matrix<T> matrix<T>::operator+(const matrix<U> &m)
{
    matrix<T> result(this->channel, this->row, this->col, 0);
    if (this->row != m.row || this->col != m.col != this->channel != m.channel)
    {
        cout << "Error: matrix size not match!" << endl;
        return result;
    }
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        result.ptr.get()[i] = m.ptr.get()[i] + this->ptr.get()[i];
    }
    return result;
}

// 矩阵加常数
template <typename T>
template <typename U>
matrix<T> matrix<T>::operator+(U val)
{
    matrix<T> result(this->channel, this->row, this->col, 0);
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        result.ptr.get()[i] = val + this->ptr.get()[i];
    }
    return result;
}

// 矩阵加等于矩阵
template <typename T>
template <typename U>
matrix<T>& matrix<T>::operator+=(matrix<U> &m)
{
    if (this->row != m.row || this->col != m.col != this->channel != m.channel)
    {
        cout << "Error: matrix size not match!" << endl;
        return *this;
    }
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        this->ptr.get()[i] += m.ptr.get()[i];
    }
    return *this;
}

//矩阵加等于常数
template <typename T>
template <typename U>
matrix<T>& matrix<T>::operator+=(U val)
{
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        this->ptr.get()[i] += val;
    }
    return *this;
}


// 矩阵减矩阵
template <typename T>
template <typename U>
matrix<T> matrix<T>::operator-(const matrix<U> &m)
{
    matrix<T> result(this->channel, this->row, this->col, 0);
    if (this->row != m.row || this->col != m.col != this->channel != m.channel)
    {
        cout << "Error: matrix size not match!" << endl;
        return result;
    }
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        result.ptr.get()[i] = m.ptr.get()[i] - this->ptr.get()[i];
    }
    return result;
}

// 矩阵减常数
template <typename T>
template <typename U>
matrix<T> matrix<T>::operator-(U val)
{
    matrix<T> result(this->channel, this->row, this->col, 0);
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        result.ptr.get()[i] = this->ptr.get()[i] - val;
    }
    return result;
}

// 矩阵减等于矩阵
template <typename T>
template <typename U>
matrix<T>& matrix<T>::operator-=(matrix<U> &m)
{
    if (this->row != m.row || this->col != m.col != this->channel != m.channel)
    {
        cout << "Error: matrix size not match!" << endl;
        return *this;
    }
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        this->ptr.get()[i] -= m.ptr.get()[i];
    }
    return *this;
}

//矩阵减等于常数
template <typename T>
template <typename U>
matrix<T>& matrix<T>::operator-=(U val)
{
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        this->ptr.get()[i] -= val;
    }
    return *this;
}

// 矩阵点乘矩阵
template <typename T>
template <typename U>
matrix<T> matrix<T>::operator*(const matrix<U> &m)
{
    matrix<T> result(this->channel, this->row, this->col, 0);
    if (this->row != m.row || this->col != m.col)
    {
        cout << "Error: matrix size not match!" << endl;
        return result;
    }
    for (int i = 0; i < this->row * this->col; i++)
    {
        result.ptr.get()[i] = m.ptr.get()[i] * this->ptr.get()[i];
    }
    return result;
}

// 矩阵乘常数
template <typename T>
template <typename U>
matrix<T> matrix<T>::operator*(U val)
{
    matrix<T> result(this->channel, this->row, this->col, 0);
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        result.ptr.get()[i] = val * this->ptr.get()[i];
    }
    return result;
}

// 矩阵乘等于矩阵
template <typename T>
template <typename U>
matrix<T>& matrix<T>::operator*=(matrix<U> &m)
{
    if (this->row != m.row || this->col != m.col != this->channel != m.channel)
    {
        cout << "Error: matrix size not match!" << endl;
        return *this;
    }
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        this->ptr.get()[i] *= m.ptr.get()[i];
    }
    return *this;
}

//矩阵乘等于常数
template <typename T>
template <typename U>
matrix<T>& matrix<T>::operator*=(U val)
{
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        this->ptr.get()[i] *= val;
    }
    return *this;
}

//矩阵除矩阵
template <typename T>
template <typename U>
matrix<T> matrix<T>::operator/(const matrix<U> &m)
{
    matrix<T> result(this->channel, this->row, this->col, 0);
    if (this->row != m.row || this->col != m.col)
    {
        cout << "Error: matrix size not match!" << endl;
        return result;
    }
    for (int i = 0; i < this->row * this->col; i++)
    {
        result.ptr.get()[i] = m.ptr.get()[i] / this->ptr.get()[i];
    }
    return result;
}

// 矩阵除常数
template <typename T>
template <typename U>
matrix<T> matrix<T>::operator/(U val)
{
    matrix<T> result(this->channel, this->row, this->col, 0);
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        result.ptr.get()[i] = this->ptr.get()[i] / val;
    }
    return result;
}

// 矩阵除等于矩阵
template <typename T>
template <typename U>
matrix<T>& matrix<T>::operator/=(matrix<U> &m)
{
    if (this->row != m.row || this->col != m.col != this->channel != m.channel)
    {
        cout << "Error: matrix size not match!" << endl;
        return *this;
    }
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        if(m.ptr.get()[i] == 0)
        {
            cout << "Error: matrix divide zero!" << endl;
            return *this;
        }
        this->ptr.get()[i] /= m.ptr.get()[i];
    }
    return *this;
}

//矩阵除等于常数
template <typename T>
template <typename U>
matrix<T>& matrix<T>::operator/=(U val)
{
    if(val == (U)0){
        cout << "Error: matrix divide zero!" << endl;
        return *this;
    }
    for (int i = 0; i < this->row * this->col * this->channel; i++)
    {
        this->ptr.get()[i] /= val;
    }
    return *this;
}


// 矩阵叉乘，结果channel数等于两个矩阵channel之积
template <typename T>
template <typename U>
matrix<T> matrix<T>::mul(matrix<U> &m)
{
    matrix<T> mat(this->channel * m.channel, this->row, m.col, 0);
    if (this->col != m.row)
    {
        cout << "mul Error: matrix col and row not match!" << endl;
        return mat;
    }
    int mul_size = this->col;
    mat.row = this->row;
    mat.col = m.col;
    mat.channel = this->channel * m.channel;
    cout << "mul size: " << mat.row << " " << mat.col << " " << mat.channel << endl;
    T sum = static_cast<T>(0);
    shared_ptr<T[]> pointer(new T[mat.row * mat.col * mat.channel]);
    for (int i = 0; i < this->channel; i++)//选择第一个矩阵的第i个通道
    {
        for (int j = 0; j < m.channel; j++)//选择第二个矩阵的第j个通道
        {
            for (int ii = 0; ii < mat.row; ii++)//选择第一个矩阵的第i行
            {
                for (int jj = 0; jj < mat.col; jj++)//选择第二个矩阵的第j列
                {
                    for (int s = 0; s < mul_size; s++)//第一个矩阵的第i行和第二个矩阵的第j列进行叉乘
                    {
                        sum += this->ptr.get()[i * this->col * this->row + ii * this->col + s] * m.ptr.get()[j * m.col * m.row + s * m.col + jj];
                        // cout << i * this->col * this->row + ii * this->col + s << " " << j * m.col * m.row + s * m.col + jj << endl;
                    }
                    pointer.get()[i * mat.col * mat.row * m.channel + j * mat.col * mat.row + ii * mat.col + jj] = sum;
                }
                sum = static_cast<T>(0);
            }
        }
    }
    mat.ptr = pointer;
    return mat;
}

template <typename T>
matrix<T> matrix<T>::conv(matrix<T> &kernel, int stride, int padding)
{
    matrix<T> mat(this->channel, 1, 1, 0);
    if (this->row < kernel.row || this->col < kernel.col)
    {
        cout << "conv Error: kernel size is too small!" << endl;
        return mat;
    }
    if (this->channel != kernel.channel)
    {
        cout << "conv Error: matrix channel not match!" << endl;
        return mat;
    }
    // 卷积操作
    mat.row = (this->row + 2 * padding - kernel.row + 1) / stride;
    mat.col = (this->col + 2 * padding - kernel.col + 1) / stride;
    shared_ptr<T[]> pointer(new T[mat.row * mat.col * mat.channel]);
    mat.ptr = pointer;
    T sum = 0;
    int x, y;
    // 遍历output里的每一位数据
    for (int k = 0; k < kernel.channel; k++)
    {
        for (int i = 0; i < mat.row; i++)
        {
            for (int j = 0; j < mat.col; j++)
            {
                // 计算coved_data中每一位的数据，具体方式为对应位置相乘求和
                for (int ii = 0; ii < kernel.row; ii++)
                {
                    x = k * this->row * this->col - padding + stride * i + ii;
                    for (int jj = 0; jj < kernel.col; jj++)
                    {
                        y = k * this->row * this->col - padding + stride * j + jj;
                        if (x < 0 || x >= this->row || y < 0 || y >= this->col)
                            continue; // 判断数字是否取padding值，若是padding则0
                        sum += this->ptr.get()[y + x * this->col + k * this->col * this->row] * kernel.ptr.get()[jj + ii * kernel.col];
                    }
                }
                mat.ptr.get()[k * mat.col * mat.row + i * mat.col + j] = sum;
                sum = 0;
            }
        }
    }
    return mat;
}

// 提取图片图层,BGR
template <typename T>
matrix<T> matrix<T>::B()
{
    if(this->channel != 3) 
    {
        cout << "B Error: channel not equal to 3!" << endl;
        return *this;
    }
    matrix<T> mat(1, this->row, this->col, 0);
    shared_ptr<T[]> pointer(new T[mat.row * mat.col]);
    memcpy(pointer.get(), this->ptr.get(), mat.row * mat.col * sizeof(T));
    mat.ptr = pointer;
    return mat;
}

template <typename T>
matrix<T> matrix<T>::G()
{
    if(this->channel != 3) 
    {
        cout << "G Error: channel not equal to 3!" << endl;
        return *this;
    }
    matrix<T> mat(1, this->row, this->col, 0);
    shared_ptr<T[]> pointer(new T[mat.row * mat.col]);
    memcpy(pointer.get(), this->ptr.get() + mat.row * mat.col, mat.row * mat.col * sizeof(T));
    mat.ptr = pointer;
    return mat;
}

template <typename T>
matrix<T> matrix<T>::R()
{
    if(this->channel != 3) 
    {
        cout << "R Error: channel not equal to 3!" << endl;
        return *this;
    }
    matrix<T> mat(1, this->row, this->col, 0);
    shared_ptr<T[]> pointer(new T[mat.row * mat.col]);
    memcpy(pointer.get(), this->ptr.get() + 2 * mat.row * mat.col, mat.row * mat.col * sizeof(T));
    mat.ptr = pointer;
    return mat;
}

//输出8位图片
template <typename T>
void matrix<T>::outputImage(const char *filename)
{
    //把图片再转成输出的格式
    shared_ptr<T[]> pointer(new T[this->row * this->col]);
    for(int i = 0; i < this->row * this->col; i++)
    {
        pointer.get()[i] = 0; // 初始化为 0
        for(int j = 0; j < this->channel; j++)
        pointer.get()[i] += this->ptr.get()[i + j * this->row * this->col];
    }
    int row = this->row;
    // 每一行需要考虑补0，让每一行是四字节的整数倍
    // cout << this->col << endl;
    int gap = (this->col % 4 == 0) ? 0 : 4 - (this->col % 4);
    int col = this->col + gap;
    int bmi[] = {col * row + 54 + 1024, 0, 54, 40, this->col, row, 1 | 8 << 16, 0, col * row, 0, 0, 256, 0};
    // cout << "gap="<<gap <<endl;
    // 定义颜色表
    typedef struct _RGBQUAD
    {
        // public:
        byte rgbBlue;     // 蓝色分量
        byte rgbGreen;    // 绿色分量
        byte rgbRed;      // 红色分量
        byte rgbReserved; // 保留值,必须为0
    } RGBQUAD;
    RGBQUAD rgbquad[256];
    for (int p = 0; p < 256; p++)
    {
        rgbquad[p].rgbBlue = (byte)p;
        rgbquad[p].rgbGreen = (byte)p;
        rgbquad[p].rgbRed = (byte)p;
        rgbquad[p].rgbReserved = (byte)0;
    }
    FILE *fp = fopen(filename, "wb");
    if( fp == NULL){
        cout << "File open error!" << endl;
    }
    fprintf(fp, "BM");
    fwrite(&bmi, 52, 1, fp);
    fwrite(&rgbquad, 4, 256, fp);
    unsigned char *array = new unsigned char[col * row];
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            if (j >= col - gap)
            {
                array[i * col + j] = (byte)0;
                continue;
            }
            // 这里输出第一个kernel卷积的结果，用于验证结果是否正确
            else
                array[i * col + j] = (byte)pointer.get()[j + this->col * i];
        }
    }
    fwrite(array, 1, row * col, fp);
    fclose(fp);
    delete[] array;
}

template <typename T>
std::ostream &operator<<(std::ostream &out, const matrix<T> &a)
{
    out << typeid(T).name() << "; row = " << a.row << " col = " << a.col << endl;
    for (int s = 1; s <= a.channel; s++)
    {
        out << "channel: " << s << endl;
        for (int i = 0; i < a.row; i++)
        {
            for (int j = 0; j < a.col; j++)
            {
                // out << std::dec << a.ptr.get()[i * a.col + j] << " ";
                out << a.ptr.get()[i * a.col + j] << " ";
            }
            out << endl;
        }
    }
    return out;
}

// template std::ostream &operator<<(std::ostream &,const matrix<int> &);
// template std::ostream &operator<<(std::ostream &,const matrix<double> &);
// template std::ostream &operator<<(std::ostream &,const matrix<float> &);
// template std::ostream &operator<<(std::ostream &,const matrix<unsigned char> &);
// template std::ostream &operator<<(std::ostream &,const matrix<short> &);

//class声明
// template class matrix<int>;
// template class matrix<double>;
// template class matrix<float>;
// template class matrix<unsigned char>;
// template class matrix<short>;
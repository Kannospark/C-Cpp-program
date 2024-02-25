// #include <iostream>
// #include <memory>
// #include <stdio.h>
// #include <string.h>
// #include "matrix.h"

// using namespace std;

// // Constructor（图片输入，结果按通道分开）
// template <typename T>
// matrix<T>::matrix(const char *filename)
// {                                     // 这里也许可以用内存一致连续优化
//     FILE *fp = fopen(filename, "rb"); // 打开文件。
//     if (fp == NULL)
//     { // 打开文件失败
//         cout << "Error: open file failed!" << endl;
//         this->row = 0; // 若打开失败，则令图像的行数为0，在主程序中判断行数是否为0来判断image初始化是否失败
//     }
//     // 得到图片长和宽，这里图片是一个像素3byte，分别是BGR
//     int row, col;
//     fseek(fp, 18, SEEK_CUR);
//     fread(&col, sizeof(int), 1, fp);
//     fread(&row, sizeof(int), 1, fp);
//     this->row = row;
//     this->col = col;
//     this->channel = 3;
//     // 得到图片大小 = 54(定义文件) + 1920x1080x3 = 6220854
//     fseek(fp, 0, SEEK_END); // 到文档末尾
//     int len = ftell(fp);
//     T *data = new T[len - 54];                    // data中包含填充的0数据
//     int gap = ((len - col * row * 3 - 54) / row); // 计算图片自动填充0的个数
//     fseek(fp, 0, SEEK_SET);                       // 先把指针指回0
//     fseek(fp, 54, SEEK_CUR);                      // 再加上54偏移，到达数据位
//     fread(data, 1, (len - 54), fp);               // 把图片里的数据全部导入data,len-54的数据量包含补0,3*row*col不包含补0
//     // 将填充0前的数据导出
//     T *real_data = new T[row * col * this->channel];
//     int move = 0;
//     for (int i = 0; i < len - 54; i++)
//     {
//         if ((i - move) % (3 * col) == (3 * col - 1))
//         {
//             real_data[i - move] = data[i];
//             move = move + gap;
//             i = i + gap;
//             continue;
//         }
//         real_data[i - move] = data[i];
//     }
//     shared_ptr<T> pointer(new T[row * col * this->channel]);
//     int size = this->row * this->col;
//     // 将图层分开
//     for (int k = 0; k < this->channel; k++)
//     {
//         for (int i = 0; i < size; i++)
//         {
//             pointer.get()[i + k * size] = real_data[i * this->channel + k];
//         }
//     }
//     // memcpy(pointer.get(), real_data, row * col * this->channel); // 将数据拷贝到image中
//     this->ptr = pointer;
//     fclose(fp); // 清除文件指针
//     delete[] data;
//     delete[] real_data;
// }

// // Constructor矩阵初始化为常数
// template <typename T>
// matrix<T>::matrix(int channel, int row, int col, T val)
// {
//     if (row <= 0 || col <= 0 || channel <= 0)
//     {
//         cout << "Error: matrix size should be positive!" << endl;
//     }
//     this->row = row;
//     this->col = col;
//     this->channel = channel;
//     shared_ptr<T> pointer(new T[row * col * channel]);
//     for (int i = 0; i < row * col * channel; i++)
//     {
//         pointer.get()[i] = val;
//     }
//     this->ptr = pointer;
// }

// // 矩阵复制初始化
// template <typename T>
// matrix<T>::matrix(matrix<T> &m)
// {
//     this->row = m.row;
//     this->col = m.col;
//     this->channel = m.channel;
//     this->ptr = m.ptr; // soft copy
// }

// // 矩阵加矩阵
// template <typename T>
// template <typename U>
// matrix<T> &matrix<T>::operator+(matrix<U> &m)
// {
//     matrix<T> result(this->channel, this->row, this->col, 0);
//     if (this->row != m.row || this->col != m.col != this->channel != m.channel)
//     {
//         cout << "Error: matrix size not match!" << endl;
//         return result;
//     }
//     for (int i = 0; i < this->row * this->col * this->channel; i++)
//     {
//         result.ptr.get()[i] = m.ptr.get()[i] + this->ptr.get()[i];
//     }
//     return result;
// }


// // 矩阵加常数
// template <typename T>
// template <typename U>
// matrix<T> &matrix<T>::operator+(U val)
// {
//     matrix<T> result(this->channel, this->row, this->col, 0);
//     for (int i = 0; i < this->row * this->col * this->channel; i++)
//     {
//         result.ptr.get()[i] = val + this->ptr.get()[i];
//     }
//     return result;
// }

// // 矩阵赋值
// template <typename T>
// template <typename U>
// matrix<T> &matrix<T>::operator=(matrix<U> &m)
// {
//     this->row = m.row;
//     this->col = m.col;
//     this->ptr = m.ptr;
//     return *this;
// }

// // 矩阵赋值常数
// template <typename T>
// template <typename U>
// matrix<T> &matrix<T>::operator=(U val)
// {
//     for (int i = 0; i < this->row * this->col * this->channel; i++)
//     {
//         this->ptr.get()[i] = val;
//     }
//     return *this;
// }

// // 点乘操作
// template <typename T>
// template <typename U>
// matrix<T> &matrix<T>::operator*(matrix<U> &m)
// {
//     matrix<T> result(this->channel, this->row, this->col, 0);
//     if (this->row != m.row || this->col != m.col)
//     {
//         cout << "Error: matrix size not match!" << endl;
//         return result;
//     }
//     for (int i = 0; i < this->row * this->col; i++)
//     {
//         result.ptr.get()[i] = m.ptr.get()[i] * this->ptr.get()[i];
//     }
//     return result;
// }

// // 矩阵乘常数
// template <typename T>
// template <typename U>
// matrix<T> &matrix<T>::operator*(U val)
// {
//     matrix<T> result(this->channel, this->row, this->col, 0);
//     for (int i = 0; i < this->row * this->col * this->channel; i++)
//     {
//         result.ptr.get()[i] = val * this->ptr.get()[i];
//     }
//     return result;
// }

// // 矩阵叉乘，结果channel数等于两个矩阵channel之积
// template <typename T>
// template <typename U>
// matrix<T>& matrix<T>::mul(matrix<U> &m)
// {
//     matrix<T> mat(this->channel * m.channel, this->row, m.col, 0);
//     if (this->col != m.row)
//     {
//         cout << "mul Error: matrix col and row not match!" << endl;
//         return mat;
//     }
//     int mul_size = this->col;
//     mat.row = this->row;
//     mat.col = m.col;
//     mat.channel = this->channel * m.channel;
//     cout << "mul size: " << mat.row << " " << mat.col << " " << mat.channel << endl;
//     T sum = static_cast<T>(0);
//     shared_ptr<T> pointer(new T[mat.row * mat.col * mat.channel]);
//     for (int i = 0; i < this->channel; i++)//选择第一个矩阵的第i个通道
//     {
//         for (int j = 0; j < m.channel; j++)//选择第二个矩阵的第j个通道
//         {
//             for (int ii = 0; ii < mat.row; ii++)//选择第一个矩阵的第i行
//             {
//                 for (int jj = 0; jj < mat.col; jj++)//选择第二个矩阵的第j列
//                 {
//                     for (int s = 0; s < mul_size; s++)//第一个矩阵的第i行和第二个矩阵的第j列进行叉乘
//                     {
//                         sum += this->ptr.get()[i * this->col * this->row + ii * this->col + s] * m.ptr.get()[j * m.col * m.row + s * m.col + jj];
//                         // cout << i * this->col * this->row + ii * this->col + s << " " << j * m.col * m.row + s * m.col + jj << endl;
//                     }
//                     pointer.get()[i * mat.col * mat.row * m.channel + j * mat.col * mat.row + ii * mat.col + jj] = sum;
//                 }
//                 sum = static_cast<T>(0);
//             }
//         }
//     }
//     mat.ptr = pointer;
//     return mat;
// }

// template <typename T>
// matrix<T> matrix<T>::conv(matrix<T> &kernel, int stride, int padding)
// {
//     matrix<T> mat(this->row, kernel.col, 1, 0);
//     if (this->row < kernel.row || this->col < kernel.col)
//     {
//         cout << "conv Error: kernel size is too small!" << endl;
//         return mat;
//     }
//     if (this->channel != kernel.channel)
//     {
//         cout << "conv Error: matrix channel not match!" << endl;
//         return mat;
//     }
//     // 卷积操作
//     mat.row = (this->row + 2 * padding - kernel.row + 1) / stride;
//     mat.col = (this->col + 2 * padding - kernel.col + 1) / stride;
//     shared_ptr<T> pointer(new T[mat.row * mat.col]);
//     mat.ptr = pointer;
//     T sum = 0;
//     int x, y;
//     // 遍历coved_data里的每一位数据
//     for (int k = 0; k < kernel.channel; k++)
//     {
//         for (int i = 0; i < mat.row; i++)
//         {
//             for (int j = 0; j < mat.col; j++)
//             {
//                 // 计算coved_data中每一位的数据，具体方式为对应位置相乘求和
//                 for (int ii = 0; ii < kernel.row; ii++)
//                 {
//                     x = k * this->row * this->col - padding + stride * i + ii;
//                     for (int jj = 0; jj < kernel.col; jj++)
//                     {
//                         y = k * this->row * this->col - padding + stride * j + jj;
//                         if (x < 0 || x >= this->row || y < 0 || y >= this->col)
//                             continue; // 判断数字是否取padding值，若是padding则0
//                         sum += this->ptr.get()[y + x * this->col + k * this->col * this->row] * kernel.ptr.get()[jj + ii * kernel.col];
//                     }
//                 }
//                 mat.ptr.get()[k * mat.col * mat.row + i * mat.col + j] = sum;
//                 sum = 0;
//             }
//         }
//     }
//     return mat;
// }

// // 提取图片图层,BGR
// template <typename T>
// matrix<T> matrix<T>::B()
// {
//     matrix<T> mat(1, this->row, this->col, 0);
//     shared_ptr<T> pointer(new T[mat.row * mat.col]);
//     memcpy(pointer.get(), this->ptr.get(), mat.row * mat.col * sizeof(T));
//     mat.ptr = pointer;
//     return mat;
// }

// template <typename T>
// matrix<T> matrix<T>::G()
// {
//     matrix<T> mat(1, this->row, this->col, 0);
//     shared_ptr<T> pointer(new T[mat.row * mat.col]);
//     memcpy(pointer.get(), this->ptr.get() + mat.row * mat.col, mat.row * mat.col * sizeof(T));
//     mat.ptr = pointer;
//     return mat;
// }

// template <typename T>
// matrix<T> matrix<T>::R()
// {
//     matrix<T> mat(1, this->row, this->col, 0);
//     shared_ptr<T> pointer(new T[mat.row * mat.col]);
//     memcpy(pointer.get(), this->ptr.get() + 2 * mat.row * mat.col, mat.row * mat.col * sizeof(T));
//     mat.ptr = pointer;
//     return mat;
// }

// template <typename T>
// std::ostream &operator<<(std::ostream &out, matrix<T> &a)
// {
//     out << typeid(T).name() << "; row = " << a.row << "col = " << a.col << endl;
//     for (int s = 1; s <= a.channel; s++)
//     {
//         out << "channel: " << s << endl;
//         for (int i = 0; i < a.row; i++)
//         {
//             for (int j = 0; j < a.col; j++)
//             {
//                 // out << std::dec << a.ptr.get()[i * a.col + j] << " ";
//                 out << a.ptr.get()[i * a.col + j] << " ";
//             }
//             out << endl;
//         }
//     }
//     return out;
// }
// template std::ostream &operator<<(std::ostream &, matrix<int> &);
// template std::ostream &operator<<(std::ostream &, matrix<double> &);
// template std::ostream &operator<<(std::ostream &, matrix<float> &);
// template std::ostream &operator<<(std::ostream &, matrix<unsigned char> &);
// template std::ostream &operator<<(std::ostream &, matrix<short> &);

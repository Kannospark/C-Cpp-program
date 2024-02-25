/*sub_im
* file name : calculatorv4.c
* desp : calculator version 4
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//宏定义
#define BASE (10) 
#define MAX(x,y) ((x)>(y)?(x):(y))
#define decimal_digits (3)  //这里可修改保留的小数位数
 
// 大数数据结构，struct表示构造结构体，typedef为新结构起了个新的名字，叫bignumber_s（写在末尾）
typedef struct bignumber_s{
    char sign; // 代表大数的符号，1为负数，0为正数
    int len;   // 代表大数的位数
    int point;  // 代表小数点位于第几位
    int sci;    //  科学计数法中0的数量
    char data[]; // 大数的数据内容，data[0]代表个位，data[1]代表十位，data[2]代表百位....
} bignumber_s;
 
bignumber_s *calc_add(bignumber_s *a, bignumber_s *b);
bignumber_s *calc_sub(bignumber_s *a, bignumber_s *b);
 
// 构造大数模板，len指向大数的数据位数，sign表示该大数的符号,point表示小数点在第几位,sci表科学计数法
bignumber_s *make_bignumber_temp(int len, int sign, int point, int sci)
{
    // 分配bignumber_s及其所代表数据 所需的内存
    bignumber_s *temp = malloc(sizeof(bignumber_s)+len);  //定义temp为bignumber_s的指针

    if (NULL == temp) {
        perror("Malloc"); //把一个描述性错误消息输出到标准错误 stderr，相当于出错后会在前面打上Malloc
        exit(-1); //退出程序，同时将-1写入环境变量ERRORLEVEL
    }
    temp->sign = sign;  //提取结构体变量bignumber_s的sign值
    temp->len  = len;
    temp->point = point;
    temp->sci = sci;
    // printf("sign=%d ",sign);
    // printf("len=%d ",len);
    // printf("point=%d ",point);
    // printf("sci=%d\n",sci);
    memset(temp->data, 0, len); //将temp里data的值的前len位填充0字符
    return temp;
}
 
// 处理数字字符串冗余的0，即"00123" -> "123"，同时去除小数点和e指数
const char *strip_str(char *str, int point, int sci_pos)
{
    int currentLen = strlen(str);
    //判断是否有科学计数法，如果有，把e去除，并减小point
    if(sci_pos!=0){
        str[currentLen - 1 - sci_pos] = '\0';
        // point = point - sci_pos - 1;
        currentLen = currentLen - sci_pos - 1;  //包括小数点
    }
    //若无科学计数法，直接计算
    if(point != 0){ //如果有小数，就把str的每一位往前移动
        for(int j = currentLen-point-1; j < currentLen; j++){
            // printf("str123[%d]=%s\n",j+1,str[j+1]);
                str[j] = str[j+1];  
        }
    }
     //然后把数字最前面的0去除
    int i = 0;
        int len = strlen(str);
        for (i=0; i<len-point&&str[i]=='0'; i++);
        // printf(str+i);
        return str+i;
}

// 将字符串数据填充进模板中
void fill_data_fromstr(bignumber_s *n, const char *str)
{
    int i = 0;
    int len = n->len;
    for (i=0; i<len; i++) {
        // printf("str[%d]=%s\n",len-1-i,str[len-1-i]);
        int d = str[len-1-i]-'0';  //这是反着填充的，str的最后一位填在大数的data的第一位
        if (d>=0 && d<=9) 
            n->data[i] = d;
        else {
          fprintf(stderr, "Invalid Number:%s\n", str);
          exit(-1);
        }
    }
}
// 从字符串构造一个大数，直接执行
bignumber_s *make_bignumber_fromstr(const char *str)
{
    // 处理负数字符串，即"-123"
    char *strnd = malloc(strlen(str)+1);  //为strnd创建一块地址，并把str复制给他
    strcpy(strnd, str);
    char str_origin[strlen(strnd)+1];  //为了保存strnd的当前值，方便后续读取科学计数法的数字
    strcpy(str_origin, strnd); 
    int sign = 0;
    if (strnd[0]=='-') {  //判断正负号
        sign = 1;
        strnd++;
        str++;
    }
    int currentLen = strlen(strnd);
    //找到字符串里e指数的位置
    int sci_pos = 0;  //表示e的位置
    int multi_sci = 0;
    for(int j = currentLen; j > 0; j--){
        if(strnd[j-1] == 'e' && multi_sci == 0){
            sci_pos = currentLen - j;
            multi_sci = 1;
        }
        else if(strnd[j-1] == 'e' && multi_sci == 1){
            fprintf(stderr,"you have input multiple e, please check your input number!");
            exit(-1);
        }
    }
    //找到字符串里小数点的位置，如果超过一个小数点会报错
    int point = 0;
    int multiPoint = 0;
    for(int j = currentLen; j > 0; j--){
        if(strnd[j-1] == '.' && multiPoint == 0){
            if(sci_pos != 0){
                point = currentLen - j -1 - sci_pos;  //倘若有科学计数法的情况
            }
            else{
                point = currentLen - j;  //没有科学技术法的情况
            }
            multiPoint = 1;  
        }
        else if(strnd[j-1] == '.' && multiPoint == 1){  // 出现多个.的情况striped str
            fprintf(stderr,"you have input multiple points, please check your input number!");
            exit(-1);
        }
    }
    // 处理数字字符串冗余的0，即"00123" -> "123"，同时去除小数点和e指数
    unsigned char * striped_str = strip_str(str, point, sci_pos);
    //这里之后返回值是去除了小数点和前面多余的0的数字
    int len = strlen(striped_str);
    //计算科学计数法sci的值
    int sci = 0;
    //str是去除了小数和e部分的整数部分,这里使用str_origin计算科学计数法的0数量
    if(sci_pos!=0){
        for(int i = strlen(str_origin) - sci_pos; i <= strlen(str_origin) - 1; i++){

            sci = 10*sci + str_origin[i] - '0';
            // printf("sci=%d,i=%d\n",sci,i);
        }
        // currentLen = currentLen - sci_pos - 1; //修正e指数去除之后的长度
    }
    //将数字填充完整
    if(point <= sci){
        sci = sci - point;
        len = len + sci;  //长度变更
        point = 0;
    }
    else{
        point = point - sci;
        sci = 0;
    }
    int i = 0;
    for(i = 0; i < sci; i++){
        striped_str[len-sci+i] = '0';
    }
    striped_str[len] = '\0';

    // printf("afterchanged: len=%d point=%d sci=%d\n",len,point,sci);
    // 指定数据位长度即符号，创建一个大数模板
    bignumber_s *temp = make_bignumber_temp(len, sign, point,sci);
    // 将字符串数据填充进模板中，完成大数创建
    fill_data_fromstr(temp, striped_str);
    return temp;
}

int judgeSci(bignumber_s * b){  //判断科学计数法输出有几个0
    if(b->sci == 1 && b->point == 0){
        int zeros = 0;
        for(int i = 0; i < b->len; i++){
            if(b->data[i] == 0) zeros++;
            else break;
        }
        return zeros;
    }
}

int judgeUseSci(bignumber_s *a, bignumber_s *b){ //判断是启动科学计数法输出,不启动0，启动1
    if(a->sci == 0  && b->sci == 0){
        return 0;
    }
    else{
        return 1;
    }
}

void print_bignumber(bignumber_s * b){
    if(b->point != 0 && b->sci != 0){
        if(b->point > b->sci) {
            b->point = b->point - b->sci;
            b->sci = 0;
        }
        else {
            b->sci = b->sci - b->point; 
            b->point = 0;
        }
    }
    int a = 0; //检验是否有小数
    int has_zero = 0; //检验是否前面有0
    printf("\n");
    int judgesci = judgeSci(b);
    if(judgesci == 0){
        for(int i = 0; i < b->len; i++){
            if(b->sign == 1 && i == 0) {
                printf("-");  //输出负号
            }
            if((i < b->len - b->point - 1) && has_zero == 0 && b->data[b->len-1-i] == 0) //防止前面输出0
            continue; //防止前面输出0
            if(i == (b->len - b->point) && (b->point!=0) && a == 0) {
                a = 1;
                has_zero = 1;
                printf(".");
                printf("%d",b->data[b->len-1-i]);
            }
            else{
                has_zero = 1;
                printf("%d",b->data[b->len-1-i]);
            }
        }
        printf("\n");
    }
    else{  //使用科学计数法输出
        for(int i = 0; i < b->len - judgesci; i++){
            if(b->sign == 1 && i == 0) {
                printf("-");  //输出负号
            }
            if((i < b->len - b->point) && has_zero == 0 && b->data[b->len-1-i] == 0) //防止前面输出0
            continue; //防止前面输出0
            if(i == (b->len - b->point) && (b->point!=0) && a == 0) {  //判断小数点
                a = 1;
                has_zero = 1;
                printf(".");
                printf("%d",b->data[b->len-1-i]);
            }
            //若要输出完整数字，请注释掉这个elseif
            else if(i == 1 && b->len - judgesci != 2){  //科学计数法的小数点
                has_zero = 1;
                printf("%d",b->data[b->len-1-i]);
                printf(".");
            }
            else{
                has_zero = 1;
                printf("%d",b->data[b->len-1-i]);
            }
            if(i == b->len - judgesci - 1 && b->len - judgesci != 2){  // 对应整数部分不止一位
                printf("e%d",b->len - 2);  //输出科学计数法
                // printf("e%d",judgesci);  //如果需要输出完整的数字，启动这部分并注释掉上面一行代码
            }
            if(i == b->len - judgesci - 1 && b->len - judgesci == 2){  // 对应整数部分就一位
                printf("e%d",judgesci);  //输出科学计数法
            }
        }
        printf("\n");
    }
}


void usage(const char *s)
{
    fprintf(stderr, "Usage:%s number1 +-x/ number2.\n",s);
    exit(-1);
}
 
//返回无符号加法时需要遍历的次数
int add_times(bignumber_s *a, bignumber_s *b){
    int point_part = MAX(a->point,b->point);
    int int_part = MAX(integer(a),integer(b));
    // printf("intp + pp = %d",int_part + point_part);
    return int_part + point_part;
}

int integer(bignumber_s *a){
    return a->len - a->point;
}

// 实现无符号加法，a和b为加数，r为和
void add_impl(bignumber_s *a, bignumber_s *b, bignumber_s *r)
{
    int i = 0;
    char carry = 0;
    int len = r->len;
    int point = r->point;
    int times = add_times(a,b);
    //带小数进行计算
    for (i=0; i<times; i++) {
        if((a->point + i - point) < 0 || a->point + i - point > (a->len-1)){
            carry += b->data[b->point+i-point];        }
        else if(b->point + i - point < 0 || b-> point + i - point > (b->len-1)){
            carry += a->data[a->point + i - point];        }
        else{
            carry += a->data[a->point+i-point] + b->data[b->point+i-point];
        }
        if(i == times - 1 && carry >= BASE) r->data[times] = 1;
        r->data[i] = carry%BASE;
        carry /= BASE;
    //计算整数
    }
}
 
bignumber_s *calc_add(bignumber_s *a, bignumber_s *b)
{
    // 情况一和情况四
    if (a->sign==b->sign) {
        // n位数+m位数，其和最大为max(n,m)+1位数
        int point = MAX(a->point,b->point); //a和b较大的一个数字的小数位数是和的小数位数
        int len = MAX(a->len-a->point, b->len-b->point) + 1 + point;  //如果超过10，就要进1

        bignumber_s *result = make_bignumber_temp(len,a->sign,point,0);  //符号与a保持一致
        result->sci = judgeUseSci(a,b);
        add_impl(a,b,result);
        return result;
    } else if (a->sign == 0 && b->sign == 1) {//情况二，变成减法
        b->sign = 0; //b数去绝对值
        return calc_sub(a,b);
    } else if (a->sign == 1 && b->sign == 0) {//情况三，变成减法
        a->sign = 0; //a数去绝对值
        return calc_sub(b,a);
    }
}
 
// 实现无符号减法，a-b , r为差
void sub_impl(bignumber_s *a, bignumber_s *b, bignumber_s *r)
{
        int i=0;
        int borrow = 0;
        int len = r->len;
        int point = r->point;
        int temp = 0;
        int times = add_times(a,b);
        for (i=0; i<times; i++) {
            if(a->point+i-point<0 || a->point+i-point>(a->len-1))
                temp = 0 + BASE - borrow  - b->data[b->point+i-point];
            else if(b->point+i-point<0 || b->point+i-point>(b->len-1))
                temp = a->data[a->point + i - point] + BASE - borrow;
            else{
                temp = a->data[a->point + i -point] + BASE - borrow - b->data[b->point + i - point];
            }
            r->data[i] = temp%BASE;
            borrow = temp / BASE ? 0 : 1;
        }
}
// data里面的数是个位在最前面，因此看有效位数从最右位开始看起，检验有几个重复的0
// 
int valid_len(bignumber_s *a)
{
    int len = a->len;
    int i = len-1;
    for (i=len-1; i>=0; i--) {  //直到遇到第一个0为止，len会一直减小，代表数字的真实位数
        if (a->data[i]==0) len--;
        else break;
    }
    return len;
}
// 判断两个大数的大小
// a > b 返回1 ，a<b 返回 -1 , a==b 返回0
int cmp(bignumber_s *a, bignumber_s *b)
{
 
        if (a->sign==0&&b->sign==1) return 1;  //a为正数，b是负数，a>b
        if (a->sign==1&&b->sign==0) return -1; //a为负数，b为正数，a<b
        //接下来就是先比较长度，取实际长度长的数字较大
        int sign = a->sign;
        int alen = a->len - a->point;
        int blen = b->len - b->point;
        if (alen>blen) return (sign==1?-1:1);
        else if (alen<blen) return (sign==1?1:-1);
        //若长度一致，则逐位比较大小
        else {
                int i = 0;
                int len = add_times(a,b); //返回需要遍历的次数
                for (i = 0; i < len; i++) {
                    if(a->len-i-1 < 0 && b->len-1-i >=0) return (sign==1?1:-1);  //如果a小数末尾数量多则a大
                    else if(b->len -1 -i < 0 && a->len-1-i >=0) return (sign==1?-1:1); //同上
                    else if (a->data[a->len -i -1] > b->data[b->len -i -1]) return (sign==1?-1:1); //比较a和b的每一位大小
                    else if (a->data[a->len - i - 1]<b->data[b->len -i -1 ]) return (sign==1?1:-1);
                }   
            return 0;
        }
}
//先判断大小，再进行无符号的减法，用大数减小数
bignumber_s *calc_sub(bignumber_s *a, bignumber_s *b)
{
    // 情况一
    if(a->sign==0 && b->sign==0) {

        if (cmp(a,b)>=0) { // 被减数大于等于减数，即差大于等于0时
                int len = a->len;
                int point = MAX(a->point,b->point);
                bignumber_s *result = make_bignumber_temp(len,0,point,0);
                sub_impl(a,b,result);
                return result;
        } else { // 被减数小于减数，即差小于0时
                int len = b->len;
                int point = MAX(a->point,b->point);
                bignumber_s *result = make_bignumber_temp(len,1,point,0);
                sub_impl(b,a,result);  //先确认结果是负号后，直接调换输入顺序，进行正的无符号减法
                return result;
        }
    } else if (a->sign==1 && b->sign==1) { //情况四，负减负等于正减正，调换顺序，变成情况一
        b->sign=0;  
        a->sign=0;
        return calc_sub(b,a); //调换减数和被减数，变成情况一
    }else if (a->sign==0 && b->sign==1) { // 情况二，正减负等于加法
        b->sign=0;
        bignumber_s *result = calc_add(a,b);
        result->sign = 0;
        return result; 
    } else if (a->sign==1 && b->sign==0) { // 情况三，负减正等于负数加法
        a->sign=0;
        bignumber_s *result = calc_add(a,b);
        result->sign = 1;
        result->sci = judgeUseSci(a,b);
        return result; 
    } 
}
 
// 实现无符号乘法，x * y , z为积
void mul_impl(bignumber_s *x, bignumber_s *y, bignumber_s *z)
{
    int n = x->len;
    int m = y->len;
    int i = 0;
    int j = 0;
    x->point + y->point;
    int carry = 0;
    for (i=0; i<m; i++) {
        // y的每一位乘以x，即计算y[i] * x[0...n] 并累加到z[i...i+n]中
        for (j=0; j<n; j++) {
            carry += y->data[i]*x->data[j] + z->data[i+j];
            z->data[i+j] = carry%BASE;
            carry /= BASE;
        }
 
        // 将剩余的进位,继续在z[i+n...n+m]中累加，从而完成y[i]乘以x[0...n]其积累加到z[i...m+n]中
        for (; j+i<n+m; j++) {
            carry += z->data[i+j];
            z->data[i+j] = carry % BASE;
            carry /= BASE;
        }
    }
}
 
bignumber_s *calc_mul(bignumber_s *a, bignumber_s *b)
{
    int len = a->len + b->len;
    int point = a->point + b->point;
    int sci = a->sci + b->sci;
    bignumber_s *result = make_bignumber_temp(len,a->sign==b->sign?0:1,point,sci); //a->sign==b->sign为情况一，否则为情况二。
    mul_impl(a,b,result);
    result->sci = judgeUseSci(a,b);
    return result;
}
 
// 大数加一操作
void plusone(bignumber_s *a)
{
   int len = a->len ;
   int i;
   int carry = 1;
   for (i=0; i<len; i++) {
      carry += a->data[i];
      a->data[i] = carry%BASE;
      carry /= BASE;
   }
}
///以下函数全是用于除法
//判断输入数字是不是0,如果全是0则返回0，如果不全是0则返回1
int judge_zero(bignumber_s *a){
    int len = a->len;
    for(int i = 0; i < len; i++){
        if(a->data[i] != 0){
            return 1;
        }
    }
    return 0;
}
//判断a和b谁更大，若大于则1，小于则-1，一样0
int bigger(bignumber_s *a,bignumber_s *b){
    int len = a->len;
    for(int i = 1; i <= len; i++){   
        if(a->data[len-i] > b->data[len-i]) {
            return 1;
        }
        else if(b->data[len-i] > a->data[len-i]) {
            return -1;
        }
    }
    return 0;
}


// 实现无符号减法，a-b , r为差

bignumber_s * add_zero(bignumber_s * a, int value){
    if(value > 0){

        int alen = a->len;
        a->len = alen + value;
        for(int i = alen; i >= 0; i--){
            a->data[i+value] = a->data[i];//这里不确定对不对**
        }
        for(int i = 0; i < value ; i++){
            a->data[i] = 0;
        }
    }
    
    if(value < 0){
        int alen = a->len;  
        a->len = alen + value;  //计算新的a的长度
        for(int i = 0; i < a->len; i++){
            a->data[i] = a->data[i-value];
        }
    }
    return a;
}

// 大数除法实现
bignumber_s *calc_div(bignumber_s *a, bignumber_s *b)
{

    bignumber_s *zero = make_bignumber_temp(1,0,0,0);
    if (!judge_zero(b)) {// 除数为0 报错
        fprintf(stderr,"Integer division by zero\n");
        exit(-1);
    }else if (!judge_zero(a)) { // 被除数为0 ，结果为0
        return zero;
    }
    int alen = a->len;  //记录原始a的长度
    int blen = b->len;
    int p = decimal_digits; //保留p位小数

    //判断a,b谁更长
    int value = b->point - a->point + p;
    a = add_zero(a, value);  //a的长度会发生变化
    int clen = a->len - blen + 1;
    bignumber_s * aa = make_bignumber_temp(blen,0,0,0);
    bignumber_s * bb = make_bignumber_temp(blen,0,0,0);
    bignumber_s * bbb = make_bignumber_temp(blen+1,0,0,0);  //创建一个bb的数位加一的版本，方便与ccc比较大小
    bignumber_s * cc = make_bignumber_temp(blen,0,0,0);
    bignumber_s * ccc = make_bignumber_temp(blen + 1,0,0,0);
    bignumber_s * ccc_medium = make_bignumber_temp(blen + 1,0,0,0);
    bignumber_s * result = make_bignumber_temp(clen,0,p,0);
    int len = blen;
    for(int i = 0; i <= a->len - blen; i++){  //a是补充完0后的
        if(i == 0){
            for(int j = 1; j <= len; j++){
                aa->data[len-j] = a->data[a->len-j];
                bb->data[len-j] = b->data[blen-j];
                bbb->data[bbb->len-j-1] = b->data[blen-j];  //从倒数第二位开始补充
            }
            bbb->data[bbb->len-1] = 0;  //最后一位补上0
            cc = aa;
            int time = 0;
            while(bigger(cc,bb)>=0){
                aa = cc;
                sub_impl(aa,bb,cc);  //输出存储在cc；里
                time++;
            }
            result->data[clen-i-1] = time;
            time = 0;
        }
        else if(i == 1){
            //为ccc赋值，ccc包含了a的下一位
            for(int j = 1; j <= blen; j++){
                ccc->data[blen-j+1] = cc->data[blen-j];
            }
            ccc->data[0] = a->data[a->len-blen-i];//赋值a的下一位
            int time = 0;  //计数

            while(bigger(ccc,bbb)>=0){
                ccc_medium = ccc;
                sub_impl(ccc_medium,bbb,ccc);  //ccc和ccc_medium一起减小bb
                time++;
            }
            result->data[clen-i-1] = time;
            time = 0;
        }
        else{
            //与上式基本相同，把数据前移,ccc长度时blen+1
            for(int j = 1; j <= ccc->len - 1; j++){
                ccc->data[ccc->len-j] = ccc->data[ccc->len-j-1];
            }
            ccc->data[0] = a->data[a->len-blen-i];//赋值a的下一位
            int time = 0;  //计数

            while(bigger(ccc,bbb)>=0){
                ccc_medium = ccc;
                sub_impl(ccc_medium,bbb,ccc);
                time++;
            }
            result->data[clen-i-1] = time;
            time = 0;
        }
    }
    // print_bignumber(result);
    result->sci = judgeUseSci(a,b);
    if(a->sign == b->sign) result->sign = 0;
    else result->sign = 1;
    return result;
}
 
int main(int argc, char *argv[])
{
    int judge = 0;
    if(0 == strcmp(argv[2],"+\0")){
        judge = 1;
    }
    if(0 == strcmp(argv[2],"-\0"))
    {
        judge = 2;
    }
    else if (0 == strcmp(argv[2],"x\0")){
        judge = 3;
    }
    else if (0 == strcmp(argv[2],"/\0")){
        judge = 4;
    }
    bignumber_s *b = make_bignumber_fromstr(argv[3]);
    bignumber_s *a = make_bignumber_fromstr(argv[1]);
    // printf("argv[2]=%d\n",*argv[2]);
    if (argc!=4) usage(argv[0]);
    if (judge == 1)
        print_bignumber(calc_add(a,b));
    else if (judge == 2)
        print_bignumber(calc_sub(a,b));
    else if (judge == 3)
        print_bignumber(calc_mul(a,b));
    else if (judge == 4)
        print_bignumber(calc_div(a,b));
    else usage(argv[0]);
    return 0;
}
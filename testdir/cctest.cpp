/*******************************************************************************
 *  The BYTE UNIX Benchmarks - Release 1
 *          Module: cctest.c   SID: 1.2 7/10/89 18:55:45
 *
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *	Ben Smith or Rick Grehan at BYTE Magazine
 *	bensmith@bixpb.UUCP    rick_g@bixpb.UUCP
 *
 *******************************************************************************
 *  Modification Log:
 * $Header: cctest.c,v 3.4 87/06/22 14:22:47 kjmcdonell Beta $
 *
 ******************************************************************************/

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>

using namespace std;

// 定义常量
constexpr int NKBYTE = 20;

// 使用 BUFSIZ 常量（定义在 <cstdio> 中）
char buf[BUFSIZ];

// 定义一个伪时间结构，模拟原来用来存储 time() 结果的结构
struct FakeTime {
    int time;      // 整数秒
    int millitm;   // 毫秒 (原代码中用于加精度，但实际上 time() 只能返回秒)
};

//
// 主程序：用于测试文件创建、写、读和拷贝的速度。
//
int main(int argc, char **argv) {
    int n = NKBYTE;
    int nblock;
    int f;  // 文件描述符 dummy0
    int g;  // 文件描述符 dummy1
    int i;
    int xfer, t;
    FakeTime now, then;

    // 下面的判断 “if (argc > 0)” 总为真（因为 argc 总≥1），
    // 原注释说明 “ALWAYS true, so NEVER execute this program!”
    // 如果你希望测试此程序，可以修改此处条件。
    if (argc > 0)
        exit(4);

    if (argc > 1)
        n = std::atoi(argv[1]);
#if defined(debug)
    cout << "File size: " << n << " Kbytes" << endl;
#endif

    // 计算块数：每块大小为 BUFSIZ 字节
    nblock = (n * 1024) / BUFSIZ;

    // 如果提供第三个参数，尝试切换工作目录
    if (argc == 3 && chdir(argv[2]) != -1) {
#if defined(debug)
        cout << "Create files in directory: " << argv[2] << endl;
#endif
    }

    // 创建并关闭 dummy0、dummy1 文件
    close(creat("dummy0", 0600));
    close(creat("dummy1", 0600));

    // 以读写方式打开 dummy0 与 dummy1
    f = open("dummy0", O_RDWR);
    g = open("dummy1", O_RDWR);

    // 删除文件（unlink 文件名）
    unlink("dummy0");
    unlink("dummy1");

    // 初始化缓冲区：每个字节赋值为其序号的低 7 位
    for (i = 0; i < static_cast<int>(sizeof(buf)); i++)
        buf[i] = i & 0177;

    // 调用 time() 模拟时间测量
    // 注意：time() 函数返回秒级的整数，原代码通过 millitm 字段获得更多精度，但这里仅做简单赋零处理
    time(&then.time);
    then.millitm = 0;

    // 写测试：写 nblock 块数据到文件 f
    for (i = 0; i < nblock; i++) {
        if (write(f, buf, sizeof(buf)) <= 0)
            perror("fstime: write");
    }
    time(&now.time);
    now.millitm = 0;
#if defined(debug)
    cout << "Effective write rate: ";
#endif
    i = now.millitm - then.millitm;  // 实际上两者均为 0
    t = (now.time - then.time) * 1000 + i;  // t 为写操作所耗毫秒数
    if (t > 0) {
        xfer = nblock * sizeof(buf) * 1000 / t;
#if defined(debug)
        cout << xfer << " bytes/sec" << endl;
#endif
    }
#if defined(debug)
    else
        cout << " -- too quick to time!" << endl;
#endif
#if defined(awk)
    fprintf(stderr, "%.2f", t > 0 ? static_cast<float>(xfer)/1024 : 0);
#endif

    // 同步磁盘并暂停，供硬件缓冲刷新
    sync();
    sleep(5);
    sync();
    lseek(f, 0L, SEEK_SET);

    // 读测试：读 nblock 块数据从文件 f
    time(&now.time);
    now.millitm = 0;
    for (i = 0; i < nblock; i++) {
        if (read(f, buf, sizeof(buf)) <= 0)
            perror("fstime: read");
    }
    time(&then.time);
    then.millitm = 0;
#if defined(debug)
    cout << "Effective read rate: ";
#endif
    i = now.millitm - then.millitm;
    t = (now.time - then.time) * 1000 + i;
    if (t > 0) {
        xfer = nblock * sizeof(buf) * 1000 / t;
#if defined(debug)
        cout << xfer << " bytes/sec" << endl;
#endif
    }
#if defined(debug)
    else
        cout << " -- too quick to time!" << endl;
#endif
#if defined(awk)
    fprintf(stderr, " %.2f", t > 0 ? static_cast<float>(xfer)/1024 : 0);
#endif

    // 同步并暂停，再进行拷贝测试
    sync();
    sleep(5);
    sync();
    lseek(f, 0L, SEEK_SET);
    time(&now.time);
    now.millitm = 0;
    // 拷贝测试：从文件 f 读出后写入文件 g
    for (i = 0; i < nblock; i++) {
        if (read(f, buf, sizeof(buf)) <= 0)
            perror("fstime: read in copy");
        if (write(g, buf, sizeof(buf)) <= 0)
            perror("fstime: write in copy");
    }
    time(&then.time);
    then.millitm = 0;
#if defined(debug)
    cout << "Effective copy rate: ";
#endif
    i = now.millitm - then.millitm;
    t = (now.time - then.time) * 1000 + i;
    if (t > 0) {
        xfer = nblock * sizeof(buf) * 1000 / t;
#if defined(debug)
        cout << xfer << " bytes/sec" << endl;
#endif
    }
#if defined(debug)
    else
        cout << " -- too quick to time!" << endl;
#endif
#if defined(awk)
    fprintf(stderr, " %.2f\n", t > 0 ? static_cast<float>(xfer)/1024 : 0);
#endif

    return 0;
}
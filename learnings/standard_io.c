#define BUFFER_SIZE 1024

typedef unsigned long long size_t;

struct _iobuf
{
    void *_Placeholder;
} typedef FILE;

FILE *__cdecl __acrt_iob_func(unsigned index);
size_t __cdecl fread(void *_DstBuf, size_t _ElementSize, size_t _Count, FILE *_File);
size_t __cdecl fwrite(const void *_Str, size_t _Size, size_t _Count, FILE *_File);

#define stdin (__acrt_iob_func(0))
#define stdout (__acrt_iob_func(1))
#define stderr (__acrt_iob_func(2))

int main(int argc, char const *argv[])
{
    char flex[BUFFER_SIZE] = "Mew\n";

    // fread(flex, 1, BUFFER_SIZE, stdin);
    fwrite(flex, 1, BUFFER_SIZE, stdout);
    fwrite(flex, 1, BUFFER_SIZE, stderr);
}

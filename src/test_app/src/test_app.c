#include "uapi_mm.h"

typedef struct emp
{
    char name[32];
    uint32_t emp_id;
} empt_t;

typedef struct student
{
    char name[32];
    uint32_t stu_id;
    char degree[24];
    uint8_t grade;
} student_t;

int main(int argc, char **argv)
{
    mm_init();
    MM_REG_STRUCT(empt_t);
    MM_REG_STRUCT(student_t);
    mm_print_registered_struct_records();

    xcalloc("empt_t", 1);
    xcalloc("student_t", 1);

    mm_print_mem_usage(NULL);
    mm_print_block_usage();

    return 0;
}

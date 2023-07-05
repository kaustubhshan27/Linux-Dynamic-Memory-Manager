#include <stdio.h>
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

    printf("\n******************** TEST 1 ********************");
    empt_t *emp1 = xcalloc("empt_t", 1);
    empt_t *emp2 = xcalloc("empt_t", 1);
    empt_t *emp3 = xcalloc("empt_t", 1);
    empt_t *stud1 = xcalloc("student_t", 1);
    empt_t *stud2 = xcalloc("student_t", 1);

    mm_print_mem_usage(NULL);
    mm_print_block_usage();

    printf("\n******************** TEST 2 ********************");
    xfree(emp1);
    xfree(emp3);
    xfree(stud2);
    mm_print_mem_usage(NULL);
    mm_print_block_usage();
    
    printf("\n******************** TEST 3 ********************");
    xfree(emp2);
    xfree(stud1);
    mm_print_mem_usage(NULL);
    mm_print_block_usage();

    return 0;
}

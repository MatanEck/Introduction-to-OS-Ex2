#pragma once
//  SourceGrades.h  //
/**********************************************

Authors:
Matan Eckhaus Moyal
Dvir Katz

Project : Ex2 - Grades

* *********************************************/
/// Description: This is the declarations module for the project Grades. 
///              Header file that contains the functions declarations for the main module to use.

#define _CRT_SECURE_NO_WARNINGS

// Includes:
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <Windows.h>

// Constants:
#define STATUS_SUCCESS  0
#define STATUS_FAILURE  1
#define MAX_THREADS 10
#define FOLDERS_NUM 4
#define ARGS_NUM 5
#define DELAY_TIME 5000

// A struct for thread parameters
typedef struct
{
	int index;
	int real;
	int human;
	int english;
	int eval;
} grade_params;


// Declarations:
static HANDLE create_thread_grades(LPTHREAD_START_ROUTINE p_start_routine,
	LPVOID p_thread_parameters,
	LPDWORD p_thread_id);

DWORD WINAPI thread_main(LPVOID lpParam);
extern int* read_file(LPSTR file_name, int* p_lines_count);
extern int convert_str_to_int_arr(char* source_file, int* source_arr, int* p_lines_count);
int calc_avg(int* source_arr, int* result_arr, int weight, int arr_len);
extern DWORD write_file(LPSTR path_name, int lines_count, char* result_arr);
extern char* convert_int_array_to_str(int* src_arr, int* p_lines_count);

// Note: see descriptions for the functions in SourceGrades.c
// MThreadgetDist2.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include<windows.h>
#include<direct.h>
#include<io.h>
#include<vector>
#include <typeinfo>
#include <thread>
#include <string>

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))
#define dist(x,y) ((x-y)*(x-y))

#define INF 1e20       //Pseudo Infitinte number for this code
using namespace std;

long long int fileSeekPos = 0;
long long int fileSeekPos2 = 0;

//int mynum = 0;  //表示已经读出的块对应文件中最大的一行

/// Data structure for sorting the query
typedef struct Index
{
	double value;
	int    index;
} Index;

/// Data structure (circular array) for finding minimum and maximum for LB_Keogh envolop
struct deque
{
	int *dq;
	int size, capacity;
	int f, r;
};


/// Sorting function for the query, sort by abs(z_norm(q[i])) from high to low
int comp(const void *a, const void* b)
{
	Index* x = (Index*)a;
	Index* y = (Index*)b;
	return abs(y->value) - abs(x->value);   // high to low
}

/// Initial the queue at the begining step of envelop calculation
void init(deque *d, int capacity)
{
	d->capacity = capacity;
	d->size = 0;
	d->dq = (int *)malloc(sizeof(int)*d->capacity);
	d->f = 0;
	d->r = d->capacity - 1;
}

/// Destroy the queue
void destroy(deque *d)
{
	free(d->dq);
}

/// Insert to the queue at the back
void push_back(struct deque *d, int v)
{
	d->dq[d->r] = v;
	d->r--;
	if (d->r < 0)
		d->r = d->capacity - 1;
	d->size++;
}

/// Delete the current (front) element from queue
void pop_front(struct deque *d)
{
	d->f--;
	if (d->f < 0)
		d->f = d->capacity - 1;
	d->size--;
}

/// Delete the last element from queue
void pop_back(struct deque *d)
{
	d->r = (d->r + 1) % d->capacity;
	d->size--;
}

/// Get the value at the current position of the circular queue
int front(struct deque *d)
{
	int aux = d->f - 1;

	if (aux < 0)
		aux = d->capacity - 1;
	return d->dq[aux];
}

/// Get the value at the last position of the circular queueint back(struct deque *d)
int back(struct deque *d)
{
	int aux = (d->r + 1) % d->capacity;
	return d->dq[aux];
}

/// Check whether or not the queue is empty
int empty(struct deque *d)
{
	return d->size == 0;
}

/// Finding the envelop of min and max value for LB_Keogh
/// Implementation idea is intoruduced by Danial Lemire in his paper
/// "Faster Retrieval with a Two-Pass Dynamic-Time-Warping Lower Bound", Pattern Recognition 42(9), 2009.
void lower_upper_lemire(double *t, int len, int r, double *l, double *u)
{
	struct deque du, dl;

	init(&du, 2 * r + 2);
	init(&dl, 2 * r + 2);

	push_back(&du, 0);
	push_back(&dl, 0);

	for (int i = 1; i < len; i++)
	{
		if (i > r)
		{
			u[i - r - 1] = t[front(&du)];
			l[i - r - 1] = t[front(&dl)];
		}
		if (t[i] > t[i - 1])
		{
			pop_back(&du);
			while (!empty(&du) && t[i] > t[back(&du)])
				pop_back(&du);
		}
		else
		{
			pop_back(&dl);
			while (!empty(&dl) && t[i] < t[back(&dl)])
				pop_back(&dl);
		}
		push_back(&du, i);
		push_back(&dl, i);
		if (i == 2 * r + 1 + front(&du))
			pop_front(&du);
		else if (i == 2 * r + 1 + front(&dl))
			pop_front(&dl);
	}
	for (int i = len; i < len + r + 1; i++)
	{
		u[i - r - 1] = t[front(&du)];
		l[i - r - 1] = t[front(&dl)];
		if (i - front(&du) >= 2 * r + 1)
			pop_front(&du);
		if (i - front(&dl) >= 2 * r + 1)
			pop_front(&dl);
	}
	destroy(&du);
	destroy(&dl);
}

/// Calculate quick lower bound
/// Usually, LB_Kim take time O(m) for finding top,bottom,fist and last.
/// However, because of z-normalization the top and bottom cannot give siginifant benefits.
/// And using the first and last points can be computed in constant time.
/// The prunning power of LB_Kim is non-trivial, especially when the query is not long, say in length 128.
double lb_kim_hierarchy(double *t, double *q, int j, int len, double mean, double std, double bsf = INF)
{
	/// 1 point at front and back
	double d, lb;
	double x0 = (t[j] - mean) / std;
	double y0 = (t[(len - 1 + j)] - mean) / std;
	lb = dist(x0, q[0]) + dist(y0, q[len - 1]);
	if (lb >= bsf)   return lb;

	/// 2 points at front
	double x1 = (t[(j + 1)] - mean) / std;
	d = min(dist(x1, q[0]), dist(x0, q[1]));
	d = min(d, dist(x1, q[1]));
	lb += d;
	if (lb >= bsf)   return lb;

	/// 2 points at back
	double y1 = (t[(len - 2 + j)] - mean) / std;
	d = min(dist(y1, q[len - 1]), dist(y0, q[len - 2]));
	d = min(d, dist(y1, q[len - 2]));
	lb += d;
	if (lb >= bsf)   return lb;

	/// 3 points at front
	double x2 = (t[(j + 2)] - mean) / std;
	d = min(dist(x0, q[2]), dist(x1, q[2]));
	d = min(d, dist(x2, q[2]));
	d = min(d, dist(x2, q[1]));
	d = min(d, dist(x2, q[0]));
	lb += d;
	if (lb >= bsf)   return lb;

	/// 3 points at back
	double y2 = (t[(len - 3 + j)] - mean) / std;
	d = min(dist(y0, q[len - 3]), dist(y1, q[len - 3]));
	d = min(d, dist(y2, q[len - 3]));
	d = min(d, dist(y2, q[len - 2]));
	d = min(d, dist(y2, q[len - 1]));
	lb += d;

	return lb;
}

/// LB_Keogh 1: Create Envelop for the query
/// Note that because the query is known, envelop can be created once at the begenining.
///
/// Variable Explanation,
/// order : sorted indices for the query.
/// uo, lo: upper and lower envelops for the query, which already sorted.
/// t     : a circular array keeping the current data.
/// j     : index of the starting location in t
/// cb    : (output) current bound at each position. It will be used later for early abandoning in DTW.
double lb_keogh_cumulative(int* order, double *t, double *uo, double *lo, double *cb, int j, int len, double mean, double std, double best_so_far = INF)
{
	double lb = 0;
	double x, d;

	for (int i = 0; i < len && lb < best_so_far; i++)
	{
		x = (t[(order[i] + j)] - mean) / std;
		d = 0;
		if (x > uo[i])
			d = dist(x, uo[i]);
		else if (x < lo[i])
			d = dist(x, lo[i]);
		lb += d;
		cb[order[i]] = d;
	}
	return lb;
}

/// LB_Keogh 2: Create Envelop for the data
/// Note that the envelops have been created (in main function) when each data point has been read.
///
/// Variable Explanation,
/// tz: Z-normalized data
/// qo: sorted query
/// cb: (output) current bound at each position. Used later for early abandoning in DTW.
/// l,u: lower and upper envelop of the current data
double lb_keogh_data_cumulative(int* order, double *tz, double *qo, double *cb, double *l, double *u, int len, double mean, double std, double best_so_far = INF)
{
	double lb = 0;
	double uu, ll, d;

	for (int i = 0; i < len && lb < best_so_far; i++)
	{
		uu = (u[order[i]] - mean) / std;
		ll = (l[order[i]] - mean) / std;
		d = 0;
		if (qo[i] > uu)
			d = dist(qo[i], uu);
		else
		{
			if (qo[i] < ll)
				d = dist(qo[i], ll);
		}
		lb += d;
		cb[order[i]] = d;
	}
	return lb;
}

/// Calculate Dynamic Time Wrapping distance
/// A,B: data and query, respectively
/// cb : cummulative bound used for early abandoning
/// r  : size of Sakoe-Chiba warpping band
double dtw(double* A, double* B, double *cb, int m, int r, double bsf = INF)
{

	double *cost;
	double *cost_prev;
	double *cost_tmp;
	int i, j, k;
	double x, y, z, min_cost;

	/// Instead of using matrix of size O(m^2) or O(mr), we will reuse two array of size O(r).
	cost = (double*)malloc(sizeof(double)*(2 * r + 1));
	for (k = 0; k<2 * r + 1; k++)    cost[k] = INF;

	cost_prev = (double*)malloc(sizeof(double)*(2 * r + 1));
	for (k = 0; k<2 * r + 1; k++)    cost_prev[k] = INF;

	for (i = 0; i<m; i++)
	{
		k = max(0, r - i);
		min_cost = INF;

		for (j = max(0, i - r); j <= min(m - 1, i + r); j++, k++)
		{
			/// Initialize all row and column
			if ((i == 0) && (j == 0))
			{
				cost[k] = dist(A[0], B[0]);
				min_cost = cost[k];
				continue;
			}

			if ((j - 1<0) || (k - 1<0))     y = INF;
			else                      y = cost[k - 1];
			if ((i - 1<0) || (k + 1>2 * r))   x = INF;
			else                      x = cost_prev[k + 1];
			if ((i - 1<0) || (j - 1<0))     z = INF;
			else                      z = cost_prev[k];

			/// Classic DTW calculation
			cost[k] = min(min(x, y), z) + dist(A[i], B[j]);

			/// Find minimum cost in row for early abandoning (possibly to use column instead of row).
			if (cost[k] < min_cost)
			{
				min_cost = cost[k];
			}
		}

		/// We can abandon early if the current cummulative distace with lower bound together are larger than bsf
		if (i + r < m - 1 && min_cost + cb[i + r + 1] >= bsf)
		{
			free(cost);
			free(cost_prev);
			return min_cost + cb[i + r + 1];
		}

		/// Move current array to previous array.
		cost_tmp = cost;
		cost = cost_prev;
		cost_prev = cost_tmp;
	}
	k--;

	/// the DTW distance is in the last cell in the matrix of size O(m^2) or at the middle of our array.
	double final_dtw = cost_prev[k];
	free(cost);
	free(cost_prev);
	return final_dtw;
}

/// Print function for debugging
void printArray(double *x, int len)
{
	for (int i = 0; i<len; i++)
		printf(" %6.2lf", x[i]);
	printf("\n");
}

/// If expected error happens, teminated the program.
void error(int id)
{
	if (id == 1)
		printf("ERROR : Memory can't be allocated!!!\n\n");
	else if (id == 2)
		printf("ERROR : File not Found!!!\n\n");
	else if (id == 3)
		printf("ERROR : Can't create Output File!!!\n\n");
	else if (id == 4)
	{
		printf("ERROR : Invalid Number of Arguments!!!\n");
		printf("Command Usage:  UCR_DTW.exe  data-file  query-file   m   R\n\n");
		printf("For example  :  UCR_DTW.exe  data.txt   query.txt   128  0.05\n");
	}
	exit(1);
}

/// Main Function
//query_num， 短序列的编号
//M，短序列的长度 
double UCR(string Data_File, vector<char> query, int M, double R)
{
	FILE *fp;            /// data file pointer
	FILE *qp;            /// query file pointer
	double bsf;          /// best-so-far
	double *t, *q;       /// data array and query array
	int *order;          ///new order of the query
	double *u, *l, *qo, *uo, *lo, *tz, *cb, *cb1, *cb2, *u_d, *l_d;


	double d;
	long long i, j;
	double ex, ex2, mean, std;
	int m = -1, r = -1;
	long long loc = 0;
	double t1, t2;
	int kim = 0, keogh = 0, keogh2 = 0;
	double dist = 0, lb_kim = 0, lb_k = 0, lb_k2 = 0;
	double *buffer, *u_buff, *l_buff;
	Index *Q_tmp;

	/// For every EPOCH points, all cummulative values, such as ex (sum), ex2 (sum square), will be restarted for reducing the floating point error.
	int EPOCH = 100000;

	m = M;

	if (R <= 1)
		r = floor(R*m);
	else
		r = floor(R);

	fopen_s(&fp, Data_File.c_str(), "r");
	if (fp == NULL)
	{
		cout << "数据文件未打开" << endl;
		error(2);
	}

	/*fopen_s(&qp, Query_File.c_str(), "r");
	if (qp == NULL)
	error(2);*/

	/// start the clock
	t1 = clock();


	/// malloc everything here
	q = (double *)malloc(sizeof(double)*m);
	if (q == NULL)
		error(1);
	qo = (double *)malloc(sizeof(double)*m);
	if (qo == NULL)
		error(1);
	uo = (double *)malloc(sizeof(double)*m);
	if (uo == NULL)
		error(1);
	lo = (double *)malloc(sizeof(double)*m);
	if (lo == NULL)
		error(1);

	order = (int *)malloc(sizeof(int)*m);
	if (order == NULL)
		error(1);

	Q_tmp = (Index *)malloc(sizeof(Index)*m);
	if (Q_tmp == NULL)
		error(1);

	u = (double *)malloc(sizeof(double)*m);
	if (u == NULL)
		error(1);

	l = (double *)malloc(sizeof(double)*m);
	if (l == NULL)
		error(1);

	cb = (double *)malloc(sizeof(double)*m);
	if (cb == NULL)
		error(1);

	cb1 = (double *)malloc(sizeof(double)*m);
	if (cb1 == NULL)
		error(1);

	cb2 = (double *)malloc(sizeof(double)*m);
	if (cb2 == NULL)
		error(1);

	u_d = (double *)malloc(sizeof(double)*m);
	if (u == NULL)
		error(1);

	l_d = (double *)malloc(sizeof(double)*m);
	if (l == NULL)
		error(1);

	t = (double *)malloc(sizeof(double)*m * 2);
	if (t == NULL)
		error(1);

	tz = (double *)malloc(sizeof(double)*m);
	if (tz == NULL)
		error(1);

	buffer = (double *)malloc(sizeof(double)*EPOCH);
	if (buffer == NULL)
		error(1);

	u_buff = (double *)malloc(sizeof(double)*EPOCH);
	if (u_buff == NULL)
		error(1);

	l_buff = (double *)malloc(sizeof(double)*EPOCH);
	if (l_buff == NULL)
		error(1);


	/// Read query file
	bsf = INF;
	i = 0;
	j = 0;
	ex = ex2 = 0;

	//int qn = Query_Num;
	//cout << "qn:" << qn << endl;
	//目的是读取指定行

	//fseek(qp, fileSeekPos2, 0);

	//cout << qn<<endl;
	//！！！s的大小 
	//char s[1000];
	//string s;
	/*
	while (fscanf_s(qp, "(%c)+", s) != 0 && i < m)
	{
	cout << "ss" << endl;
	d = atoi(s);

	//检查是否读到了指定的短序列
	cout << d << '\t';
	ex += d;
	ex2 += d*d;
	q[i] = d;
	i++;
	}
	*/
	//char buf[5000];
	char *buf = (char *)malloc(sizeof(char) * 2000);
	//fgets(buf, 2000, qp);
	//char *buf = &query[0];
	//cout << "buf[0,1,2]" << buf[0] << buf[1] << buf[2] << endl;
	//cout << "buf[0,1,2]" << query[31] <<":" <<query[32] <<":"<< query[33] << endl;

	int a = 0;
	while (a < query.size())
	{
		//int b = 0;
		////p = &query[a];
		//while (query[a] != ',' && a < query.size())
		//{
		//	p[b++] = query[a++];
		//}
		//d = atoi(p);
		//cout << "d: " << d << endl;
		//ex += d;
		//ex2 += d*d;
		//q[i] = d;
		//i++;
		//a++;
		//free(p);
		//cout << a<<": "<<query[a]<<endl;
		buf[a] = query[a];
		a++;
	}
	//cout << a << endl;
	//cout << strlen(buf) << endl;
	char *ds = ",";
	char *p = nullptr;
	char *nextp = nullptr;
	p = strtok_s(buf, ds, &nextp);
	while (p != NULL)
	{
		d = atoi(p);
		//cout << "d:" << d << endl;
		ex += d;
		ex2 += d*d;
		q[i] = d;
		i++;
		p = strtok_s(nullptr, ds, &nextp);
	}
	//cout << "i: " << i << endl;
	free(p);
	//free(nextp);
	free(buf);
	//fclose(qp);

	/// Do z-normalize the query, keep in same array, q
	mean = ex / m;
	std = ex2 / m;
	std = sqrt(std - mean*mean);
	for (i = 0; i < m; i++)
		q[i] = (q[i] - mean) / std;

	/// Create envelop of the query: lower envelop, l, and upper envelop, u
	lower_upper_lemire(q, m, r, l, u);

	/// Sort the query one time by abs(z-norm(q[i]))
	for (i = 0; i<m; i++)
	{
		Q_tmp[i].value = q[i];
		Q_tmp[i].index = i;
	}
	qsort(Q_tmp, m, sizeof(Index), comp);

	/// also create another arrays for keeping sorted envelop
	for (i = 0; i<m; i++)
	{
		int o = Q_tmp[i].index;
		order[i] = o;
		qo[i] = q[o];
		uo[i] = u[o];
		lo[i] = l[o];
	}
	free(Q_tmp);

	/// Initial the cummulative lower bound
	for (i = 0; i<m; i++)
	{
		cb[i] = 0;
		cb1[i] = 0;
		cb2[i] = 0;
	}

	i = 0;          /// current index of the data in current chunk of size EPOCH
	j = 0;          /// the starting index of the data in the circular array, t
	ex = ex2 = 0;
	bool done = false;
	int it = 0, ep = 0, k = 0;
	long long I;    /// the starting index of the data in current chunk of size EPOCH

	while (!done)
	{
		/// Read first m-1 points
		ep = 0;
		if (it == 0)
		{
			for (k = 0; k<m - 1; k++)
				if (fscanf_s(fp, "%lf", &d) != EOF)
					buffer[k] = d;
		}
		else
		{
			for (k = 0; k<m - 1; k++)
				buffer[k] = buffer[EPOCH - m + 1 + k];
		}

		/// Read buffer of size EPOCH or when all data has been read.
		ep = m - 1;
		while (ep<EPOCH)
		{
			if (fscanf_s(fp, "%lf", &d) == EOF)
				break;
			buffer[ep] = d;
			ep++;
		}

		/// Data are read in chunk of size EPOCH.
		/// When there is nothing to read, the loop is end.
		if (ep <= m - 1)
		{
			done = true;
		}
		else
		{
			lower_upper_lemire(buffer, ep, r, l_buff, u_buff);

			/// Just for printing a dot for approximate a million point. Not much accurate.
			if (it % (1000000 / (EPOCH - m + 1)) == 0)
				//fprintf(stderr,".");

				/// Do main task here..
				ex = 0; ex2 = 0;
			for (i = 0; i<ep; i++)
			{
				/// A bunch of data has been read and pick one of them at a time to use
				d = buffer[i];

				/// Calcualte sum and sum square
				ex += d;
				ex2 += d*d;

				/// t is a circular array for keeping current data
				t[i%m] = d;

				/// Double the size for avoiding using modulo "%" operator
				t[(i%m) + m] = d;

				/// Start the task when there are more than m-1 points in the current chunk
				if (i >= m - 1)
				{
					mean = ex / m;
					std = ex2 / m;
					std = sqrt(std - mean*mean);

					/// compute the start location of the data in the current circular array, t
					j = (i + 1) % m;
					/// the start location of the data in the current chunk
					I = i - (m - 1);

					/// Use a constant lower bound to prune the obvious subsequence
					lb_kim = lb_kim_hierarchy(t, q, j, m, mean, std, bsf);

					if (lb_kim < bsf)
					{
						/// Use a linear time lower bound to prune; z_normalization of t will be computed on the fly.
						/// uo, lo are envelop of the query.
						lb_k = lb_keogh_cumulative(order, t, uo, lo, cb1, j, m, mean, std, bsf);
						if (lb_k < bsf)
						{
							/// Take another linear time to compute z_normalization of t.
							/// Note that for better optimization, this can merge to the previous function.
							for (k = 0; k<m; k++)
							{
								tz[k] = (t[(k + j)] - mean) / std;
							}

							/// Use another lb_keogh to prune
							/// qo is the sorted query. tz is unsorted z_normalized data.
							/// l_buff, u_buff are big envelop for all data in this chunk
							lb_k2 = lb_keogh_data_cumulative(order, tz, qo, cb2, l_buff + I, u_buff + I, m, mean, std, bsf);
							if (lb_k2 < bsf)
							{
								/// Choose better lower bound between lb_keogh and lb_keogh2 to be used in early abandoning DTW
								/// Note that cb and cb2 will be cumulative summed here.
								if (lb_k > lb_k2)
								{
									cb[m - 1] = cb1[m - 1];
									for (k = m - 2; k >= 0; k--)
										cb[k] = cb[k + 1] + cb1[k];
								}
								else
								{
									cb[m - 1] = cb2[m - 1];
									for (k = m - 2; k >= 0; k--)
										cb[k] = cb[k + 1] + cb2[k];
								}

								/// Compute DTW and early abandoning if possible
								dist = dtw(tz, q, cb, m, r, bsf);

								if (dist < bsf)
								{   /// Update bsf
									/// loc is the real starting location of the nearest neighbor in the file
									bsf = dist;
									loc = (it)*(EPOCH - m + 1) + i - m + 1;
								}
							}
							else
								keogh2++;
						}
						else
							keogh++;
					}
					else
						kim++;

					/// Reduce obsolute points from sum and sum square
					ex -= t[j];
					ex2 -= t[j] * t[j];
				}
			}

			/// If the size of last chunk is less then EPOCH, then no more data and terminate.
			if (ep<EPOCH)
				done = true;
			else
				it++;
		}
	}

	i = (it)*(EPOCH - m + 1) + ep;
	fclose(fp);

	free(q);
	free(u);
	free(l);
	free(uo);
	free(lo);
	free(qo);
	free(cb);
	free(cb1);
	free(cb2);
	free(tz);
	free(t);
	free(l_d);
	free(u_d);
	free(l_buff);
	free(u_buff);

	free(buffer);//在病毒文件过多时这个太耗内存了 要注释掉！！
	free(order);
	t2 = clock();
	//printf("\n");

	/// Note that loc and i are long long.

	//    cout << "Location : " << loc << endl;
	//    cout << "Distance : " << sqrt(bsf) << endl;
	//    cout << "Data Scanned : " << i << endl;
	//    cout << "Total Execution Time : " << (t2-t1)/CLOCKS_PER_SEC << " sec" << endl;

	/// printf is just easier for formating ;)
	//    printf("\n");
	//    printf("Pruned by LB_Kim    : %6.2f%%\n", ((double) kim / i)*100);
	//    printf("Pruned by LB_Keogh  : %6.2f%%\n", ((double) keogh / i)*100);
	//    printf("Pruned by LB_Keogh2 : %6.2f%%\n", ((double) keogh2 / i)*100);
	//    printf("DTW Calculation     : %6.2f%%\n", 100-(((double)kim+keogh+keogh2)/i*100));
	//cout << bsf << endl;
	return sqrt(bsf);
}

//获取目录下所有文件
void getFiles(string path, vector<string>& files)
{
	//文件句柄  
	long   hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之  
			//如果不是,加入列表  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

//void memMap(SYSTEM_INFO &mysinf, __int64 &qwFileSize, __int64 &qwFileOffSet, HANDLE &hFileMapping, int th_mynum, vector<vector<char>> &buf, vector<char> &tmp)
//{
//	////TCHAR * file_name = TEXT("D:\\study files\\Data Mining\\DNA\RAW_DATA\\test\\test2.txt");;
//	//SYSTEM_INFO sinf;
//	//GetSystemInfo(&sinf);
//	//// Open the file that we want to map.
//	//HANDLE hFile = ::CreateFile(L"D:\\data\\C.txt",
//	//	GENERIC_READ,
//	//	FILE_SHARE_READ,
//	//	NULL,
//	//	OPEN_EXISTING,
//	//	FILE_FLAG_SEQUENTIAL_SCAN,
//	//	NULL);
//
//	//// Create a file-mapping object for the file.
//	//HANDLE hFileMapping = ::CreateFileMapping(hFile,
//	//	NULL,
//	//	PAGE_READONLY,
//	//	0, 0,
//	//	NULL);
//
//	//DWORD dwFileSizeHigh;
//	//__int64 qwFileSize = GetFileSize(hFile, &dwFileSizeHigh);
//	//qwFileSize += (((__int64)dwFileSizeHigh) << 32);
//	//::CloseHandle(hFile);
//
//	////cout << qwFileSize << endl;
//	//__int64 qwFileOffSet = 0;
//	////string *buf = (string*)malloc(sizeof(string)*10) ;
//	////char *buf = (char *)malloc(sizeof(char) * 2000);
//
//	//int num = 0;
//	//vector<char> tmp;
//	//while (qwFileSize>0){
//
//	DWORD dwBytesInBlock = 1024 * mysinf.dwAllocationGranularity;
//	//size_t dwBytesInBlock = 0;
//	if (qwFileSize < 1024 * mysinf.dwAllocationGranularity)
//		dwBytesInBlock = (DWORD)qwFileSize;
//	//cout << dwBytesInBlock / 1024 << endl;
//
//	PBYTE pbFile = (PBYTE)MapViewOfFile(hFileMapping, FILE_MAP_READ,
//		(DWORD)(qwFileOffSet >> 32),
//		(DWORD)(qwFileOffSet & 0xFFFFFFFF),
//		dwBytesInBlock);
//
//	//cout << pbFile << endl;
//	/*
//	char ** buf = new char *[1024 * 64];
//	for (int i = 0; i < 1024 * 64; i++)
//	{
//	buf[i] = new char[3000];
//	}
//	*/
//	//char**  buf = (char **)malloc(sizeof(char *) * 1024 * 64);
//	/*for (int i = 0; i<1024 * 64; i++)
//	buf[i] = (char *)malloc(sizeof(char) * 3000);
//	int k = 0;*/
//	//将上次映射的块的最后一行数据读取出来放在buf的第一行中，之后由k开始继续存放下次映射的块中的内容
//	int k = 0;
//
//	vector<char> buftmp;
//	if (th_mynum != 0 && tmp.size() != 0)
//	{
//		for (vector<char>::iterator it = tmp.begin(); it != tmp.end(); it++)
//		{
//			buftmp.push_back(*it);
//			k++;
//		}
//		vector<char>().swap(tmp);	//释放掉tmp
//	}
//
//	//vector<char>().swap(buftmp);
//
//	int i = 0, j = 0;
//	while (j < dwBytesInBlock)
//	{
//
//		if (pbFile[j] == '\n')
//		{
//			buftmp.push_back('\n');
//			buf.push_back(buftmp);
//			vector<char>().swap(buftmp);
//			//buf[i][k] = '\n';
//			i++;
//			j++;
//			k = 0;
//		}
//		else
//		{
//			buftmp.push_back(pbFile[j]);
//			//buf[i][k] = pbFile[j];
//			j++;
//			k++;
//		}
//	}
//	buf.push_back(buftmp);
//	vector<char>().swap(buftmp);
//
//	//tmp[i]表示该块中最后一行的序列（可能只读取了文件中对应行的前部分序列）， 现在将其取出，放在tmp中，供下次映射文件块时使用
//	for (int a = 0; a < k; a++)
//	{
//		tmp.push_back(buf[i][a]);
//	}
//	::UnmapViewOfFile(pbFile);
//	qwFileOffSet += dwBytesInBlock;
//	qwFileSize -= dwBytesInBlock;
//
//	//system("pause");
//	//}
//
//	return;
//}

void myThread(vector<string> &files, int st, double** &thAns)
{
	//cout << files.size() << endl;
	SYSTEM_INFO mysinf;
	GetSystemInfo(&mysinf);
	// Open the file that we want to map.
	HANDLE hFile = ::CreateFile(L"D:\\data\\C.txt",
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);

	// Create a file-mapping object for the file.
	HANDLE hFileMapping = ::CreateFileMapping(hFile,
		NULL,
		PAGE_READONLY,
		0, 0,
		NULL);

	DWORD dwFileSizeHigh;
	__int64 qwFileSize = GetFileSize(hFile, &dwFileSizeHigh);
	qwFileSize += (((__int64)dwFileSizeHigh) << 32);
	::CloseHandle(hFile);

	//cout << qwFileSize << endl;
	__int64 qwFileOffSet = 0;


	//char **buf;
	vector<vector<char>> buf;
	//buf = memMap(3844948, mysinf, qwFileSize, qwFileOffSet, hFileMapping);
	int totalnum = 3844948;

	int th_mynum = 0;
	vector<char> tmp;
	//memMap(mysinf, qwFileSize, qwFileOffSet, hFileMapping, th_mynum, buf, tmp);

	//不用函数memMap  多线程可能会存在互斥情况
	DWORD dwBytesInBlock = 1024 * mysinf.dwAllocationGranularity;
	//size_t dwBytesInBlock = 0;
	if (qwFileSize < 1024 * mysinf.dwAllocationGranularity)
		dwBytesInBlock = (DWORD)qwFileSize;
	//cout << dwBytesInBlock / 1024 << endl;
	PBYTE pbFile = (PBYTE)MapViewOfFile(hFileMapping, FILE_MAP_READ,
		(DWORD)(qwFileOffSet >> 32),
		(DWORD)(qwFileOffSet & 0xFFFFFFFF),
		dwBytesInBlock);
	//将上次映射的块的最后一行数据读取出来放在buf的第一行中，之后由k开始继续存放下次映射的块中的内容
	int k1 = 0;
	vector<char> buftmp;
	if (th_mynum != 0 && tmp.size() != 0)
	{
		for (vector<char>::iterator it = tmp.begin(); it != tmp.end(); it++)
		{
			buftmp.push_back(*it);
			k1++;
		}
		vector<char>().swap(tmp);	//释放掉tmp
	}
	//vector<char>().swap(buftmp);
	int i1 = 0, j1 = 0;
	while (j1 < dwBytesInBlock)
	{

		if (pbFile[j1] == '\n')
		{
			buftmp.push_back('\n');
			buf.push_back(buftmp);
			vector<char>().swap(buftmp);
			//buf[i][k] = '\n';
			i1++;
			j1++;
			k1 = 0;
		}
		else
		{
			buftmp.push_back(pbFile[j1]);
			//buf[i][k] = pbFile[j];
			j1++;
			k1++;
		}
	}
	buf.push_back(buftmp);
	vector<char>().swap(buftmp);
	//tmp[i]表示该块中最后一行的序列（可能只读取了文件中对应行的前部分序列）， 现在将其取出，放在tmp中，供下次映射文件块时使用
	for (int a = 0; a < k1; a++)
	{
		tmp.push_back(buf[i1][a]);
	}
	::UnmapViewOfFile(pbFile);
	qwFileOffSet += dwBytesInBlock;
	qwFileSize -= dwBytesInBlock;
	//映射第一块数据完毕

	int tmpCount = 0;	//记录当前块的行数
	if (qwFileSize > 0)
	{
		th_mynum = th_mynum + buf.size() - 1;	//更新mynum至当前块的倒数第二行
		tmpCount = buf.size() - 1;
	}
	else
	{
		th_mynum = th_mynum + buf.size();	//说明已经映射到最后一块
		tmpCount = buf.size();
	}
	

	//读取len到double数组ans
	double * ans = (double*)malloc(sizeof(double) * totalnum);
	ifstream in("D:\\data\\test1_len.txt");
	char buffer[256];
	ifstream myfile("D:\\data\\C_len.txt");

	if (!myfile){
		cout << "Unable to open myfile";
		exit(1); // terminate with error

	}
	//if (!outfile){
	//	cout << "Unable to open otfile";
	//	exit(1); // terminate with error
	//}
	int i2 = 0;
	while (!myfile.eof() && i2 < totalnum)
	{
		myfile.getline(buffer, 10);
		//cout << atoi(buffer) << endl;
		ans[i2++] = atoi(buffer);
	}

	int mycount = st;
	double R = 0.05;
	while (mycount <= totalnum)
	{
		long start1 = clock();
		int M = ans[mycount - 1];
		if (mycount > th_mynum)  //需要读取的行号比当前得到的块的最大行号大，则说明块数不够，需继续向下读取块
		{
			while (mycount > th_mynum)
			{
				vector<vector<char>>().swap(buf);	//释放buf
				//tmpCount = 0;
				//memMap(mysinf, qwFileSize, qwFileOffSet, hFileMapping, th_mynum, buf, tmp);

				//同样，映射一块数据
				//DWORD dwBytesInBlock = 1024 * mysinf.dwAllocationGranularity;
				if (qwFileSize < 1024 * mysinf.dwAllocationGranularity)
					dwBytesInBlock = (DWORD)qwFileSize;
				//cout << dwBytesInBlock / 1024 << endl;

				PBYTE pbFile = (PBYTE)MapViewOfFile(hFileMapping, FILE_MAP_READ,
					(DWORD)(qwFileOffSet >> 32),
					(DWORD)(qwFileOffSet & 0xFFFFFFFF),
					dwBytesInBlock);
				//将上次映射的块的最后一行数据读取出来放在buf的第一行中，之后由k开始继续存放下次映射的块中的内容
				int k = 0;
				vector<char> buftmp;
				if (th_mynum != 0 && tmp.size() != 0)
				{
					for (vector<char>::iterator it = tmp.begin(); it != tmp.end(); it++)
					{
						buftmp.push_back(*it);
						k++;
					}
					vector<char>().swap(tmp);	//释放掉tmp
				}
				//vector<char>().swap(buftmp);
				int i = 0, j = 0;
				while (j < dwBytesInBlock)
				{

					if (pbFile[j] == '\n')
					{
						buftmp.push_back('\n');
						buf.push_back(buftmp);
						vector<char>().swap(buftmp);
						//buf[i][k] = '\n';
						i++;
						j++;
						k = 0;
					}
					else
					{
						buftmp.push_back(pbFile[j]);
						//buf[i][k] = pbFile[j];
						j++;
						k++;
					}
				}
				buf.push_back(buftmp);
				vector<char>().swap(buftmp);

				//tmp[i]表示该块中最后一行的序列（可能只读取了文件中对应行的前部分序列）， 现在将其取出，放在tmp中，供下次映射文件块时使用
				for (int a = 0; a < k; a++)
				{
					tmp.push_back(buf[i][a]);
				}
				::UnmapViewOfFile(pbFile);
				qwFileOffSet += dwBytesInBlock;
				qwFileSize -= dwBytesInBlock;


				if (qwFileSize > 0)
				{
					th_mynum = th_mynum + buf.size() - 1;	//更新mynum至当前块的倒数第二行
					tmpCount = buf.size() - 1;
				}
				else
				{
					th_mynum = th_mynum + buf.size();	//说明已经映射到最后一块
					tmpCount = buf.size();
				}
			}
		}

		//while循环结束说明可以在本块数据中找到对应的需要读取的行号，对应的行号为 mycount + tmpCount - 1 - mynum(根据 a + mynum - tmpCount + 1 == mycount 得到)
		//将当前短序列与所有病毒细菌文件匹配， UCR

		//非多线程
		//cout << "The 第 " << mycount << " 次 running" << endl;
		int j = 0;
		for (; j < files.size(); j++)
		{
			//cout << UCR(files[j], buf[mycount + tmpCount - 1 - mynum], M, R) << '\t';
			//outfile << UCR(files[j], buf[mycount + tmpCount - 1 - mynum], M, R);
			//outfile << '\t';
			//cout << '\t' << "第 " << j << " 个细菌病毒文件" << endl;
			thAns[mycount - 1][j] = UCR(files[j], buf[mycount + tmpCount - 1 - th_mynum], M, R);

		}
		//cout << "The 第 " << mycount << " 次 running" << endl;
		//cout << UCR(files[j], buf[mycount + tmpCount - 1 - mynum], M, R) << endl;
		//outfile << UCR(files[j], buf[mycount + tmpCount - 1 - mynum], M, R) << endl;

		//多线程来实现
		//cout << files.size() << endl;
		//double *ans;
		//ans = (double*)malloc(sizeof(double)*files.size());
		//thread a0(myThread, files, buf[mycount + tmpCount - 1 - mynum], M, R, 0, ans);
		//thread a1(myThread, files, buf[mycount + tmpCount - 1 - mynum], M, R, 1, ans);
		//thread a2(myThread, files, buf[mycount + tmpCount - 1 - mynum], M, R, 2, ans);
		//thread a3(myThread, files, buf[mycount + tmpCount - 1 - mynum], M, R, 3, ans);
		////thread a4(myThread, files, buf[mycount + tmpCount - 1 - mynum], M, R, 4, ans);
		//a0.join();
		//a1.join();
		//a2.join();
		//a3.join();
		////a4.join();
		//int j = 0;
		//for (; j < files.size() - 1; j++)
		//{
		//	outfile << ans[j] << '\t';
		//}
		//outfile << ans[j] << endl;
		//free(ans);


		long finish1 = clock();
		cout << "The 第 " << mycount << " 次 running time is: " << (double)(finish1 - start1) / CLOCKS_PER_SEC / 60.0 << endl;
		mycount = mycount + 3;	//三个线程
		if (mycount > 6) break;		//预先设置 mycount的最大值（即最多读取多少行序列） 提前跳出程序
	}
	free(ans);
	::CloseHandle(hFileMapping);
}

int _tmain(int argc, _TCHAR* argv[])
{
	long start, finish;
	start = clock();


	ofstream outfile("D:\\data\\C_out.txt");
	
	//fprintf(UCR(files[j], qn, i, M, R));

	//读取所有的病毒/细菌文件
	vector<string> files;
	/*char * filePath = "D:\\study files\\Data Mining\\DNA\\RAW_DATA\\test\\data3";
	getFiles(filePath, files);*/

	string filepath = "D:\\study files\\Data Mining\\DNA\\RAW_DATA\\queryOrder.txt";
	ifstream infile("D:\\study files\\Data Mining\\DNA\\RAW_DATA\\queryOrder.txt");
	string fileBuf;
	while (!infile.eof())
	{
		//fileBuf = infile.getline;
		getline(infile, fileBuf);
		fileBuf = "F:tmp\\Bac_Vir_txt\\" + fileBuf;
		files.push_back(fileBuf);
		//cout << fileBuf << endl;
	}
	cout << files.size() << endl;
	
	//多线程实现 一个线程负责多个短序列
	
	int totalnum = 8;
	double **thAns = (double**)malloc(sizeof(double*)*totalnum);
	for (int j = 0; j < totalnum; j++)
	{
		thAns[j] = (double*)malloc(sizeof(double)*files.size());
	}
	cout << "running" << endl;
	thread a1(myThread, files, 1, thAns);
	thread a2(myThread, files, 2, thAns);
	thread a3(myThread, files, 3, thAns);
	a1.join();
	a2.join();
	a3.join();
	cout << "running" << endl;
	for (int j = 0; j < 6; j++)
	{
		int k = 0;
		for (; k < files.size() - 1; k++)
		{
			//cout << thAns[j][k] << '\t';
			outfile << thAns[j][k] << '\t';
		}
		//cout << thAns[j][k] << endl;
		outfile << thAns[j][k] << endl;
	}
	//free(ans);
	outfile.close();

	
	finish = clock();
	cout << "The running time is: " << (double)(finish - start) / CLOCKS_PER_SEC / 60.0 << endl;
	return 0;
}








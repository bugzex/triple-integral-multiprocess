// int3.c - вычисление тройного интеграла с использованием нитей.

#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

double func(double x,double y, double z);
double integrate(double a, double b, double c, double d, double e, double f, int N, int M, int K);
void* proc(void *);
void* pthstat(void *);

double a, b, c, d, e, f, N, M, K;									// параметры интегрирования
unsigned pthnum;													// количество нитей
double result = 0.0;												// рассчитанный интегралл
char *stat_list;													// строка состояния вычисления
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_t *threads;													// нити-математики
pthread_t stat;														// нить надзиратель
unsigned stat_flg;																

int main(int argc, char* argv[])
{ 
  if(argc == 1) {
    puts("Usage: int3 [a] [b] [c] [d] [e] [f] [N] [M] [K] [pthreads number]");
    return 0;
  }
	
	sscanf(argv[1], "%lf", &a);
	sscanf(argv[2], "%lf", &b);
	sscanf(argv[3], "%lf", &c);
	sscanf(argv[4], "%lf", &d);
	sscanf(argv[5], "%lf", &e);
	sscanf(argv[6], "%lf", &f);
	N = atoi(argv[7]);
	M = atoi(argv[8]);
	K = atoi(argv[9]);
	stat_flg = pthnum = atoi(argv[10]);							
	time_t begtime, endtime, restime;							// время вычисления...
	int i;

  threads = (pthread_t *)malloc(sizeof(pthread_t) * pthnum);
  stat_list = (char*)calloc(pthnum, sizeof(char));
  
	// '+' - задание выполнено, '-' - нет.
	// по началу ни одно не выполнено...
	for(i = 0; i < pthnum; i++)
    stat_list[i] = '-';
  
  begtime = time(NULL);
	pthread_create(&stat, 0, pthstat, (void*)0);
  for(i = 0; i < pthnum; i++)
    pthread_create(threads + i, 0, proc, (void *)i);
  
  pthread_join(stat, NULL);
	for(i = 0; i < pthnum; i++)
    pthread_join(threads[i], NULL);
 
  endtime = time(NULL);
  restime = endtime - begtime;
  
  printf("Result = %lf\nCalculating time = %d sec\n", result, restime);
  return 0;
}

double func(double x,double y, double z)
{
  double t, r;
  if((t = sqrt(x*x + y*y + z*z)) > 1)
    r = 0.5;
  if(t > 2)
    r = 0;
  if(t < 0.5)
    r+= 0.5 * sin(t);
  return r;
}

double integrate(double a, double b, double c, double d, double e, double f, int N, int M, int K)
{
  double hx, hy, hz;
  double x, y, z;
  double res = 0; 
  int i, j, k;

  hx = (b-a)/N;
  hy = (d-c)/M;
  hz = (f-e)/K;

  for(i = 1; i < N; i++)
	{
		x = a + i * hx + 0.5 * hx;
    for(j = 1; j < M; j++)
		{
      y = c + j * hy + 0.5 * hy;
      for(k = 1; k < K; k++)
			{
        z = e + k * hz + 0.5 * hz;
        res+= func(x, y, z) * hx * hy * hz;
      }
    }
  }
  
	return res;
}

void* proc(void *r)
{ 
  int rank = (int)r;
	int n;
  double A, B, integral, len;
  
	len = (b - a)/pthnum;
  n = N/pthnum;
  A = a + len * rank;
  B = A + len;
  
	integral = integrate(A, B, c, d, e, f, n, M, K);
  pthread_mutex_lock(&mut);
  result+= integral;
  pthread_mutex_unlock(&mut);
  
	stat_list[rank] = '+';
	--stat_flg;
  return 0;
}

void* pthstat(void *arg)
{
	unsigned i = stat_flg;
	while(stat_flg > 0)
		if(stat_flg == i)
		{
			printf("%s\n", stat_list);
			--i;
		}
  
  return 0;
}
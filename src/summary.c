/*
 *  summary : an R like five number summary
 *  
 * February 2011
 * dent earl, dearl (a) soe ucsc edu
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.gnu.org/licenses/gpl-2.0.html
 */
#include <assert.h>
#include <limits.h> /* INT_MAX, LINE_MAX */
#include <math.h>  /* pow() */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int debug_flag;
int verbose_flag;
float my_version_number = 0.1;
char *my_version_date = "1 February 2011";

struct doubleList {
   double d;
   struct doubleList *next;
};
typedef struct doubleList doubleList;

void version(void){
   printf("summary version %.1lf, %s.\n", my_version_number, my_version_date);
   exit(0);
}

void usage(void){
   fprintf(stderr, "USAGE: summary takes input via stdin. --precision [0..9]\n\n");
   exit(2);
}

void memError(void){
   fprintf(stderr, "Error, unable to allocate memory. Exiting.\n");
   exit(2);
}

void help(void){
   printf( "  summary a program like the R function summary().                    \n"
           "  $ echo -e '112\\n98.1\\n' | summary                                 \n"
           "        n       Min.    1st Qu.     Median       Mean    3rd Qu.       Max.     Stdev.\n"
           "        2    98.1000   101.5750   105.0500   105.0500   108.5250   112.0000     9.8288\n"
           "  stdin: one number on each line of input, no puntuation, text or     \n"
           "  blank lines.  \n"
           "  --precision [0..9] adjusts the number of decimals.\n"
           "  Quartiles Method:\n"
           "  We use R type 7, (in R try ?stats::quantile)\n"
           "  Type 7 m = 1-p.  p[k] = (k - 1) / (n - 1).  In this case, p[k] =\n"
           "  mode[F(x[k])].  This is used by S.\n"
           );
   exit(0);
}
void gatherOptions(int argc, char **argv, int *prec )
{
   int c;
   while (1)
      {
         static struct option long_options[] =
            {
               {"debug", no_argument, &debug_flag, 1},
               {"verbose", no_argument, &verbose_flag, 1},
               {"help", no_argument, 0, 'h'},
               {"version", no_argument, 0, 'v'},
               {"precision", required_argument, 0, 'p'},
               {0, 0, 0, 0}
            };
         /* getopt_long stores the option index here. */
         int option_index = 0;
         c = getopt_long(argc, argv, "p:",
                         long_options, &option_index);
         /* Detect the end of the options. */
         if (c == -1){
            break;
         }
         switch (c)
            {
            case 0:
               break;
            case 'p':
               sscanf( optarg, "%d", prec );
               break;
            case 'h':
               help();
            case 'v':
               version();
            case '?':
               /* getopt_long already printed an error message. */
               break;
            default:
               abort ();
            }
      }
   if ((*prec < 0) || (*prec > 9 ))
      usage();
}

void initList(doubleList *dl){
   dl = (doubleList *)malloc(sizeof(doubleList));
   if (dl == NULL)
      memError();
   dl -> d = 0.0;
   dl -> next = NULL;
}

void addToList(doubleList **dl, double data){
   doubleList *p;
   p = (doubleList *)malloc(sizeof(doubleList));
   if (p==NULL)
      memError();
   p -> d = data;
   p -> next = *dl;
   *dl = p;
}

int countList(doubleList *dl){
   if (dl == NULL)
      return 0;
   else
      return (1+countList(dl->next));
}

void releaseList(doubleList *dl){
   if (dl != NULL){
      releaseList(dl->next);
      free(dl);
   }
}

double * listToArray(doubleList *dl, int n){
   int i;
   double *a = NULL;
   doubleList *p;
   a = calloc(n, sizeof(double));
   if ( a == NULL )
      memError();
   p = dl;
   for( i=0; i < n; i++ ){
      a[i] = p->d;
      p = p->next;
   }
   return a;
}

void printList(doubleList *dl){
   if (dl != NULL){
      printf("%lf%s", dl->d, (dl->next == NULL)?"\n":", " );
      printList(dl->next);
   }
}

int dbl_cmp(const void *a, const void *b){
   const double *da = (const double *)a;
   const double *db = (const double *)b;
   if ( *da > *db )
      return 1;
   else
      if ( *da < *db )
         return -1;
      else
         return 0;
}

void printArray( double *a, int n ){
   int i;
   for (i = 0; i < n; ++i)
      printf("%lf, ", a[i] );
   printf("\n");
}

double median( double *x, int n , int s ){
   /* s is the starting point in the array to consider
      this allows us to more easily find Q1 and Q3
   */
   if ( n == 1 )
      return x[ s ];
   if ( (n-s) % 2 )
      return ( x[ s + (((n - s) - 1) / 2 ) ] );
   else
      return ( ( x[ s + ((n - s) / 2) ] + x[ s + ( (n-s) / 2 ) - 1 ] ) / 2.0 );
}

void copyArraySubset( double *x, double *y, int start, int end){
   int i, j=0;
   for (i = start; i <= end; ++i)
      y[ j++ ] = x [ i ];
}

double Q1( double *x, int n ){
   /* Q1 uses the median of the lower half approach
      to find the first quartile.
   */
   if ( n % 2 )
      return median( x, (n-1)/2, 0 );
   else{
      double *a = NULL;
      a = calloc( (n/2) + 1, sizeof(double));
      if ( a == NULL )
         memError();
      copyArraySubset( x, a, 0, n/2 );
      a[ n/2 + 1] = median( x, n, 0);
      return median( a, n/2 + 1, 0);
   }
}

double Q3( double *x, int n ){
   /* Q3 uses the median of the upper half approach
      to find the third quartile.
    */
   if ( n % 2 ){
      return median( x, n, (n-1)/2 );
   }else{
      double *a = NULL;
      a = calloc( (n/2) + 1, sizeof(double));
      if ( a == NULL )
         memError();
      copyArraySubset( x, a, n/2, n );
      a[ n/2 + 1] = median( x, n, 0);
      return median( a, n/2 + 1, 0);
   }
}

double quantile( double *x, int n, double p){
   /* Q(p) = (1 - gamma) * x[j] + gamma * x[j+1],
      where ( j - m ) / n <= p < ( j - m + 1 ) / n, x[j] is the 
      jth order statistic, n is the sample size,
      the value of gamma is a function of j = floor(np + m)
      and g = n * p + m - j.
      m = 1 - p. p_k = (k-1) / (n-1). Used by the stats
      packages R and S.
    */
   // this code is refactored from the R function summary.R
   double index = (n - 1.0) * p; // R version : 1.0 + (n - 1.0) * p
   int lo = floor( index );
   int hi = ceil( index );
   double qs = x[ lo ];
   double h = index - lo;
   qs = (1 - h) * qs + h * x[ hi ];
   return qs;
}

double sampStdev( double *x, int n, double ave){
   double s=0;
   int i;
   if (n < 2)
      return -1.0;
   for (i = 0; i < n; ++i)
      s += pow( ( x[i] - ave ), 2);
   s /= ( n - 1 );
   return sqrt( s );
}

void summarize( double *x, int n, double ave, int prec ){
   qsort( x, n, sizeof(double), dbl_cmp );
   printf( "%9s  %9s  %9s  %9s  %9s  %9s  %9s  %9s\n", 
           "n", "Min.", "1st Qu.", "Median", "Mean", "3rd Qu.", 
           "Max.", "Stdev." );
   printf( "%9d  %9.*f  %9.*f  %9.*f  %9.*f  %9.*f  %9.*f  %9.*f\n", 
           n, prec, x[0], prec, quantile( x, n, 0.25 ), prec, median(x, n, 0),
           prec, ave, prec, quantile( x, n, 0.75 ), 
           prec, x[n-1], prec, sampStdev(x, n, ave) );
}

int main (int argc, char **argv)
{
   double ave = 0.0;
   double d;
   int n = 0;
   int prec = 4;
   gatherOptions( argc, argv, &prec );
   doubleList *list = NULL;
   initList( list );
   while ( scanf("%lf", &d) == 1 ){
      addToList( &list, d );
      ave += ( d - ave ) / ++n;
   }
   double *array;
   array = listToArray( list, n );
   releaseList( list );
   summarize( array, n, ave, prec );
   free( array );
   return 0;
}

/*
 *  summary : an R like "five" number summary
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
 *
 *  Thanks and credit are due to Ivan Frohne and Rob J Hyndman
 *  who wrote quantile() in R.
 *  References:
 *
 *   Becker, R. A., Chambers, J. M. and Wilks, A. R. (1988) _The New S
 *   Language_.  Wadsworth & Brooks/Cole.
 *
 *   Hyndman, R. J. and Fan, Y. (1996) Sample quantiles in statistical
 *   packages, _American Statistician_, *50*, 361-365.
 *
THE FOLLOWING TEXT WAS REMOVED FROM THE QUANTILE DOCUMENTATION IN R:

quantile returns estimates of underlying distribution quantiles based on one 
or two order statistics from the supplied elements in x at probabilities in 
probs. One of the nine quantile algorithms discussed in Hyndman and Fan 
(1996), selected by type, is employed.

All sample quantiles are defined as weighted averages of consecutive order 
statistics. Sample quantiles of type i are defined by:

Q[i](p) = (1 - gamma) x[j] + gamma x[j+1],

where 1 <= i <= 9, (j-m)/n <= p < (j-m+1)/n, x[j] is the jth order statistic, 
n is the sample size, the value of γ is a function of j = floor(np + m) and
g = np + m - j, and m is a constant determined by the sample quantile type.

Discontinuous sample quantile types 1, 2, and 3

For types 1, 2 and 3, Q[i](p) is a discontinuous function of p, with m = 0 
when i = 1 and i = 2, and m = -1/2 when i = 3.

Type 1
Inverse of empirical distribution function. gamma = 0 if g = 0, and 1 otherwise.
Type 2
Similar to type 1 but with averaging at discontinuities. gamma = 0.5 if g = 0, 
and 1 otherwise.
Type 3
SAS definition: nearest even order statistic. gamma = 0 if g = 0 and j is even, 
and 1 otherwise.

Continuous sample quantile types 4 through 9

For types 4 through 9, Q[i](p) is a continuous function of p, with gamma = g
and m given below. The sample quantiles can be obtained equivalently by linear 
interpolation between the points (p[k],x[k]) where x[k] is the kth order 
statistic. Specific expressions for p[k] are given below.

Type 4
m = 0. p[k] = k / n. That is, linear interpolation of the empirical cdf.
Type 5
m = 1/2. p[k] = (k - 0.5) / n. That is a piecewise linear function where 
the knots are the values midway through the steps of the empirical cdf. 
This is popular amongst hydrologists.
Type 6
m = p. p[k] = k / (n + 1). Thus p[k] = E[F(x[k])]. This is used by Minitab 
and by SPSS.
Type 7 [ the default ]
m = 1-p. p[k] = (k - 1) / (n - 1). In this case, p[k] = mode[F(x[k])]. 
This is used by S.
Type 8
m = (p+1)/3. p[k] = (k - 1/3) / (n + 1/3). Then p[k] =~ median[F(x[k])]. 
The resulting quantile estimates are approximately median-unbiased regardless 
of the distribution of x.
Type 9
m = p/4 + 3/8. p[k] = (k - 3/8) / (n + 1/4). The resulting quantile estimates 
are approximately unbiased for the expected order statistics if x is normally 
distributed.
Further details are provided in Hyndman and Fan (1996) who recommended 
type 8. The default method is type 7, as used by S and by R < 2.0.0.
 */
#include <assert.h>
#include <limits.h> /* INT_MAX, LINE_MAX */
#include <float.h> /* DBL_EPSILON */
#include <math.h>  /* pow() */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int debug_flag;
int verbose_flag;
char *my_version_number = "0.1.1";
char *my_version_date = "21 March 2011";

struct doubleList {
   double d;
   struct doubleList *next;
};
typedef struct doubleList doubleList;

void version(void){
   printf("summary version %s, %s.\n", my_version_number, my_version_date );
   exit(0);
}

void usage(void){
   fprintf(stderr, "USAGE: summary takes input via stdin, one number per line. --precision [0..9] --type [1..9]\n\n");
   exit(2);
}

void memError(void){
   fprintf(stderr, "Error, unable to allocate memory. Exiting.\n");
   exit(2);
}

void help(void){
   printf( "  summary an R-like five number (plus two) summary.\n"
           "  Version %s\n"
           "  %s\n"
           "    dent earl, dearl (a) soe ucsc edu\n\n"
           "  This program is free software; you can redistribute it and/or modify\n"
           "  it under the terms of the GNU General Public License as published by\n"
           "  the Free Software Foundation; either version 2 of the License, or\n"
           "  (at your option) any later version.\n\n"
           "  This program is distributed in the hope that it will be useful,\n"
           "  but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
           "  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
           "  GNU General Public License for more details.\n\n"
           "  You should have received a copy of the GNU General Public License\n"
           "  along with this program; if not, a copy is available at\n"
           "  http://www.gnu.org/licenses/gpl-2.0.html\n\n"
           "  Thanks and credit are due to Ivan Frohne and Rob J Hyndman\n"
           "  who wrote quantile() in R.\n"
           "  References:\n\n"
           "   Becker, R. A., Chambers, J. M. and Wilks, A. R. (1988) _The New S\n"
           "   Language_.  Wadsworth & Brooks/Cole.\n\n"
           "   Hyndman, R. J. and Fan, Y. (1996) Sample quantiles in statistical\n"
           "   packages, _American Statistician_, *50*, 361-365.\n\n"
           "  USAGE:\n"
           "   $ echo -e '112\\n98.1\\n101\\n' | summary\n"
           "        n       Min.    1st Qu.     Median       Mean    3rd Qu.       Max.     Stdev.\n"
           "        3    98.1000    99.5500   101.0000   103.7000   106.5000   112.0000     7.3328\n\n"
           "  stdin: one number on each line of input, no puntuation, text or\n"
           "  blank lines.  \n\n"
           "  OPTIONS\n"
           "   --precision [0..9] adjusts the number of decimals. The default is 4.\n"
           "   --type [1..9] selects one of the nine quantile algorithms. The default is 7. For\n"
           "                 more information, inside of R try  ?stats::quantile to see details.\n\n",
           my_version_number, my_version_date
           );
   exit(0);
}
void gatherOptions(int argc, char **argv, int *prec, int *type )
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
               {"type", required_argument, 0, 't'},
               {0, 0, 0, 0}
            };
         /* getopt_long stores the option index here. */
         int option_index = 0;
         c = getopt_long(argc, argv, "p:t:",
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
            case 't':
               sscanf( optarg, "%d", type );
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
   if ((*prec < 0) || (*prec > 9 ) || (*type < 1) || (*type > 9))
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

void copyArrayIntoCenter( double *x, double *y, int n ){
   int i;
   for (i = 0; i < n; ++i)
      y[ i+2 ] = x [ i ];
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

double quantile( double *x, int n, double p, int type){
   /* Q(p) = (1 - gamma) * x[j] + gamma * x[j+1],
      where ( j - m ) / n <= p < ( j - m + 1 ) / n, x[j] is the 
      jth order statistic, n is the sample size,
      the value of gamma is a function of j = floor(np + m)
      and g = n * p + m - j.
      m = 1 - p. p_k = (k-1) / (n-1). Used by the stats
      packages R and S.
    */
   // this code is refactored from the R function summary.R
   double a, b, nppm, h, qs;
   int j;
   if ( type == 7 ){
      double index = (n - 1.0) * p; // R version : 1.0 + (n - 1.0) * p
      int lo = floor( index );
      int hi = ceil( index );
      qs = x[ lo ];
      h = index - lo;
      qs = (1 - h) * qs + h * x[ hi ];
      return qs;
   }else{
      if ( type <= 3 ){
         /* Types 1..3 are discontinuous sample quantiles */

         if (type == 3)
            nppm = n * p - 0.5;
         else
            nppm = n * p;
         j = floor( nppm );
         switch( type ){
         case 1:
            if ( nppm > j )
               h = 1.0;
            else
               h = 0.0;
            break;
         case 2:
            if ( nppm > j )
               h = 1.0;
            else
               h = 0.5;
            break;
         case 3:
            if (( nppm == j ) && ( j % 2 ) == 0 )
               h = 0.0;
            else
               h = 1.0;
            break;
         }
      }else{
         /* Types 4 through 9 are continuous sample quantiles */
         switch( type ){
         case 4:
            a = 0.0;
            b = 1.0;
            break;
         case 5:
            a = 0.5;
            b = 0.5;
            break;
         case 6:
            a = 0.0;
            b = 0.0;
            break;
         /* case 7 has alpha = beta = 1 */
         case 8:
            a = 1.0 / 3.0;
            b = 1.0 / 3.0;
            break;
         case 9:
            a = 3.0 / 8.0;
            b = 3.0 / 8.0;
            break;
         }
         double fuzz = 4.0 * DBL_EPSILON;
         nppm = a + p * ( n + 1 - a - b );
         j = floor( nppm + fuzz );
         h = (double)nppm - (double)j;
         if ( fabs(h) < fuzz )
            h = 0.0;
      }
      double *y = NULL;
      y = calloc( n + 4, sizeof(double));
      if ( y == NULL )
         memError();
      copyArrayIntoCenter( x, y, n );
      y[ 0 ]     = x[ 0 ];
      y[ 1 ]     = x[ 0 ];
      y[ n + 3 ] = x[ n ];
      y[ n + 4 ] = x[ n ];
      double qs = y[ j + 1 ];
      if ( h == 1.0)
         qs = y[ j + 2 ];
      if (( h > 0.0 ) && ( h < 1.0 ))
         qs = (1.0 - h ) * y[ j + 1 ] + h * y[ j + 2 ];
      return qs;
   }
}

double sampStdev( double *x, int n, double ave){
   double s=0.0;
   int i;
   if ( n < 2 )
      return -1.0;
   for ( i = 0; i < n; ++i )
      s += pow( ( x[i] - ave ), 2 );
   s /= ( n - 1 );
   return sqrt( s );
}

double sum( double *x, int n ){
   double a = 0.0;
   int i;
   for ( i = 0; i < n; ++i )
      a += x[ i ];
   return a;
}

void summarize( double *x, int n, double ave, int prec, int type ){
   double stdev = 0.0;
   char stdevStr[32];
   int ret;
   stdev = sampStdev( x, n, ave );
   if ( stdev > -1.0 )
      ret = sprintf( stdevStr, "%9.*f", prec, stdev );
   else
      ret = sprintf( stdevStr, "%s", "NA" );
   qsort( x, n, sizeof(double), dbl_cmp );
   printf( "%9s  %9s  %9s  %9s  %9s  %9s  %9s  %9s  %9s\n", 
           "n", "Min.", "1st Qu.", "Median", "Mean", "3rd Qu.", 
           "Max.", "Stdev.", "Sum" );
   printf( "%9d  %9.*f  %9.*f  %9.*f  %9.*f  %9.*f  %9.*f  %9s  %9.*f\n", 
           n, prec, x[0], prec, quantile( x, n, 0.25, type ), prec, median( x, n, 0 ),
           prec, ave, prec, quantile( x, n, 0.75, type ), 
           prec, x[n-1], stdevStr, prec, sum( x, n ) );
}

int main (int argc, char **argv)
{
   double ave = 0.0;
   double d;
   int n = 0;
   int prec = 4;
   int type = 7;
   gatherOptions( argc, argv, &prec, &type );
   doubleList *list = NULL;
   initList( list );
   while ( scanf("%lf", &d) == 1 ){
      addToList( &list, d );
      ave += ( d - ave ) / ++n;
   }
   double *array;
   array = listToArray( list, n );
   releaseList( list );
   summarize( array, n, ave, prec, type );
   free( array );
   return 0;
}

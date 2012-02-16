# summary

4 May 2011

## Author
[Dent Earl](https://github.com/dentearl/)
BME Dept, School of Engineering, UCSC


## Description
" summary() from the R project, now on the command line. "

summary is a C program for getting the [five number summary](http://en.wikipedia.org/wiki/Five-number_summary) 
(plus sample size and standard deviation) on the command line. 
It is a partial refactoring of the R quantile() function from the R project 
and as such is offered here under GPL 2.

## License
summary software is provided under the GNU GPL version 2.

## Example
    [dearl@machine demo]$ cat testData.txt | summary
        n       Min.    1st Qu.     Median       Mean    3rd Qu.       Max.     Stdev.        Sum
      100    -3.0166    -0.6717    -0.0531    -0.0974     0.5544     2.0199     0.9752    -9.7356

## Details
    [dearl@machine demo]$ summary --help
      summary an R-like five number (plus two) summary.
      Version 0.1.3
      4 May 2011
        dent earl, dearl (a) soe ucsc edu
    
      This program is free software; you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation; either version 2 of the License, or
      (at your option) any later version.
    
      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.
    
      You should have received a copy of the GNU General Public License
      along with this program; if not, a copy is available at
      http://www.gnu.org/licenses/gpl-2.0.html
    
      Thanks and credit are due to Ivan Frohne and Rob J Hyndman
      who wrote quantile() in R.
      References:
    
       Becker, R. A., Chambers, J. M. and Wilks, A. R. (1988) _The New S
       Language_.  Wadsworth & Brooks/Cole.
    
       Hyndman, R. J. and Fan, Y. (1996) Sample quantiles in statistical
       packages, _American Statistician_, *50*, 361-365.
    
      Usage:
       $ echo -e '112\n98.1\n101\n' | summary
            n       Min.    1st Qu.     Median       Mean    3rd Qu.       Max.     Stdev.
            3    98.1000    99.5500   101.0000   103.7000   106.5000   112.0000     7.3328

      stdin: one number on each line of input, no puntuation, text or
      blank lines.  

      OPTIONS
       --precision [0..9] adjusts the number of decimals. The default is 4.
       --type [1..9] selects one of the nine quantile algorithms. The default is 7. For
                 more information, inside of R try  ?stats::quantile to see details.
       --sorted  use this flag if your data is already sorted in assending order to
                 get a speedup.

#include<stdarg.h>
#include<stdio.h>

int sum(int, ...);

int main(void)
{
   printf("Sum of 10, 20 and 30 = %d\n",  sum(3, 10, 20, 30) );
   printf("Sum of 4, 20, 25 and 30 = %d\n",  sum(4, 4, 20, 25, 30) );

   return 0;
}

int sum(int num_args, ...)
{
   int val = 0;
   va_list ap;
   int i;

   va_start(ap, num_args);
   for(i = 0; i < num_args; i++)
   {
	  int x; 
      x = va_arg(ap, int);
	  printf("x is:%d\n", x);
      val += x;
   }
   va_end(ap);
 
   return val;
}

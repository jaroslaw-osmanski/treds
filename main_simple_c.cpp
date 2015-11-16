#include <stdio.h>
#include <cstdlib>
// malloc'ed
int* matrix_calculation(int* a, int ax, int ay, int* b, int bx, int by)
{
	if(ax != by) {
		return 0;
	}
	int* c = (int*) std::malloc(sizeof(int) * ay * bx);
	for (int j = 0; j < ay; ++j) {
		for (int i = 0; i < bx; ++i) {
			c[j * bx + i] = 0;
		}
	}

	for (int j = 0; j < ay; ++j) {
		for (int i = 0; i < bx; ++i) {
			for (int k = 0; k < by; ++k) {
				c[j * bx + i] += (a[j*ax + k] * b[by*k + i]);
			}
		}
	}

	return c;
}

int main()
{
  int* a = (int*) std::malloc(sizeof(int) * 2 * 2);
  int* b = (int*) std::malloc(sizeof(int) * 2 * 2);
  a[0] = 0; a[1] = 1; a[2] = 1; a[3] = 1;
  b[0] = 2; b[1] = 2; b[2] = 1; b[3] = 1;
  int* c = matrix_calculation(a, 2, 2, b, 2, 2);
  for(int i = 0; i < 4; ++i) printf("%d ", c[i]);
  free(a);
  free(b);
  free(c);
  return 0;
}

#include <cstdio>
#include <cstdlib>

// created with new
int* matrix_calculation(int* a, size_t ax, size_t ay,
		int* b, size_t bx, size_t by)
{
	if (ax != by) {
		return 0;
	}
	int* c = new int[(ay * bx)];
	for (size_t j = 0; j < ay; ++j) {
		for (size_t i = 0; i < bx; ++i) {
			c[j * bx + i] = 0;
		}
	}

	for (size_t j = 0; j < ay; ++j) {
		for (size_t i = 0; i < bx; ++i) {
			for (size_t k = 0; k < by; ++k) {
				c[j * bx + i] += (a[j*ax + k] * b[by*k + i]);
			}
		}
	}

	return c;
}

int main()
{
//simple tests
  int* a = new int[2 * 2];
  int* b = new int[2 * 2];
  a[0] = 0; a[1] = 1; a[2] = 1; a[3] = 1;
  b[0] = 2; b[1] = 2; b[2] = 1; b[3] = 1;
  int* c = matrix_calculation(a, 2, 2, b, 2, 2);
  for(size_t i = 0; i < 4; ++i) {
	  printf("%d ", c[i]);
  }
  delete[] a; //very important!
  delete[] b;
  delete[] c;
  return 0;
}

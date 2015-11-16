#include <cstdio>
#include <cstdlib>

class Matrix
{
public:
  Matrix(size_t x, size_t y) : p(new int[(x*y)]), x(x), y(y) {
    for(size_t i = 0; i < x; ++i) {
      for (size_t j = 0; j < y; ++j) {
        int& el = at(i,j);
        el = 0;
      }
    }
  };
  ~Matrix() {
    delete[] p;
  };

  int& at(int i, int j) {
    return p[j*x + i];
  };

  int sindex(int i) const {
    return p[i];
  }
  size_t get_x() const {
    return x;
  }

  size_t get_y() const {
    return y;
  }
private:
  int* p;
  size_t x;
  size_t y;
};

Matrix* matrix_calculation(Matrix* a, Matrix* b)
{
	if (a->get_x() != b->get_y()) {
		return 0;
	}
	Matrix* c = new Matrix(b->get_x(), a->get_y());

	size_t ay = a->get_y();

	size_t bx = b->get_x();
	size_t by = b->get_y();

	for (size_t j = 0; j < ay; ++j) {
		for (size_t i = 0; i < bx; ++i) {
			for (size_t k = 0; k < by; ++k) {
				c->at(i, j) += a->at(k, j) * b->at(i, k);
			}
		}
	}

	return c;
}


int main()
{
//simple tests
  Matrix* a = new Matrix(2, 2);
  Matrix* b = new Matrix(2, 2);
  a->at(0,0) = 0; a->at(1,0) = 1;
  a->at(0,1) = 1; a->at(1,1) = 1;

  b->at(0,0) = 2; b->at(1,0) = 2;
  b->at(0,1) = 1; b->at(1,1) = 1;

  Matrix* c = matrix_calculation(a, b);
  for(size_t i = 0; i < 4; ++i) {
	  printf("%d ", c->sindex(i));
  }
  delete a; //very important!
  delete b;
  delete c;
  return 0;
}

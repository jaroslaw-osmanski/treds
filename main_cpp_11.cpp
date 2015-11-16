#include <cstdio>
#include <cstdlib>
#include <functional>

class Matrix
{
public:
  int& at(size_t i, size_t j) {
    return p[j*x + i];
  };

  int at(size_t i, size_t j) const {
    return p[j*x + i];
  };

  Matrix(size_t x, size_t y) : p(new int[(x*y)]), x(x), y(y) {
    auto pred = [](Matrix& m, size_t i, size_t j) { m.at(i,j) = 0; };
    foreach(pred);
  };

  virtual ~Matrix() {
    delete[] p;
  };

  void foreach(std::function<void (Matrix& m, size_t x, size_t y)> pred) {
    for(size_t i = 0; i < x; ++i) {
      for (size_t j = 0; j < y; ++j) {
        pred(*this, i, j);
      }
    }
  }

  int sindex(size_t i) const {
    return p[i];
  }
  size_t get_x() const {
    return x;
  }

  size_t get_y() const {
    return y;
  }

  static Matrix* multiply(const Matrix& a, const Matrix& b)
  {
    if (a.get_x() != b.get_y()) {
      return 0;
    }
    Matrix* c = new Matrix(b.get_x(), a.get_y());

    auto multiply = [a,b](Matrix& m, size_t i, size_t j) {
      for (size_t k = 0; k < b.get_y(); ++k) {
        m.at(i, j) += a.at(k, j) * b.at(i, k);
      }
    };
    c->foreach(multiply);

    return c;
  }

private:
  int* p;
  size_t x;
  size_t y;
};

int main()
{
//simple tests
  Matrix a(2, 2);
  Matrix b(2, 2);
  a.at(0,0) = 0; a.at(1,0) = 1;
  a.at(0,1) = 1; a.at(1,1) = 1;

  b.at(0,0) = 2; b.at(1,0) = 2;
  b.at(0,1) = 1; b.at(1,1) = 1;

  Matrix* c = Matrix::multiply(a, b);
  for(size_t i = 0; i < 4; ++i) {
	  printf("%d ", c->sindex(i));
  }
  delete c;
  return 0;
}

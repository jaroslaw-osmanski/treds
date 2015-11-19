#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <vector>

#include <pthread.h>
#include "gtest/gtest.h"

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

  virtual Matrix* multiply(const Matrix& a, const Matrix& b)
  {
    if (a.get_x() != b.get_y()) {
      return 0;
    }
    Matrix* c = new Matrix(b.get_x(), a.get_y());

    auto multiply = [&a,&b](Matrix& m, size_t i, size_t j) {
      for (size_t k = 0; k < b.get_y(); ++k) {
        m.at(i, j) += a.at(k, j) * b.at(i, k);
      }
    };
    c->foreach(multiply);

    return c;
  }

  void print() {
    auto print_fun = [this](Matrix& m, size_t i, size_t j) {
      printf("%d ", m.at(i,j));
      if (j == this->get_x()) {
        printf("\n");
      }
    };
    foreach(print_fun);
  }

protected:
  int* p;
  size_t x;
  size_t y;
};


pthread_mutex_t thread_pool_m = PTHREAD_MUTEX_INITIALIZER;

class MatrixThreaded : public Matrix
{
public:
  MatrixThreaded(size_t x, size_t y) : Matrix(x, y) {
  };

  struct MultiplyData {

    MultiplyData(Matrix* _this,
        const Matrix& a, const Matrix& b,
        size_t i, size_t j, size_t thread_index) :
       _this(_this), a(a), b(b), i(i), j(j), thread_index(thread_index) {
    }

    Matrix* _this;
    const Matrix& a;
    const Matrix& b;
    size_t i;
    size_t j;
    size_t thread_index;
  };

  static void* multiply_thread_fun(void *multiply_data) {
    MultiplyData* data = static_cast<MultiplyData*>(multiply_data);

    Matrix* const _this = data->_this;
    const size_t i = data->i;

    for(size_t j = 0; j < _this->get_y(); ++j) {
      size_t sum = 0;
      for (size_t k = 0; k < data->b.get_y(); ++k) {
        sum += data->a.at(k, j) * data->b.at(i, k);
      }
      _this->at(i, j) = sum;
    }
    return 0;
  }

  virtual Matrix* multiply(const Matrix& a, const Matrix& b)
  {
    if (a.get_x() != b.get_y()) {
      return 0;
    }
    Matrix* c = new MatrixThreaded(b.get_x(), a.get_y());
    static const size_t THREAD_NUM = 8;
    pthread_t threads[THREAD_NUM];
    MultiplyData prototype(c, a, b, 0, 0, 0);
    std::vector<MultiplyData> data(THREAD_NUM, prototype);
    std::vector<size_t> threads_indexes;
    std::vector<size_t> threads_available;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    void *status;
//    int rc;

    for(size_t t_i = 0; t_i < THREAD_NUM; ++t_i) {
      threads_available.push_back(t_i);
    }


    for(size_t i = 0; i < x; ++i) {
        if(threads_indexes.size() >= THREAD_NUM) {
          pthread_join(threads[threads_indexes[0]], &status);
          threads_available.push_back(threads_indexes[0]);
          threads_indexes.erase(threads_indexes.begin());
        }

        size_t t_index = threads_available[0];
        threads_available.erase(threads_available.begin());
        data[t_index].i = i;
        data[t_index].j = 0;
        data[t_index].thread_index = t_index;
        pthread_create(&threads[t_index],
             &attr, multiply_thread_fun, (void*) &data.data()[t_index]);
        threads_indexes.push_back(t_index);
    }
    for (size_t in_t = 0; in_t < threads_indexes.size(); ++in_t) {
      pthread_join(threads[threads_indexes[in_t]], &status);
    }

    pthread_attr_destroy(&attr);

    return c;
  }





};

TEST(MatrixThreaded, SmallTest) {
  const size_t SIZE = 16;
  MatrixThreaded a(SIZE, SIZE);
  MatrixThreaded b(SIZE, SIZE);

  for (size_t i = 0; i < SIZE; ++i) {
    for (size_t j = 0; j < SIZE; ++j) {
      a.at(i,j) = (i + j) % 1024;
      b.at(i,j) = (i - j) % 1024;
    }
  }
  std::unique_ptr<Matrix> c(a.multiply(a, b));
}

TEST(MatrixThreaded, Large1024Test) {
  const size_t SIZE = 1024;
  MatrixThreaded a(SIZE, SIZE);
  MatrixThreaded b(SIZE, SIZE);

  for (size_t i = 0; i < SIZE; ++i) {
    for (size_t j = 0; j < SIZE; ++j) {
      a.at(i,j) = (i + j) % 1024;
      b.at(i,j) = (i - j) % 1024;
    }
  }
  std::unique_ptr<Matrix> c(a.multiply(a, b));
}


TEST(MatrixNotThreaded, Large1024Test) {
  const size_t SIZE = 1024;
  Matrix a(SIZE, SIZE);
  Matrix b(SIZE, SIZE);

  for (size_t i = 0; i < SIZE; ++i) {
    for (size_t j = 0; j < SIZE; ++j) {
      a.at(i,j) = (i + j) % 1024;
      b.at(i,j) = (i - j) % 1024;
    }
  }
  std::unique_ptr<Matrix> c(a.multiply(a, b));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

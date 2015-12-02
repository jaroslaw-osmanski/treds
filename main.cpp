#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <vector>
#include <queue>

#include <pthread.h>
#include <unistd.h>
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
        size_t start, size_t end, size_t thread_index) :
       _this(_this), a(a), b(b), start(start), end(end), thread_index(thread_index), is_finished(0) {
    }

    Matrix* _this;
    const Matrix& a;
    const Matrix& b;
    size_t start;
    size_t end;
    size_t thread_index;
    volatile bool is_finished;
  };

  static void* multiply_thread_fun(void *multiply_data) {
    MultiplyData* data = static_cast<MultiplyData*>(multiply_data);

    Matrix* const _this = data->_this;

    for (size_t i = data->start; i < data->end; ++i) {

      for(size_t j = 0; j < _this->get_y(); ++j) {
        unsigned int sum = 0;
        for (size_t k = 0; k < data->b.get_y(); ++k) {
          sum += data->a.at(k, j) * data->b.at(i, k);
        }
        _this->at(i, j) = sum;
      }

    }
    data->is_finished = 1;
    return 0;
  }

  template<size_t i, size_t to>
  struct For {
    static const size_t found = 1;
  };

  template<size_t to>
  struct For<to,to> {
    static const size_t found = to;
  };



  template<size_t THREADS>
  struct Manager {
    Manager() : threads_available(0), ALL((1 << THREADS) - 1)
    {
      for(size_t t_i = 0; t_i < THREADS; ++t_i) {
        threads_available += (1 << t_i);
      }
    }

    void republish(size_t index) {
      threads_available += (1 << index);

    }

    int join() {
      for(size_t t_i = 0; t_i < THREADS; ++t_i) {
        if (!(threads_available & (1 << t_i))) {
          threads_available += (1 << t_i);
          return t_i;
        }
      }
      return THREADS;
    }
    int select() {
      for(size_t t_i = 0; t_i < THREADS; ++t_i) {
        if (threads_available & (1 << t_i)) {
          threads_available -= (1 << t_i);
          return t_i;
        }
      }
      return THREADS;
    }

    int is_any_thread_used() {
      return threads_available != ALL;
    }
    int full() {
      return threads_available == 0;
    }

    int threads_available;
    const int ALL;
  };

  virtual Matrix* multiply(const Matrix& a, const Matrix& b)
  {
    if (a.get_x() != b.get_y()) {
      return 0;
    }
    Matrix* c = new MatrixThreaded(b.get_x(), a.get_y());
    static const size_t THREAD_NUM = 4;
    pthread_t threads[THREAD_NUM];
    Manager<THREAD_NUM> manager;
    MultiplyData prototype(c, a, b, 0, 0, 0);
    std::vector<MultiplyData> data(THREAD_NUM, prototype);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    void *status;



    for(size_t i = 0; i < x; ) {
        for (size_t i = 0; i < THREAD_NUM; ++i) {
          if(data[i].is_finished) {
            pthread_join(threads[i], &status);
            manager.republish(i);
            data[i].is_finished = 0;
          }
        }
        if(!manager.full()) {

        size_t t_index = manager.select();
        data[t_index].start = i;
        data[t_index].end = (i + 100 < x) ? i + 100 : x;
        data[t_index].thread_index = t_index;
        data[t_index].is_finished = 0;

        pthread_create(&threads[t_index],
             &attr, multiply_thread_fun, (void*) &data.data()[t_index]);
        i = i + 100;
        } else {
          usleep(100);
        }
    }
    while(manager.is_any_thread_used()) {
      size_t thread_index = manager.join();
      pthread_join(threads[thread_index], &status);
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
  const size_t SIZE = 5024;
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
  const size_t SIZE = 5024;
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

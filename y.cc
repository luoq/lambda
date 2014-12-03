#include <functional>
#include <iostream>
/* Simulate self application
   For Rec<F> f, f.o(f)
   For std::function<F(Rec<F>)> g, Rec<F> h, g(Rec<F>(g))
*/
template <typename F>
struct Rec{
  std::function<F(Rec<F>)> o;
};

// y0(f) will not halt as C++ is a strict language
template <typename X>
X y0(std::function<X(X)> f) {
  Rec<X> r{std::function<X(Rec<X>)>(
      [f](Rec<X> x){
	return f(x.o(x));
      })};
  return r.o(r);
}

template <typename A, typename B>
std::function<B(A)> y(std::function<std::function<B(A)>(std::function<B(A)>)> f) {
  typedef std::function<B(A)> X;
  Rec<X> r{std::function<X(Rec<X>)>(
      [f](Rec<X> x){
	return f(
	  std::function<B(A)>([x](A t){
	      return x.o(x)(t);
	    }));
      })};
  return r.o(r);
}


typedef std::function<int(int)> Func;
typedef std::function<Func(Func)> FuncFunc;
FuncFunc almost_fac = [](Func f) {
  return Func([f](int n) {
      if (n <= 1) return 1;
      return n * f(n - 1);
    });
};

FuncFunc almost_fib = [](Func f) {
  return Func([f](int n) {
      if (n <= 2) return 1;
      return  f(n - 1) + f(n - 2);
    });
};

int main() {
  auto fib = y(almost_fib);
  std::cout << "fib(6) = " << fib(6) << std::endl;
  auto fac = y(almost_fac);
  std::cout << "fac(10) = " << fac(10) << std::endl;
  return 0;
}

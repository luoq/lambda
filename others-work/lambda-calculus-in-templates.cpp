/*
 * A simple, lambda-calculus-based functional language encoded in C++
 * templates.  The language supports simple type-level literals,
 * conditionals and higher-order functions.
 *
 * We use an Eval/Apply-style interpreter as described in SICP.
 *
 * Author: Matthew Might
 * Site:   http://matt.might.net/
 *
 */

#include <stdlib.h>
#include <iostream>
#include <assert.h>



/* A type-level Pressburger encoding of natural numbers. */

// A type for 0:
struct Zero 
{
  enum { value = 0 } ;
} ;

// 1 == Succ<Zero>
// 2 == Succ<Succ<Zero> >
// ...
template <typename N>
struct Succ 
{
  enum { value = N::value + 1 } ;
} ;



/* The syntax contains five expression types: lambda terms for
 * anonymous functions, function applications, variable references,
 * conditionals and literals. */

// Core syntax:
template <int FormalName,typename Body>
struct Lambda {} ;

template <typename Fun, typename Arg>
struct App {} ; 

template <int Name>
struct Ref {} ;

// Sugar:
template <typename Cond, typename Then, typename Else>
struct If {} ;

template <typename T>
struct Lit {} ;




/* Environments are structures that map (variable) names to values.
 * Environments are paired with expressions to give meanings to free
 * variables. */

// EmptyEnv is the empty environment.
struct EmptyEnv ;

// Bindings<Name,Value,Env> is a type than encodes the environment Env
// extended with a binding for Name => Value.
template <int Name, typename Value, typename Env>
struct Binding {} ;

// EnvLookup<Name,Env> :: result looks up the value of Name in Env.
template <int Name, typename Env>
struct EnvLookup {} ;

template <int Name>
struct EnvLookup <Name,EmptyEnv> {} ; // Name not found.

template <int Name, typename Value, typename Env>
struct EnvLookup <Name, Binding<Name,Value,Env> > 
{
  Value typedef result ;
} ;

template <int Name, int Name2, typename Value2, typename Env>
struct EnvLookup <Name, Binding<Name2,Value2,Env> >
{
  typename EnvLookup<Name,Env> :: result typedef result ;
} ;




// Values:
template <typename Lam, typename Env>
struct Closure {} ;

struct True {} ;
struct False {} ;


/* We use Eval & Apply-style interpreter [SICP]. */

// Eval<Exp,Env> :: result is the value of expression Exp in
// environment Env.
template <typename Exp, typename Env>
struct Eval {} ;

// Apply<Proc,Value> :: result is the value of applying Proc to Value.
template <typename Proc, typename Value>
struct Apply {} ;



// Literals evaluate to themselves:
template <typename T, typename Env>
struct Eval <Lit<T>, Env>
{
  T typedef result ;
} ;

// Variable references are looked up in the current environment:
template <int Name, typename Env>
struct Eval <Ref<Name>, Env>
{
  typename EnvLookup<Name, Env> :: result typedef result ;
} ;

// Lambda terms evaluate into closures:
template <int Name, typename Body, typename Env>
struct Eval <Lambda<Name,Body>, Env>
{
  Closure<Lambda<Name, Body>, Env> typedef result ;
} ;

// Applications apply the value of the function expression to the
// value of the argument expression:
template <typename Fun, typename Arg, typename Env>
struct Eval<App<Fun, Arg> , Env> {
  typename Apply<typename Eval<Fun,Env> :: result ,
                 typename Eval<Arg,Env> :: result > :: result 
           typedef result ;
} ;

// Branch true:
template <typename Then, typename Else, typename Env>
struct Eval<If<True,Then,Else>,Env> {
  typename Eval<Then,Env> :: result typedef result ;
} ;

// Branch false:
template <typename Then, typename Else, typename Env>
struct Eval<If<False,Then,Else>,Env> {
  typename Eval<Else,Env> :: result typedef result ;
} ;

// Evaluate the condition:
template <typename Cond, typename Then, typename Else, typename Env>
struct Eval<If<Cond,Then,Else>,Env> {
  typename Eval<If<typename Eval<Cond,Env> :: result, 
                   Then,
                   Else>,
                Env> :: result 
           typedef result ;
} ;


// Transition to the body of the lambda term inside the closure:
template <int Name, typename Body, typename Env, typename Value>
struct Apply<Closure<Lambda<Name,Body>, Env>, Value> {
  typename Eval<Body, Binding<Name,Value,Env> > :: result 
           typedef result ;
} ;





int main (int argc, char* argv[]) {

  // Testing [2 => 1, 3 => 0](3):
  int v = EnvLookup<3, Binding<2, Succ<Zero>,
                       Binding<3, Zero,
                       EmptyEnv> > > :: result :: value ;

  assert(v == 0) ;


  // Testing ((lambda (x) x) 2):
  enum { X } ;
  
  int x = Eval<App<Lambda<X,Ref<X> >,Lit<Succ<Succ<Zero> > > >,EmptyEnv> :: result :: value ;

  assert(x == 2) ;


  // Testing (if #f 0 1):
  int y = Eval<If<Lit<False>,Lit<Zero>,Lit<Succ<Zero> > >,EmptyEnv> :: result :: value ;
  
  assert(y == 1) ;

  std::cout << "All tests passed!" << std::endl;

  return EXIT_SUCCESS ;
}

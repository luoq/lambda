#lang racket

;; PART 1 -- Introduction to Lambda Calculus
;;
;; Lambda-calculus is the world's smallest programming language.
;; In its entirety, it consists of:
;; *functions (sometimes called abstractions)
;; *function application
;; *variables

;; Let's start with the simple identity function:

(define id (λ(x) x))

;; Calling this function on itself, what is the result?
(id id)

;; Despite the minimal nature of lambda-calculus, it is Turing-complete,
;; and we can encode a wide array of language features.

;; PART 2 -- Boolean values
;;
;; To begin with, lets define boolean values.
;; We will name them 'tru' and 'fls' to avoid confusion with Racket's
;; own constructs.

(define tru (λ(x) (λ(y) x)))
(define fls (λ(x) (λ(y) y)))

;; Defining booleans as functions may seem odd.  In this case,
;; 'tru' is really 'returnFirstParameter' and 'fls' is
;; 'returnSecondParameter'.  We can use these values to form an
;; 'if then else' construct, which we will call 'test':

(define test (λ(cond) (λ(thn) (λ(els) ((cond thn) els)))))

;; The above function takes the boolean value and then applies it to the two
;; expressions.  In the case that it is 'tru', it will result in
;; returning the first expression.  If it is 'false', it will return
;; the second expression.  (Note that these expressions are curried,
;; so the syntax may look a little strange).

(((test tru) tru) fls)
(((test fls) tru) fls)

;; Reading #<prodedure:tru>, #<procedure:fls> is kind of ugly.
;; Is there a way that we can translate it to something similar to scheme?
;; As it turns out, we can use the test function that we have created.

(define (lambda->bool lam)
  (((test lam) #t) #f))

(lambda->bool tru)
(lambda->bool fls)


;; With our boolean values, we can also define logical operators, such as 'and'.
;; (We will call it 'andd' to avoid conflict's with Racket's 'and'.

(define andd (λ(b) (λ(c) ((b c) fls))))

;; Why does this work?  Here are a couple of tests to consider.

(lambda->bool ((andd tru) tru))
(lambda->bool ((andd tru) fls))

;; Define the 'nott' and 'orr' operators below without using andd.

(define nott (λ(b) ((b fls) tru)))

(define orr (λ(b) (λ(c) ((b tru) c))))

;; Here are some test cases:

(lambda->bool ((orr fls) (nott fls)))
(lambda->bool (nott ((orr fls) tru)))


;; PART 3 -- Pairs
;;
;; We can also encode data structures in lambda calculus.
;; We'll look at pairs.

(define pair (λ(f) (λ(s) (λ(b) ((b f) s)))))

;; When this function is applied to two arguments,
;; 'f' is bount to the first item in our pair and 's' to the second item.
;; The following is equivalent to (\b -> b tru fls):

(define myPair ((pair tru) fls))

;; In order for this to be useful, we must be able to get items out of the pair.
;; The following functions will get the first and second items, respectively:

(define first (λ(p) (p tru)))
(define second (λ(p) (p fls)))

;; The following lines produce "true" and "false" respectively.
;; Again, everything to the right of the $ is lambda calculus.

(lambda->bool (first myPair))
(lambda->bool (second myPair))

;; From pairs, we can begin to build other data structures such as lists.
;; (We'll leave this as an exercise for the reader).



;; PART 4 -- Church numerals
;;
;; With Boolean values, we were able to represent "primitive" values by
;; using special functions.  However, there are only two possible values.
;; How can we represent a potentially infinite set of values, like numbers?
;;
;; Church numerals define numbers as a collection of functions.
;; Below we will represent the numbers 0 through three:


(define zero  (λ(s) (λ(z) z)))
(define one   (λ(s) (λ(z) (s z))))
(define two   (λ(s) (λ(z) (s (s z)))))
(define three (λ(s) (λ(z) (s (s (s z))))))

;; So a number is represented by the number of 's' applications in the body.
;; Why is this a useful representation?  And what is s exactly?
;;
;; In these functions, z represents zero, and s represents a "successor".
;; As you might guess, we will need to create this successor function.

(define scc (λ(n) (λ(s) (λ(z) (s ((n s) z))))))

;; So, instead of defining one, two, three manually, we could instead derive them from zero.

(define one-alt (scc zero))
(define two-alt (scc one-alt))
(define three-alt (scc two-alt))

;; Walking through `scc zero` gives us the following steps:
;;     (λ(n) (λ(s) (λ(z) (s ((n s) z))))) zero
;;  -> (λ(s) (λ(z) (s ((zero s) z))))
;;  =  (λ(s) (λ(z) (s ((λ(s1) (λ(z1) z1)) s z))))   [expanding zero -- we use s1 and z1 to avoid confusion]
;;  -> (λ(s) (λ(z) (s (λ(z1) z1) z)))
;;  -> (λ(s) (λ(z) (s z)))
;;
;; So is this just a cute trick, or do the Church numerals actually represent numbers
;; in the real world in some meaningful way?  Note that each number is essentially
;; instructions for how to construct itself given a successor function and zero.
;; Exploiting this fact, we can write a fairly simple function to translate
;; Church numerals into Racket ints with the right successor function and zero value.

(define (lambda->int lam)
  ((lam (curry + 1)) 0))

(lambda->int zero)
(lambda->int one)
(lambda->int two)
(lambda->int three)

(lambda->int one-alt)
(lambda->int two-alt)
(lambda->int three-alt)


;; Using this principle, we can define addition of two numbers (m and n)
;; as getting the successor to n, m times.

(define plus (λ(m) (λ(n) ((m scc) n))))

(lambda->int ((plus three) two))

;; We can define multiplication in a similar manner:

(define multiply (λ(m) (λ(n) ((m (plus n)) zero))))


;; For ease of use, you may use the following function to convert a
;; Racket integer to a Church numeral.

(define (int->lambda n)
  (if (= n 0)
      zero
      (λ(s) (λ(z) (s (((int->lambda (- n 1)) s) z))))))

(lambda->int ((plus (int->lambda 3)) (int->lambda 4)))

(lambda->int ((multiply (int->lambda 2)) (int->lambda 3)))

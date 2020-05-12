#!/bin/bash

assert(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi

}

assert 0 0
assert 50 50
assert 21 "5+20-4" # add sub
assert 41 " 12 + 34 - 5" # white space
assert 47 '5+6*7' # mul
assert 8 '24 / 3' # div
#assert 15 '5*(9-6)' # mul and parenthesis
#assert 4 '(3+5)/2' # div and parenthesis
assert 10 "+10" # unary
assert 10 "-10+20" # negative unary 

# compare
assert 0 '0==1'
assert 1 '42==42'
assert 1 '0!=1'
assert 0 '42!=42'

assert 1 '0<1'
assert 0 '1<1'
assert 0 '2<1'
assert 1 '0<=1'
assert 1 '1<=1'
assert 0 '2<=1'

assert 1 '1>0'
assert 0 '1>1'
assert 0 '1>2'
assert 1 '1>=0'
assert 1 '1>=1'
assert 0 '1>=2' 


echo OK
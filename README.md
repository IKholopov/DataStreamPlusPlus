## Simple LINQ-like library for C++ (POC)

Works with arbitary containers, std::vector\<T\> by default

Implemnted Expressions:
  - Select
  - Take
  - GroupBy
  - OrderBy
  - Flatten
  - Where
  - Recurrent expression
  
  "First 5 numbers in Fibonacci sequence divisible by 3, squared if even" example:
  ```
  struct FibState {
      int current;
      int prev;
  } starting {1, 0};
  
  for(auto v: 
      RecurrentContainer< FibState >([](const FibState st){
                                        return FibState{st.current+st.prev, st.current};
                                    }, starting)
                                    .Select<int>([](const FibState st){ return st.current; })
                                    .Where([](const int x){ return x%3 == 0; })
                                    .Select<int>([](const int x) {
                                        return x%2 == 0 ? x*x : x;
                                    })
                                    .Take(5))
    {
        std::cout << v << std::endl;
    }
  ```
  A bit ugly? Probably, but this is what a proof of concept for.

---------------------------------------
Moved from private study repository

HPCE 2014 CW6
=============
The General approach
-------------------------------
Code was ran with the Visual Studio's Performance Profiler over a few minutes to identify key functions which were taking the most exectuion time:
![image](https://cloud.githubusercontent.com/assets/3355737/6650373/a8bd3e5e-ca06-11e4-9048-fa2f345940ef.png)

TBB was used to optimise these functions in the first instance, whilst if it seems that OpenCL can improve performance further, it is also implmented.

###Life
Within the `update` function, the 2 `for` loops for `dx` and `dy` only operates the function 9 times. By manually expand the operation to eliminate the need of this `for` loop, performance is increased.

The duration taken for the script to execute was measured with an example of `n`=100 and the results are as follows:

| |With given code | With loop unrolled|
|------------- |------------- | ------------- |
|TBB|17.44s| 16.38s
|OpenCL| 0.33s|0.34s

The unrolled loop provide a minor improvement for TBB and about the same performance for OpenCL. It is assummed that OpenCL operations are so quick anyway that those 2 short `for` loops will not provide much improvement in actual time. 

For TBB, the calling of update is parallelised since it loops `n`*`n` times, the library `tbb::parallel_for` with a `tbb::blocked_range2d` is used.

For OpenCL, the implementation is a bit more tricky since `bool` variables cannot be used as a kernal variable. Hence the vector `state` is turned into a `int` vector for kernel operations. It is also noted that `.at` is a C++ function, so it is replaced to simply calling the vector index directly.

#####Approach to improve performance
Testing is done and here's the graph


###Matrix_exponent
The function that took most time is `MatrixMul` since it contains 3 loops, each over `n` iterations. The operation within `MatrixMul` does not have any dependencies, providing scope to parallel the operations. 

For TBB, a new function `MatrixMul_tbb` is created and once again, the library `tbb::parallel_for` with a `tbb::blocked_range2d` is used. For OpenCL, a kernel `MatrixMul` is created to perform the same function. 

#####Approach to improve performance
Testing is done and here's the graph


###Median_bits
The part of the code that took the bulk of the execution time was the double for loops inside the implementation of `Execute()`. The operations were parallelised using `tbb:blocked_range2d` to parallelise the function across available CPU cores. Some tests were conducted to provide the best chunk range to use.

#####Approach to improve performance
![image](http://i.imgur.com/TASMYz5.png)

###Option_explicit
Similar to `median_bits`, the implementation of `execution()` had multiple for loops which could have been optimised.
Two of the `for` loops in the code had a variables vU and vD which depended on the value calculated on the previous iteration:
```
vU = vU*u;
vD = vD*d;
```
With knowledge of the `for` loop's iteration, we can calculate the value of `vU` by using:
```
vU = input->S0*std::pow(u, (i));
```
With `std::pow()` a relatively computationally expensive function compared to multiply, the program can be sped up further with vU only calculated at the start of each TBB `tbb::parfor` chunk:

```
auto f2 = [&](const my_range_t &chunk2){	
  double vU = input->S0*std::pow(u, (chunk2.begin()));	//This is expensive, do it outside of the i loop
  double vD = input->S0*std::pow(d, (chunk2.begin()));
  for (unsigned i = chunk2.begin(); i != chunk2.end(); i++){
	  double vCU = wU*state[n + i + 1] + wM*state[n + i] + wD*state[n + i - 1];	
	  double vCD = wU*state[n - i + 1] + wM*state[n - i] + wD*state[n - i - 1];
	  vCU = (std::max)(vCU, vU - input->K);	//vU depends on the previous iteration's result for vU
	  vCD = (std::max)(vCD, vD - input->K);
	  tmp[n + i] = vCU;
	  tmp[n - i] = vCD;
	  vU = vU*u;
	  vD = vD*d;
  }
};
tbb::parallel_for(range2, f2, tbb::simple_partitioner());
```

At the end of the for loop, there is a copy of `tmp` into `state`. This was sped up by swapping the pointers of `tmp` and `state` with 
```
std::swap(state, tmp);	//state = tmp;
```

###String_search
It was deemed that it is difficult to speed up `string_search` because of the dependencies between the loops. The pseudo code of the operation is at follows:

```
for each character (i from 0)
  for each pattern (p from 0)
  
    check if there is a match
    
    if there is a match
      increment the counter (histogram)
      increment i to the end of the matched string
    end
  end
end
```

When there is a match, the `i` increment not only avoids the same string being matched again (bearing in mind the wildcard `.` in the search term), but it also forbiddens other patterns further down the `input->pattern` array in the same region. This makes it difficult to parallel the operation without picking up the false alarms. 

![image](http://i.imgur.com/IGuOmdY.png)

As illustrated in the diagram above, the blocks represent the matches and only the green ones will be recorded based on the algorithm above. 

One of the ideas to speed up the puzzle is to parallellise the matching process and record the `offset`, `pattern` and `len` information of each match, then eliminate the false alarms from the results. The following algorithm is proposed:

```
parfor each pattern (p from 0)
  for each character (i from 0)
  
    check if there is a match
    
    if there is a match
      record the matched pattern, offset and length of match
      increment 1 to the ith position in a vector (offset_histogram). This vector records the number of matches in each offset - this helps to deal with duplicates and false alarms.
    end
  end
end

Set up a vector sized number of matches and initlise values to 1. This acts as an flag for whether the match is a false alarm or not. 

for each value of offset_histogram
  if the value is bigger than 1
    check the pattern of each entry and apart from the entry with the smallest pattern number, change the flags to zero.
  end
end

for each match
  check for the offset and length, and select the matches to keep as appropriate
end
```

Athough the potential speedup of the matching can be significant, the resultant sorting can only be done sequentially and this involves two `for` loops. Since the original code actually computes relatively fast anyway, it was deemed that the new algorithm might actually lengthen the computation so this was not attempted any further.

###Circuit_sim
With the help of Visual Studio's profiler, we found that the function `next` was taking up the most execution time. To optimise `next` we created function `next_tbb` to optimise the implementation of the function. Both `tbb::task_group` and `tbb::parfor` were trialed to optimise the operation of the program. 

`tbb::task_group` was attempted to be used in the `calcSrc` function which is called in a `for` loop inside the function `next`. However, it was found that `tbb::task_group` actually significantly slowed down the operation of the program, so this was not utilised in the code. 

`tbb::parfor` was utilised to parallelise the for loop in `next`. This was found to have sped up the performance of the program on AWS.

Some issues were encountered using `tbb::parfor` because of the type of data that was used (`vector<bool>`). This was remedied by converting the data while it was being operated on into `vector<char>` and creating overloaded functions which would operate on this type of data. `vector<char>` was selected as it was the next smallest array for storing data after `vector<bool>`

Testing Methodology
-------------------


Work division
-------------
Both of us sat down to analyse the different puzzles and the coding of puzzles are assigned to each person:
Yuchen - circuit_sim (TBB), option_explicit(TBB), median bits (TBB)
Michael -  life (TBB and OpenCL), matrix_exponent (TBB and OpenCL), string search (attempt)

At the testing stage, Yuchen wrote the bash scripts to effectively capture the useful information from the log output. Michael analysed the data and suggests the best approach for solving the puzzles. 





Original Specification
=====================
- Issued: Sun 8th March
- Due: Sun 22nd March, 23:59

(First DoC exam open to EEE/EIE/MSc is 24th March, AFAICT)

(Just realised I haven't made the private repositories yet. To come...)

Specification
-------------

You have been given the included code with the
goal of making things faster. On enquiring which
things, you were told "all the things". Further
deliberation on what a "thing" was resulted in
the elaboration that it was an instance of
`puzzler::Puzzle`. Further tentative queries
revealed that "faster" was determined by the
wall-clock execution time of `puzzler::Puzzle::Execute`,
with an emphasis on larger scale factors, on an
amazon GPU instance.

At this point marketing got quite irate, and
complained about developers not knowing how to
do their job, and they had commissioned this wonderful
enterprise framework, and did they have to do
all the coding themselves? Sales then chimed
in that they had similar problems having to
hold the developers hand, and that they did
VBA as part of their business masters, and it was
easy. Oh, and that they had already sold a customer
a version that contains more things; the spec should
be ready on Friday 13th (and no, that is not ominous,
it just happens to be a religious holiday for the
customer), but it is only "small" stuff. Developers
are all agile these days aren't they?

Meta-specification
------------------

The previous coursework was about deep diving on one
problem, and (hopefully) trying out a number of alternative
strategies. This coursework represents the other end
of the spectrum, which is sadly the more common end: you
haven't got much time, either to analyse the problem or
to do low-level optimisation, and the problem is actually
a large number of sub-problems. So the goal here is to
identify and capture as much of the low-hanging performance
fruit as possible while not breaking anything.

The code-base I've given you is somewhat baroque,
(though not as convoluted as my original version,
I took pity), and despite having some rather iffy
OOP practises, actually has things quite reasonably
isolated. You will probably encounter the problem
that sometimes the reference solution starts to take
a very long time at large scales, but the persistence
framework gives you a way of dealing with that.

Beyond that, there isn't a lot more guidance, either
in terms of what you should focus on, or how
_exactly_ it will be measured. Part of the assesment
is in seeing whether you can work out what can be
accelerated, and where you should spend your time.
And in reacting to externally evolving specs and
code - the Friday 13th comment is true, though the
change is minor (there aren't another five problems),
additive (all work done this week is needed and
evaluated for the final assesment), and has a default
fallback (a git pull will bring any submission back
into correcness).

The allocation of marks I'm using is as before:

- Performance: 33%

  - You are competing with each other here, so there is an element of
    judgement in terms of how much you think others are doing or are
    capable of.

- Correctness: 33%

  - As far as I'm aware the ReferenceExecute is always correct, though slow.

- Code style, insight, analysis: 33%

  - Can I understand your code (can you understand your code)? Are the methods
    and techniques employed appropriate?

Deliverable format
------------------

- As before, your repository should contain a readme.txt, readme.pdf, or readme.md covering:

    - What is the approach used to improve performance, in terms of algorithms, patterns, and optimisations.

    - A description of any testing methodology or verification.

    - A summary of how work was partitioned within the pair, including planning, analysis, design, and testing, as well as coding.

- Anything in the `include` directory is not owned by you, and subject to change
  
  - Changes will happen in an additive way (existing classes and APIs will remain, new ones may be added)
  
  - Bug-fixes to `include` stuff are still welcome.

- The public entry point to your code is via `puzzler::PuzzleRegistrar::UserRegisterPuzzles`,
    which must be compiled into the static library `lib/libpuzzler.a`.
    
    - Clients will not directly include your code, they will only `#include "puzzler/puzzles.h`,
      then access puzzles via the registrar. They will get access to the registrar implementation
      by linking against `lib/libpuzzler.a`.
    
    - **Note**: If you do something complicated in your building of libpuzzler, it should still be
      possible to build it by going into `lib` and calling `make all`.
      
- The programs in `src` have no special meaning or status, they are just example programs 

The reason for all this strange indirection is that I want to give
maximum freedom for you to do strange things within your implementation
(example definitions of "strange" include CMake) while still having a clean
abstraction layer between your code and the client code.

Notes
-----

All the algorithms here are quite classic, though for the most
part different enough to require some thought. Where it is possible
to directly use an off-the-shelf implementation (partially true in
most cases), you need to bear in mind that you're trying to
show-case your understanding and ability here. So if you're
relying on someone elses library, you need to:

- Correctly and clearly attribute it
- Be able to demonstrate you understand how and why it works

Make sure you spend a little bit of time thinking about how
feasible it is to accelerate something - in some cases you
may be able to get linear speed-up in the processor count,
in others less so. Sometimes the fundamental algorithmic
complexity doesn't look friendly, and can be improved in
simple ways.

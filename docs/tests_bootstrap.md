# Bootstrap

`AFF3CT-core` comes with simple tests to validate its behavior. The later are
always a good way to bootstrap when you want to write your first code with the
DSEL. The source codes of the following tests are located in the 
`tests/bootstrap` folder. Each file corresponds to an executable test.

## Simple Chain

At some point we have to start with something :-). The following graphs are a 
very simple chains made from `increment`/`incrementf` tasks that simply perform 
"$+1$" on the data. Each time there are 6 `Incrementer` ($t_{[2:7]}$) so the 
final expected values in the `Finalizer` ($t_8$) is equal to the values from the 
`Initilizer` ($t_1$) "$+6$".

=== "With Input and Output Sockets"

    <figure markdown>
      ![Simple sequence](./assets/test_simple_chain.svg){ width="1000" }
      <figcaption>`test-simple-chain`.</figcaption>
    </figure>
    ```bash
    test-simple-chain -t 1
    ```  
    This version of the simple chain is based on `increment` tasks that have one
    input socket and one output socket.

    **Command Line Arguments**

    The following verbatim is a copy-paste from the `-h` stdout:

    ```bash
    usage: ./bin/test-simple-chain [options]

      -t, --n-threads       Number of threads to run in parallel                                  [10]
      -f, --n-inter-frames  Number of frames to process in one task                               [1]
      -s, --sleep-time      Sleep time duration in one task (microseconds)                        [5]
      -d, --data-length     Size of data to process in one task (in bytes)                        [2048]
      -e, --n-exec          Number of sequence executions                                         [100000]
      -o, --dot-filepath    Path to dot output file                                               [empty]
      -c, --copy-mode       Enable to copy data in sequence (performance will be reduced)         [true]
      -b, --step-by-step    Enable step-by-step sequence execution (performance will be reduced)  [false]
      -p, --print-stats     Enable to print per task statistics (performance will be reduced)     [false]
      -g, --debug           Enable task debug mode (print socket data)                            [false]
      -u, --subseq          Enable subsequence in the executed sequence                           [false]
      -v, --verbose         Enable verbose mode                                                   [false]
      -h, --help            This help                                                             [false]
    ```

=== "With Forward Sockets"

    <figure markdown>
      ![Simple sequence](./assets/test_simple_chain_fwd.svg){ width="1000" }
      <figcaption>`test-simple-chain-fwd`.</figcaption>
    </figure>
    ```bash
    test-simple-chain-fwd -t 1
    ```  
    This version of the simple chain is based on `incrementf` tasks that have
    only one forward socket.

    **Command Line Arguments**

    The following verbatim is a copy-paste from the `-h` stdout:

    ```bash
    usage: ./bin/test-simple-chain-fwd [options]

      -t, --n-threads       Number of threads to run in parallel                                  [10]
      -f, --n-inter-frames  Number of frames to process in one task                               [1]
      -s, --sleep-time      Sleep time duration in one task (microseconds)                        [5]
      -d, --data-length     Size of data to process in one task (in bytes)                        [2048]
      -e, --n-exec          Number of sequence executions                                         [100000]
      -o, --dot-filepath    Path to dot output file                                               [empty]
      -c, --copy-mode       Enable to copy data in sequence (performance will be reduced)         [true]
      -b, --step-by-step    Enable step-by-step sequence execution (performance will be reduced)  [false]
      -p, --print-stats     Enable to print per task statistics (performance will be reduced)     [false]
      -g, --debug           Enable task debug mode (print socket data)                            [false]
      -u, --subseq          Enable subsequence in the executed sequence                           [false]
      -v, --verbose         Enable verbose mode                                                   [false]
      -h, --help            This help                                                             [false]
    ```

=== "With Input, Output Sockets and Forward Sockets"

    <figure markdown>
      ![Simple sequence](./assets/test_simple_chain_hybrid.svg){ width="1000" }
      <figcaption>`test-simple-chain-hybrid`.</figcaption>
    </figure>
    ```bash
    test-simple-chain-hybrid -t 1
    ```  
    This version of the simple chain is based on a combination of `increment` 
    and `incrementf` tasks.

    **Command Line Arguments**

    The following verbatim is a copy-paste from the `-h` stdout:

    ```bash
    usage: ./bin/test-simple-chain-hybrid [options]

      -t, --n-threads       Number of threads to run in parallel                                  [10]
      -f, --n-inter-frames  Number of frames to process in one task                               [1]
      -s, --sleep-time      Sleep time duration in one task (microseconds)                        [5]
      -d, --data-length     Size of data to process in one task (in bytes)                        [2048]
      -e, --n-exec          Number of sequence executions                                         [100000]
      -o, --dot-filepath    Path to dot output file                                               [empty]
      -c, --copy-mode       Enable to copy data in sequence (performance will be reduced)         [true]
      -b, --step-by-step    Enable step-by-step sequence execution (performance will be reduced)  [false]
      -p, --print-stats     Enable to print per task statistics (performance will be reduced)     [false]
      -g, --debug           Enable task debug mode (print socket data)                            [false]
      -v, --verbose         Enable verbose mode                                                   [false]
      -h, --help            This help                                                             [false]
    ```

----

## Looping

The looping tests are here to introduce and to validate how to implement simple
control flow. Each of the graph below execute one or multiple loops and in the
innermost loop 6 `increment` tasks are executed. `Switcher` modules are used to 
create two different paths, one for the loop and the other for the exit (the 
`Finalizer` task here). The output socket of the `iterate` task (from the 
`Iterator`  module) is bound to the input socket 1 of the `commute` task. The 
`iterate` task controls if the commute should execute the 6 `increment` tasks or 
the end the stream by executing the `finalize` task.

=== "`For` Loop"

    <figure markdown>
      ![Simple sequence](./assets/test_for_loop.svg){ width="1000" }
      <figcaption>`test-for-loop`.</figcaption>
    </figure>
    ```bash
    test-for-loop -t 1 -i 10
    ```
    This test implements a classic `for-loop` where the condition is evaluated
    first in $t_3$ (a basic loop counter) and the the body of the loop is 
    executed ($t_{[5:10]}$) after. Note that this `for-loop` example can easily
    be extended to a more generic `while-loop` if the $t_3$ task is replaced 
    by an other task that depends on an input data socket. 

    In the command line, `-i 10` indicates that the loop is repeated 10 times.

    **Command Line Arguments**

    The following verbatim is a copy-paste from the `-h` stdout:

    ```bash
    usage: ./bin/test-for-loop [options]

      -t, --n-threads       Number of threads to run in parallel                                  [10]
      -f, --n-inter-frames  Number of frames to process in one task                               [1]
      -s, --sleep-time      Sleep time duration in one task (microseconds)                        [5]
      -d, --data-length     Size of data to process in one task (in bytes)                        [2048]
      -e, --n-exec          Number of sequence executions                                         [100000]
      -i, --n-loop          Number of iterations to perform in the loop                           [10]
      -o, --dot-filepath    Path to dot output file                                               [empty]
      -c, --copy-mode       Enable to copy data in sequence (performance will be reduced)         [true]
      -b, --step-by-step    Enable step-by-step sequence execution (performance will be reduced)  [false]
      -p, --print-stats     Enable to print per task statistics (performance will be reduced)     [false]
      -g, --debug           Enable task debug mode (print socket data)                            [false]
      -h, --help            This help                                                             [false]
    ```

=== "`Do-while` Loop"

    <figure markdown>
      ![Simple sequence](./assets/test_do_while_loop.svg){ width="1000" }
      <figcaption>`test-do-while-loop`.</figcaption>
    </figure>
    ```bash
    test-do-while-loop -t 1 -i 10
    ```
    An implementation of a `do-while` loop where the condition $t_9$ is 
    evaluated after the body of the loop ($t_{[3:8]}$).

    In the command line, `-i 10` indicates that the loop is repeated 10 times.

    **Command Line Arguments**

    The following verbatim is a copy-paste from the `-h` stdout:

    ```bash
    usage: ./bin/test-do-while-loop [options]

      -t, --n-threads       Number of threads to run in parallel                                  [10]
      -f, --n-inter-frames  Number of frames to process in one task                               [1]
      -s, --sleep-time      Sleep time duration in one task (microseconds)                        [5]
      -d, --data-length     Size of data to process in one task (in bytes)                        [2048]
      -e, --n-exec          Number of sequence executions                                         [100000]
      -i, --n-loop          Number of iterations to perform in the loop                           [9]
      -o, --dot-filepath    Path to dot output file                                               [empty]
      -c, --copy-mode       Enable to copy data in sequence (performance will be reduced)         [true]
      -b, --step-by-step    Enable step-by-step sequence execution (performance will be reduced)  [false]
      -p, --print-stats     Enable to print per task statistics (performance will be reduced)     [false]
      -g, --debug           Enable task debug mode (print socket data)                            [false]
      -h, --help            This help                                                             [false]
    ```

=== "Nested Loops"

    <figure markdown>
      ![Simple sequence](./assets/test_nested_loops.svg){ width="1000" }
      <figcaption>`test-nested-loops`.</figcaption>
    </figure>

    ```bash
    test-do-while-loop -t 1 -i 5 -j 2
    ```

    Implementation of 2 nested `for-loop`s. `-j 2` controls the number of times
    the innermost loops is repeated and `-i 5` controls the outermost loop. 

    **Command Line Arguments**

    The following verbatim is a copy-paste from the `-h` stdout:

    ```bash
    usage: ./bin/test-nested-loops [options]

      -t, --n-threads       Number of threads to run in parallel                                  [10]
      -f, --n-inter-frames  Number of frames to process in one task                               [1]
      -s, --sleep-time      Sleep time duration in one task (microseconds)                        [5]
      -d, --data-length     Size of data to process in one task (in bytes)                        [2048]
      -e, --n-exec          Number of sequence executions                                         [100000]
      -i, --n-loop-out      Number of iterations to perform in the outer loop                     [5]
      -j, --n-loop-in       Number of iterations to perform in the inner loop                     [2]
      -o, --dot-filepath    Path to dot output file                                               [empty]
      -c, --copy-mode       Enable to copy data in sequence (performance will be reduced)         [true]
      -b, --step-by-step    Enable step-by-step sequence execution (performance will be reduced)  [false]
      -p, --print-stats     Enable to print per task statistics (performance will be reduced)     [false]
      -g, --debug           Enable task debug mode (print socket data)                            [false]
      -h, --help            This help                                                             [false]
    ```

----

## `Switch-case`

The following test implements a `switch-case` pattern based on `increment` 
tasks ($t_{[4;9]}$). Depending on the `control` task ($t_2$), one of the three 
different paths will be executed. The first path is composed by $t_4$, $t_5$ and 
$t_6$ tasks, the second path is composed by $t_7$ and $t_8$ tasks while the 
third path is only composed by the $t_9$ task.

<figure markdown>
  ![Simple sequence](./assets/test_exclusive_paths.svg){ width="1000" }
  <figcaption>test-exclusive-paths.</figcaption>
</figure>

```bash
test-exclusive-paths -t 1 -y
```

The `-y` option indicates that the `Controller` is cyclic: for the first stream
the first path is selected, for the second stream the second path is taken, for
the third stream the third path is taken, for the fourth stream the first path
is taken and so on...

It is also possible to have a fixed path for all the streams with the `-a N` 
option (with `N` the path id).

!!! note
    You cannot use `-y` and `-a` parameters at the same time, they are 
    exclusive.

**Command Line Arguments**

The following verbatim is a copy-paste from the `-h` stdout:

```bash
usage: ./bin/test-exclusive-paths [options]

  -t, --n-threads       Number of threads to run in parallel                                  [10]
  -f, --n-inter-frames  Number of frames to process in one task                               [1]
  -s, --sleep-time      Sleep time duration in one task (microseconds)                        [5]
  -d, --data-length     Size of data to process in one task (in bytes)                        [2048]
  -e, --n-exec          Number of sequence executions                                         [100000]
  -a, --path            Path to take in the switch (0, 1 or 2)                                [0]
  -o, --dot-filepath    Path to dot output file                                               [empty]
  -c, --copy-mode       Enable to copy data in sequence (performance will be reduced)         [true]
  -b, --step-by-step    Enable step-by-step sequence execution (performance will be reduced)  [false]
  -p, --print-stats     Enable to print per task statistics (performance will be reduced)     [false]
  -g, --debug           Enable task debug mode (print socket data)                            [false]
  -y, --cyclic-path     Enable cyclic selection of the path (with this `--path` is ignored)   [false]
  -h, --help            This help                                                             [false]
```

## Simple Pipeline

This test is an implementation of a 3 stages pipeline. The first stage 
$S_1 = t_1$ reads data from a file. Thus, the `generate` task ($t_1$) is 
intrinsically sequential (= executes on a single thread). The second stage 
$S_2 = t_{[2:7]}$ just copies the data from the stage $S_1$ through `relay` 
tasks. The `relay` tasks can be replicated and ran over multiple threads. 
Finally, the last stage $S_3 = t_8$ is a `Sink` that writes the data on the 
file system. This operation is also intrinsically sequential.

<figure markdown>
  ![Simple sequence](./assets/test_simple_pipeline.svg){ width="1000" }
  <figcaption>test-simple-pipeline.</figcaption>
</figure>

```bash
test-simple-pipeline -t 3 --in-filepath ~/.bashrc --out-filepath output_file
```

!!! note
    The second output socket of the `generate` task ($t_1$) is bound to the
    second input socket of the `send_count` task ($t_8$). If the read data is 
    not a multiple of the stream size, then the stream is padded with 0. But,
    these zeros are NOT written on the file system by the `send_count` task.
    The second output socket of the `generate` task ($t_1$) contains the number
    of bits that have been read from the input file. Then, the `send_count` task
    can avoid to write the padding zeros (if any).

**Command Line Arguments**

The following verbatim is a copy-paste from the `-h` stdout:

```bash
usage: ./bin/test-simple-pipeline [options]

  -t, --n-threads       Number of threads to run in parallel                                  [10]
  -f, --n-inter-frames  Number of frames to process in one task                               [1]
  -s, --sleep-time      Sleep time duration in one task (microseconds)                        [5]
  -d, --data-length     Size of data to process in one task (in bytes)                        [2048]
  -u, --buffer-size     Size of the buffer between the different stages of the pipeline       [2048]
  -o, --dot-filepath    Path to dot output file                                               [empty]
  -i, --in-filepath     Path to the input file (used to generate bits of the chain)           [empty]
  -j, --out-filepath    Path to the output file (written at the end of the chain)             ["file.out"]
  -c, --copy-mode       Enable to copy data in sequence (performance will be reduced)         [true]
  -b, --step-by-step    Enable step-by-step sequence execution (performance will be reduced)  [false]
  -p, --print-stats     Enable to print per task statistics (performance will be reduced)     [false]
  -g, --debug           Enable task debug mode (print socket data)                            [false]
  -q, --force-sequence  Force sequence instead of pipeline                                    [false]
  -w, --active-waiting  Enable active waiting in the pipeline synchronizations                [false]
  -h, --help            This help                                                             [false]
```


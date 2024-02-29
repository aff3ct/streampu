<a name="BootstrapTests"></a>
# Bootstrap

`AFF3CT-core` comes with simple tests to validate its behavior. The later are
always a good way to bootstrap when you want to write your first code with the
DSEL. The following sections describe each test from the `tests/bootstrap` 
folder.

## Simple Chain

### The Beginning of the Beginning

At some point we have to start with something :-). This graph is a very simple
chain made from *increment* tasks that simply perform "$+1$" on the data.

<figure markdown>
  ![Simple sequence](./assets/test_simple_chain.svg){ width="1000" }
  <figcaption>test-simple-chain.</figcaption>
</figure>

### With Forward Sockets

<figure markdown>
  ![Simple sequence](./assets/test_simple_chain_fwd.svg){ width="1000" }
  <figcaption>test-simple-chain-fwd.</figcaption>
</figure>

### Combining In/Out and Forward Sockets

<figure markdown>
  ![Simple sequence](./assets/test_simple_chain_hybrid.svg){ width="1000" }
  <figcaption>test-simple-chain-hybrid.</figcaption>
</figure>

## Looping

### `For` Loop

<figure markdown>
  ![Simple sequence](./assets/test_for_loop.svg){ width="1000" }
  <figcaption>test-for-loop.</figcaption>
</figure>

### `Do-while` loop

<figure markdown>
  ![Simple sequence](./assets/test_do_while_loop.svg){ width="1000" }
  <figcaption>test-do-while-loop.</figcaption>
</figure>

### Nested Loops

<figure markdown>
  ![Simple sequence](./assets/test_nested_loops.svg){ width="1000" }
  <figcaption>test-nested-loops.</figcaption>
</figure>

## `Switch-case` with Exclusive Paths

<figure markdown>
  ![Simple sequence](./assets/test_exclusive_paths.svg){ width="1000" }
  <figcaption>test-exclusive-paths.</figcaption>
</figure>

## Simple Pipeline

<figure markdown>
  ![Simple sequence](./assets/test_simple_pipeline.svg){ width="1000" }
  <figcaption>test-simple-pipeline.</figcaption>
</figure>

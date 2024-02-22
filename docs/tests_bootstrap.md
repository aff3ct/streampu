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

TODO

### `For` Loop

TODO

### `Do-while` loop

TODO

### Nested Loops

TODO

## `Switch-case` with Exclusive Paths

TODO

## Simple Pipeline

TODO

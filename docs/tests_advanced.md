# Advanced

## Generic Pipeline

A new test with a generic pipeline has also been added. It is possible to define 
the middle tasks from the command line (the initial `Init` and last task `Sink` 
are automatically added) using these parameters:

- `-n`: the number of tasks on each stage.
- `-t`: the number of threads on each stage.
- The socket type (`SIO` or `SFWD`) of the tasks:
    - `-r`: specifying each socket type (`SIO` $\rightarrow$ `relay` task and 
            `SFWD` $\rightarrow$  `relayf` task).
    - `-R`: specifying socket type by stage (all the sockets of the stage will
      be of this type).

!!! note
    You cannot use `-r` and `-R` parameters at the same time, they are 
    exclusive.

Here are some examples of generated pipelines:

=== "Simple pipeline" 
    <figure markdown>
      ![simple pipeline io](./assets/test_generic_pipeline_io.svg){ width="600" }
      <figcaption>`test-generic-pipeline`: input/output sockets & 3-stage pipeline.</figcaption>
    </figure>
    ```bash
    test-generic-pipeline -i INPUT_FILE -n "1,3,1" -t "1,3,1" -R "(read,relay,write)"
    ```

=== "Simple pipeline forward"
    <figure markdown>
      ![simple pipeline fwd](./assets/test_generic_pipeline_fwd.svg){ width="600" }
      <figcaption>`test-generic-pipeline`: forward sockets & 3-stage pipeline.</figcaption>
    </figure>
    ```bash
    test-generic-pipeline -i INPUT_FILE -n "1,3,1" -t "1,3,1" -R "(read,relayf,write)"
    ```

=== "Simple pipeline hybrid"
    <figure markdown>
      ![simple pipeline hybrid](./assets/test_generic_pipeline_hybrid.svg){ width="600" }
      <figcaption>`test-generic-pipeline`: hybrid in/out and forward sockets & 3-stage pipeline.</figcaption>
    </figure>
    ```bash
    test-generic-pipeline -i INPUT_FILE -t "1,3,1" -r "((read),(relayf,relay,relayf),(write))"
    ```

=== "Simple pipeline hybrid with a 5-stage pipeline"
    <figure markdown>
      ![simple pipeline hybrid](./assets/test_generic_pipeline_hybrid_5_stages.svg){ width="1100" }
      <figcaption>`test-generic-pipeline`: hybrid in/out and forward sockets & 5-stage pipeline.</figcaption>
    </figure>
    ```bash
    test-generic-pipeline -i INPUT_FILE -t "1,3,1,2,1" -r "((read),(relayf,relay,relayf,relay),(relayf),(relay,relay),(write))"
    ```

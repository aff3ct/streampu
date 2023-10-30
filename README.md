# AFF3CT-core

[![GitLab Pipeline Status](https://img.shields.io/gitlab/pipeline-status/aff3ct/aff3ct-core.svg?branch=development)](https://gitlab.com/aff3ct/aff3ct-core/pipelines)
[![GitLab Code Coverage](https://img.shields.io/gitlab/pipeline-coverage/aff3ct/aff3ct-core?branch=development)](https://aff3ct.gitlab.io/aff3ct-core/)
[![License: MIT](https://img.shields.io/github/license/aff3ct/aff3ct-core.svg)](./LICENSE)

This project is used as the multi-threaded dataflow runtime engine of
[AFF3CT](https://github.com/aff3ct/aff3ct) (a simulator for channel coding and a
library for real time Software-Defined Radio systems).

As the core of [AFF3CT](https://github.com/aff3ct/aff3ct) was no longer specific 
to [AFF3CT](https://github.com/aff3ct/aff3ct), it has been extracted from it: 
**AFF3CT-core was born!**

Here are the main features of AFF3CT-core:
  - Definition of modules, tasks and sockets (dataflow)
  - Elementary modules and tasks implementations
  - Domain Specific Embedded Language for streaming applications
  - Parallel constructs (fork-join, pipeline)

This library is suitable for SDR systems, video processing and more generally it
matches single-rate Synchronous DataFlow (SDF) streaming applications.

## Documentation

*Coming soon :-)!*

## License

The project is licensed under the MIT license.

## How to cite AFF3CT-core

The main contributions of this work are described in the following journal 
article:  
- A. Cassagne, R. Tajan, O. Aumage, D. Barthou, C. Leroux and C. Jégo,
  “[A DSEL for High Throughput and Low Latency Software-Defined Radio on Multicore CPUs](https://doi.org/10.1002/cpe.7820),“
  *Wiley Concurrency and Computation: Practice and Experience (CCPE)*, 2023 [[Bibtex Entry](https://aff3ct.github.io/resources/bibtex/Cassagne2023%20-%20A%20DSEL%20for%20High%20Throughput%20and%20Low%20Latency%20Software-Defined%20Radio%20on%20Multicore%20CPUs.bib)]

*To thank us even more*, we encourage you to also cite the original 
[AFF3CT](https://github.com/aff3ct/aff3ct) journal article to give more 
visibility to our work:  
- A. Cassagne et al.,
  “[AFF3CT: A Fast Forward Error Correction Toolbox!](https://doi.org/10.1016/j.softx.2019.100345),“
  *Elsevier SoftwareX*, 2019 [[Bibtex Entry](https://aff3ct.github.io/resources/bibtex/Cassagne2019a%20-%20AFF3CT:%20A%20Fast%20Forward%20Error%20Correction%20Toolbox.bib)]

# Investigating possible lazy evaluation implementations for OpenFOAM

This repository investigates the viability of switching lazy evaluation in OpenFOAM code. Switching is a good idea only if:
- Lazy evaluation provides a significant performance improvement in meshfree context.
- Using the API needs to stay straightforward. The API can be radically different than what OpenFOAM API looks like.

Other factors that may affect the decision to switch:
- Planning to use multi-threading?
- Computation on GPU

> **This requires G++13 or higher and we need to link against `-std=c++23`**

> **Also, OpenFOAM code needs to be fixed to compile with C++23**

Benchmarks are performed using [foamUT](https://github.com/FoamScience/foamUT) with Catch2 as a back-end.

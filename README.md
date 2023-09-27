# Investigating possible lazy evaluation implementations for OpenFOAM

Take a look at this [blog post](https://foamscience.github.io/MeshFreeFoam-Docs/blog/2023/09/27/lazy-evaluation-part-1/) for a detailed discussion and possible updates on this issue.

This repository investigates the viability of switching lazy evaluation in OpenFOAM code. Switching is a good idea only if:
- Lazy evaluation provides a significant performance improvement in meshfree context.
- Using the API needs to stay straightforward. The API can be radically different than what OpenFOAM API looks like.

Other factors that may affect the decision to switch:
- Planning to use multi-threading?
- Computation on GPU

> **This requires G++13 or higher and we need to link against `-std=c++23`**

> **Also, OpenFOAM code needs to be fixed (very minor things) to compile with C++23**

Benchmarks are performed using [foamUT](https://github.com/FoamScience/foamUT) with Catch2 as a back-end. On Github's machines, the speedup is almost x3 for the few [operations](https://github.com/FoamScience/Eager-TemplateExpr-Views-OpenFOAM/blob/80c59112d5660cd24630e4c57c6f7648c8a0b8c7/expressionTemplatesVsViewsTests.C#L30) I test with.
